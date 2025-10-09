"""
Model loading and management using llama-cpp-python
"""

import os
from pathlib import Path
from typing import Dict, List, Optional
import psutil

try:
    from llama_cpp import Llama
    LLAMA_CPP_AVAILABLE = True
except ImportError:
    print("⚠️ llama-cpp-python not installed. Install with: pip install llama-cpp-python")
    LLAMA_CPP_AVAILABLE = False
    Llama = None

class ModelInfo:
    """Model metadata"""
    def __init__(self, name: str, path: Path, size_mb: int):
        self.name = name
        self.path = path
        self.size_mb = size_mb
        self.loaded = False
        self.context_size = 4096
        self.gpu_layers = -1

class ModelManager:
    """Manages model loading/unloading and inference"""
    
    def __init__(self):
        self.loaded_models: Dict[str, Llama] = {}
        self.available_models: Dict[str, ModelInfo] = {}
        self.models_dir = Path.home() / "Documents" / "rmm"
    
    def scan_available_models(self) -> List[dict]:
        """Scan for GGUF models in ~/Documents/rmm/"""
        self.available_models.clear()
        
        if not self.models_dir.exists():
            return []
        
        # Look for .gguf files in all subdirectories
        for model_dir in self.models_dir.iterdir():
            if model_dir.is_dir():
                # Check for model.gguf in subdirectory
                gguf_file = model_dir / "model.gguf"
                if gguf_file.exists():
                    size_mb = gguf_file.stat().st_size // (1024 * 1024)
                    model_info = ModelInfo(
                        name=model_dir.name,
                        path=gguf_file,
                        size_mb=size_mb
                    )
                    self.available_models[model_dir.name] = model_info
        
        # Also check for direct .gguf files in models_dir
        for gguf_file in self.models_dir.glob("*.gguf"):
            if gguf_file.is_file():
                size_mb = gguf_file.stat().st_size // (1024 * 1024)
                model_info = ModelInfo(
                    name=gguf_file.stem,
                    path=gguf_file,
                    size_mb=size_mb
                )
                self.available_models[gguf_file.stem] = model_info
        
        return [
            {
                "name": info.name,
                "path": str(info.path),
                "size_mb": info.size_mb,
                "loaded": info.name in self.loaded_models
            }
            for info in self.available_models.values()
        ]
    
    def load_model(
        self, 
        model_name: str,
        n_ctx: int = 4096,
        n_gpu_layers: int = -1,
        verbose: bool = False
    ) -> bool:
        """Load a model into memory"""
        
        if not LLAMA_CPP_AVAILABLE:
            raise RuntimeError("llama-cpp-python not available")
        
        if model_name in self.loaded_models:
            print(f"✅ Model {model_name} already loaded")
            return True
        
        if model_name not in self.available_models:
            raise ValueError(f"Model {model_name} not found in available models")
        
        model_info = self.available_models[model_name]
        
        try:
            print(f"📥 Loading model: {model_name}")
            print(f"   Path: {model_info.path}")
            print(f"   Size: {model_info.size_mb} MB")
            print(f"   Context: {n_ctx}")
            print(f"   GPU Layers: {n_gpu_layers}")
            
            # Load with llama.cpp
            llm = Llama(
                model_path=str(model_info.path),
                n_ctx=n_ctx,
                n_gpu_layers=n_gpu_layers,
                verbose=verbose,
                n_threads=os.cpu_count(),
            )
            
            self.loaded_models[model_name] = llm
            model_info.loaded = True
            
            print(f"✅ Model {model_name} loaded successfully!")
            return True
            
        except Exception as e:
            print(f"❌ Failed to load model {model_name}: {e}")
            raise
    
    def unload_model(self, model_name: str) -> bool:
        """Unload a model from memory"""
        if model_name not in self.loaded_models:
            return False
        
        print(f"🗑️ Unloading model: {model_name}")
        del self.loaded_models[model_name]
        
        if model_name in self.available_models:
            self.available_models[model_name].loaded = False
        
        print(f"✅ Model {model_name} unloaded")
        return True
    
    def unload_all(self):
        """Unload all models"""
        model_names = list(self.loaded_models.keys())
        for name in model_names:
            self.unload_model(name)
    
    def get_model(self, model_name: str) -> Optional[Llama]:
        """Get a loaded model instance"""
        return self.loaded_models.get(model_name)
    
    def generate(
        self,
        model_name: str,
        prompt: str,
        max_tokens: int = 512,
        temperature: float = 0.7,
        top_p: float = 0.9,
        top_k: int = 40,
        stream: bool = True
    ):
        """Generate text from a loaded model"""
        
        if model_name not in self.loaded_models:
            raise ValueError(f"Model {model_name} not loaded")
        
        llm = self.loaded_models[model_name]
        
        # Generate with streaming
        response = llm(
            prompt,
            max_tokens=max_tokens,
            temperature=temperature,
            top_p=top_p,
            top_k=top_k,
            stream=stream,
            echo=False
        )
        
        if stream:
            # Return generator for streaming
            for chunk in response:
                if 'choices' in chunk and len(chunk['choices']) > 0:
                    text = chunk['choices'][0].get('text', '')
                    if text:
                        yield text
        else:
            # Return full response
            if 'choices' in response and len(response['choices']) > 0:
                return response['choices'][0]['text']
            return ""
    
    def get_memory_usage(self) -> int:
        """Get current memory usage in MB"""
        process = psutil.Process()
        return process.memory_info().rss // (1024 * 1024)

