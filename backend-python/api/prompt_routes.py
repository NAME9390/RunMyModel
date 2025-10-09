"""
API routes for prompt templates and architect
"""

from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
from typing import List, Dict, Optional

from prompts.templates import PromptTemplateManager, PromptTemplate
from prompts.architect import PromptArchitect, PromptBlock

router = APIRouter()

# Global instances
template_manager = PromptTemplateManager()
architects: Dict[str, PromptArchitect] = {}  # Session-based architects

# Pydantic models
class PromptBlockCreate(BaseModel):
    type: str
    content: str
    variables: Optional[List[str]] = []

class PromptBuildRequest(BaseModel):
    template_id: str
    variables: Dict[str, str]

class PromptArchitectSession(BaseModel):
    session_id: str

class PromptVariableSet(BaseModel):
    name: str
    value: str

# ==================== Templates ====================

@router.get("/prompts/templates")
async def list_templates(category: Optional[str] = None):
    """List all available prompt templates"""
    templates = template_manager.list_templates(category)
    categories = template_manager.get_categories()
    
    return {
        "templates": templates,
        "categories": categories,
        "count": len(templates)
    }

@router.get("/prompts/templates/{template_id}")
async def get_template(template_id: str):
    """Get a specific template"""
    template = template_manager.get_template(template_id)
    
    if not template:
        raise HTTPException(status_code=404, detail=f"Template {template_id} not found")
    
    return template.to_dict()

@router.post("/prompts/build")
async def build_prompt(request: PromptBuildRequest):
    """Build a prompt from a template"""
    prompt = template_manager.build_prompt(request.template_id, request.variables)
    
    if prompt is None:
        raise HTTPException(status_code=404, detail=f"Template {request.template_id} not found")
    
    return {"prompt": prompt}

# ==================== Visual Architect ====================

@router.post("/prompts/architect/session")
async def create_architect_session():
    """Create a new prompt architect session"""
    import uuid
    session_id = str(uuid.uuid4())
    architects[session_id] = PromptArchitect()
    
    return {"session_id": session_id}

@router.post("/prompts/architect/{session_id}/block")
async def add_block(session_id: str, block: PromptBlockCreate):
    """Add a block to the prompt"""
    if session_id not in architects:
        raise HTTPException(status_code=404, detail="Session not found")
    
    architect = architects[session_id]
    block_id = architect.add_block(block.type, block.content, block.variables)
    
    return {
        "block_id": block_id,
        "prompt": architect.build()
    }

@router.delete("/prompts/architect/{session_id}/block/{block_id}")
async def remove_block(session_id: str, block_id: str):
    """Remove a block from the prompt"""
    if session_id not in architects:
        raise HTTPException(status_code=404, detail="Session not found")
    
    architect = architects[session_id]
    success = architect.remove_block(block_id)
    
    return {
        "success": success,
        "prompt": architect.build()
    }

@router.put("/prompts/architect/{session_id}/block/{block_id}")
async def update_block(session_id: str, block_id: str, content: str):
    """Update a block's content"""
    if session_id not in architects:
        raise HTTPException(status_code=404, detail="Session not found")
    
    architect = architects[session_id]
    success = architect.update_block(block_id, content)
    
    return {
        "success": success,
        "prompt": architect.build()
    }

@router.post("/prompts/architect/{session_id}/variable")
async def set_variable(session_id: str, variable: PromptVariableSet):
    """Set a variable value"""
    if session_id not in architects:
        raise HTTPException(status_code=404, detail="Session not found")
    
    architect = architects[session_id]
    architect.set_variable(variable.name, variable.value)
    
    return {
        "prompt": architect.build()
    }

@router.get("/prompts/architect/{session_id}")
async def get_architect_state(session_id: str):
    """Get current state of prompt architect"""
    if session_id not in architects:
        raise HTTPException(status_code=404, detail="Session not found")
    
    architect = architects[session_id]
    
    return {
        "session_id": session_id,
        "blocks": architect.to_dict()["blocks"],
        "variables": architect.to_dict()["variables"],
        "prompt": architect.build()
    }

@router.post("/prompts/architect/{session_id}/reorder")
async def reorder_blocks(session_id: str, block_ids: List[str]):
    """Reorder blocks"""
    if session_id not in architects:
        raise HTTPException(status_code=404, detail="Session not found")
    
    architect = architects[session_id]
    architect.reorder_blocks(block_ids)
    
    return {
        "prompt": architect.build()
    }

