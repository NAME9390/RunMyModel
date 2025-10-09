"""
Visual prompt architect - Build prompts from blocks
"""

from typing import List, Dict, Optional
from dataclasses import dataclass, asdict
import json

@dataclass
class PromptBlock:
    """Represents a single block in a prompt"""
    id: str
    type: str  # system, user, context, instruction, output_format
    content: str
    variables: List[str] = None
    order: int = 0
    
    def to_dict(self) -> dict:
        return asdict(self)

class PromptArchitect:
    """Visual prompt builder"""
    
    def __init__(self):
        self.blocks: List[PromptBlock] = []
        self.variables: Dict[str, str] = {}
    
    def add_block(self, block_type: str, content: str, variables: Optional[List[str]] = None) -> str:
        """Add a new block to the prompt"""
        block_id = f"block_{len(self.blocks)}"
        block = PromptBlock(
            id=block_id,
            type=block_type,
            content=content,
            variables=variables or [],
            order=len(self.blocks)
        )
        self.blocks.append(block)
        return block_id
    
    def remove_block(self, block_id: str) -> bool:
        """Remove a block"""
        self.blocks = [b for b in self.blocks if b.id != block_id]
        # Reorder
        for i, block in enumerate(self.blocks):
            block.order = i
        return True
    
    def update_block(self, block_id: str, content: str) -> bool:
        """Update block content"""
        for block in self.blocks:
            if block.id == block_id:
                block.content = content
                return True
        return False
    
    def reorder_blocks(self, block_ids: List[str]):
        """Reorder blocks"""
        new_blocks = []
        for block_id in block_ids:
            for block in self.blocks:
                if block.id == block_id:
                    new_blocks.append(block)
                    break
        
        self.blocks = new_blocks
        for i, block in enumerate(self.blocks):
            block.order = i
    
    def set_variable(self, name: str, value: str):
        """Set a variable value"""
        self.variables[name] = value
    
    def build(self) -> str:
        """Build final prompt from blocks"""
        sorted_blocks = sorted(self.blocks, key=lambda b: b.order)
        
        prompt_parts = []
        for block in sorted_blocks:
            content = block.content
            
            # Variable substitution
            for var_name, var_value in self.variables.items():
                content = content.replace(f"{{{var_name}}}", var_value)
            
            # Format based on type
            if block.type == "system":
                prompt_parts.append(f"System: {content}")
            elif block.type == "user":
                prompt_parts.append(f"User: {content}")
            elif block.type == "context":
                prompt_parts.append(f"Context: {content}")
            elif block.type == "instruction":
                prompt_parts.append(f"Instruction: {content}")
            elif block.type == "output_format":
                prompt_parts.append(f"Output Format: {content}")
            else:
                prompt_parts.append(content)
        
        return "\n\n".join(prompt_parts)
    
    def to_dict(self) -> dict:
        """Export to dictionary"""
        return {
            "blocks": [block.to_dict() for block in self.blocks],
            "variables": self.variables
        }
    
    def from_dict(self, data: dict):
        """Import from dictionary"""
        self.blocks = [
            PromptBlock(**block_data)
            for block_data in data.get("blocks", [])
        ]
        self.variables = data.get("variables", {})
    
    def save(self, filepath: str) -> bool:
        """Save prompt to file"""
        try:
            with open(filepath, 'w') as f:
                json.dump(self.to_dict(), f, indent=2)
            return True
        except Exception as e:
            print(f"❌ Failed to save prompt: {e}")
            return False
    
    def load(self, filepath: str) -> bool:
        """Load prompt from file"""
        try:
            with open(filepath, 'r') as f:
                data = json.load(f)
            self.from_dict(data)
            return True
        except Exception as e:
            print(f"❌ Failed to load prompt: {e}")
            return False

