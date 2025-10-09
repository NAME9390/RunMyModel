"""
FastAPI routes for model management and inference
"""

from fastapi import APIRouter, HTTPException, BackgroundTasks
from fastapi.responses import StreamingResponse
from pydantic import BaseModel
from typing import Optional, List
import json

from models.loader import ModelManager
from utils.config import Config

router = APIRouter()

# Global instances (injected from main.py)
model_manager = ModelManager()
config = Config()

# Pydantic models for request/response
class ModelLoadRequest(BaseModel):
    model_name: str
    n_ctx: int = 4096
    n_gpu_layers: int = -1

class ChatCompletionRequest(BaseModel):
    model: str
    messages: List[dict]
    temperature: float = 0.7
    max_tokens: int = 512
    top_p: float = 0.9
    top_k: int = 40
    stream: bool = True

class CompletionRequest(BaseModel):
    model: str
    prompt: str
    temperature: float = 0.7
    max_tokens: int = 512
    top_p: float = 0.9
    top_k: int = 40
    stream: bool = True

# ==================== Model Management ====================

@router.get("/models")
async def list_models():
    """List all available models"""
    models = model_manager.scan_available_models()
    return {"models": models, "count": len(models)}

@router.get("/models/loaded")
async def list_loaded_models():
    """List currently loaded models"""
    loaded = [
        name for name in model_manager.loaded_models.keys()
    ]
    return {"loaded_models": loaded, "count": len(loaded)}

@router.post("/models/load")
async def load_model(request: ModelLoadRequest):
    """Load a model into memory"""
    try:
        success = model_manager.load_model(
            model_name=request.model_name,
            n_ctx=request.n_ctx,
            n_gpu_layers=request.n_gpu_layers
        )
        return {
            "success": success,
            "model": request.model_name,
            "message": f"Model {request.model_name} loaded successfully"
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/models/unload")
async def unload_model(model_name: str):
    """Unload a model from memory"""
    success = model_manager.unload_model(model_name)
    if success:
        return {"success": True, "message": f"Model {model_name} unloaded"}
    else:
        raise HTTPException(status_code=404, detail=f"Model {model_name} not loaded")

# ==================== Inference ====================

@router.post("/chat/completions")
async def chat_completion(request: ChatCompletionRequest):
    """OpenAI-compatible chat completion endpoint"""
    
    # Build prompt from messages
    prompt = ""
    for msg in request.messages:
        role = msg.get("role", "user")
        content = msg.get("content", "")
        if role == "system":
            prompt += f"System: {content}\n\n"
        elif role == "user":
            prompt += f"User: {content}\n\n"
        elif role == "assistant":
            prompt += f"Assistant: {content}\n\n"
    
    prompt += "Assistant: "
    
    try:
        if request.stream:
            # Streaming response
            def generate():
                try:
                    for token in model_manager.generate(
                        model_name=request.model,
                        prompt=prompt,
                        max_tokens=request.max_tokens,
                        temperature=request.temperature,
                        top_p=request.top_p,
                        top_k=request.top_k,
                        stream=True
                    ):
                        # OpenAI format
                        chunk = {
                            "choices": [{
                                "delta": {"content": token},
                                "index": 0,
                                "finish_reason": None
                            }]
                        }
                        yield f"data: {json.dumps(chunk)}\n\n"
                    
                    # Final chunk
                    final = {
                        "choices": [{
                            "delta": {},
                            "index": 0,
                            "finish_reason": "stop"
                        }]
                    }
                    yield f"data: {json.dumps(final)}\n\n"
                    yield "data: [DONE]\n\n"
                    
                except Exception as e:
                    error = {"error": str(e)}
                    yield f"data: {json.dumps(error)}\n\n"
            
            return StreamingResponse(
                generate(),
                media_type="text/event-stream"
            )
        else:
            # Non-streaming response
            text = ""
            for token in model_manager.generate(
                model_name=request.model,
                prompt=prompt,
                max_tokens=request.max_tokens,
                temperature=request.temperature,
                top_p=request.top_p,
                top_k=request.top_k,
                stream=True
            ):
                text += token
            
            return {
                "choices": [{
                    "message": {
                        "role": "assistant",
                        "content": text
                    },
                    "index": 0,
                    "finish_reason": "stop"
                }]
            }
    
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/completions")
async def completion(request: CompletionRequest):
    """Simple completion endpoint"""
    try:
        if request.stream:
            def generate():
                try:
                    for token in model_manager.generate(
                        model_name=request.model,
                        prompt=request.prompt,
                        max_tokens=request.max_tokens,
                        temperature=request.temperature,
                        top_p=request.top_p,
                        top_k=request.top_k,
                        stream=True
                    ):
                        yield token
                except Exception as e:
                    yield f"\n\n[ERROR: {str(e)}]"
            
            return StreamingResponse(
                generate(),
                media_type="text/plain"
            )
        else:
            text = ""
            for token in model_manager.generate(
                model_name=request.model,
                prompt=request.prompt,
                max_tokens=request.max_tokens,
                temperature=request.temperature,
                top_p=request.top_p,
                top_k=request.top_k,
                stream=True
            ):
                text += token
            
            return {"text": text}
    
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

# ==================== System Info ====================

@router.get("/system/info")
async def system_info():
    """Get system information"""
    import platform
    import psutil
    
    return {
        "platform": platform.system(),
        "platform_version": platform.version(),
        "python_version": platform.python_version(),
        "cpu_count": psutil.cpu_count(),
        "memory_total_gb": psutil.virtual_memory().total / (1024**3),
        "memory_available_gb": psutil.virtual_memory().available / (1024**3),
        "backend_memory_mb": model_manager.get_memory_usage()
    }

