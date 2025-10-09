"""
Prompt template engine and management
Provides pre-built prompt structures for different use cases
"""

from typing import Dict, List, Optional
from pathlib import Path
import json
import yaml

class PromptTemplate:
    """Represents a prompt template"""
    
    def __init__(self, name: str, description: str, category: str, blocks: List[dict]):
        self.name = name
        self.description = description
        self.category = category
        self.blocks = blocks
    
    def to_dict(self) -> dict:
        return {
            "name": self.name,
            "description": self.description,
            "category": self.category,
            "blocks": self.blocks
        }
    
    def build_prompt(self, variables: Dict[str, str]) -> str:
        """Build final prompt from template blocks with variable substitution"""
        prompt_parts = []
        
        for block in self.blocks:
            block_type = block.get("type", "text")
            content = block.get("content", "")
            
            # Variable substitution
            for var_name, var_value in variables.items():
                content = content.replace(f"{{{var_name}}}", var_value)
            
            if block_type == "system":
                prompt_parts.append(f"System: {content}")
            elif block_type == "user":
                prompt_parts.append(f"User: {content}")
            elif block_type == "context":
                prompt_parts.append(f"Context: {content}")
            elif block_type == "instruction":
                prompt_parts.append(f"Instruction: {content}")
            else:
                prompt_parts.append(content)
        
        return "\n\n".join(prompt_parts)

class PromptTemplateManager:
    """Manages prompt templates"""
    
    def __init__(self, templates_dir: Optional[Path] = None):
        if templates_dir is None:
            templates_dir = Path.home() / "Documents" / "rmm" / "prompts"
        
        self.templates_dir = templates_dir
        self.templates_dir.mkdir(parents=True, exist_ok=True)
        
        # Load built-in templates
        self.templates: Dict[str, PromptTemplate] = {}
        self._load_builtin_templates()
        self._load_custom_templates()
    
    def _load_builtin_templates(self):
        """Load built-in prompt templates"""
        
        # Coding Assistant Template
        self.templates["coding_assistant"] = PromptTemplate(
            name="Coding Assistant",
            description="Help with coding tasks, debugging, and code review",
            category="Coding",
            blocks=[
                {
                    "type": "system",
                    "content": "You are an expert software engineer specializing in {language}. "
                              "You write clean, efficient, and well-documented code."
                },
                {
                    "type": "context",
                    "content": "Programming Language: {language}\n"
                              "Task Type: {task_type}\n"
                              "Code Style: {code_style}"
                },
                {
                    "type": "user",
                    "content": "{user_request}"
                },
                {
                    "type": "instruction",
                    "content": "Provide code with inline comments. "
                              "Explain your approach and any important decisions."
                }
            ]
        )
        
        # Writing Assistant Template
        self.templates["writing_assistant"] = PromptTemplate(
            name="Writing Assistant",
            description="Help with creative writing, essays, and content creation",
            category="Writing",
            blocks=[
                {
                    "type": "system",
                    "content": "You are a professional writer specializing in {writing_type}. "
                              "Your writing is {tone} and engaging."
                },
                {
                    "type": "context",
                    "content": "Writing Type: {writing_type}\n"
                              "Target Audience: {audience}\n"
                              "Tone: {tone}\n"
                              "Length: {length}"
                },
                {
                    "type": "user",
                    "content": "{user_request}"
                },
                {
                    "type": "instruction",
                    "content": "Create compelling content that matches the specified tone and audience. "
                              "Use proper formatting and structure."
                }
            ]
        )
        
        # Data Analysis Template
        self.templates["data_analysis"] = PromptTemplate(
            name="Data Analyst",
            description="Analyze data, create visualizations, and provide insights",
            category="Analysis",
            blocks=[
                {
                    "type": "system",
                    "content": "You are a data analyst expert. You analyze data, identify patterns, "
                              "and provide actionable insights."
                },
                {
                    "type": "context",
                    "content": "Data Type: {data_type}\n"
                              "Analysis Goal: {goal}\n"
                              "Output Format: {output_format}"
                },
                {
                    "type": "user",
                    "content": "{user_request}"
                },
                {
                    "type": "instruction",
                    "content": "Provide clear analysis with supporting evidence. "
                              "Include visualizations or code where appropriate."
                }
            ]
        )
        
        # General Assistant Template
        self.templates["general_assistant"] = PromptTemplate(
            name="General Assistant",
            description="General-purpose AI assistant for various tasks",
            category="General",
            blocks=[
                {
                    "type": "system",
                    "content": "You are a helpful AI assistant. You provide accurate, "
                              "well-reasoned responses to user questions."
                },
                {
                    "type": "user",
                    "content": "{user_request}"
                }
            ]
        )
        
        # Debugging Template
        self.templates["debugging"] = PromptTemplate(
            name="Debug Helper",
            description="Help debug code and fix errors",
            category="Coding",
            blocks=[
                {
                    "type": "system",
                    "content": "You are a debugging expert. You analyze code, identify issues, "
                              "and suggest fixes."
                },
                {
                    "type": "context",
                    "content": "Language: {language}\n"
                              "Error Message: {error_message}\n"
                              "Code Context:\n{code_context}"
                },
                {
                    "type": "instruction",
                    "content": "1. Identify the root cause of the error\n"
                              "2. Explain why it's happening\n"
                              "3. Provide a fix with explanation\n"
                              "4. Suggest how to prevent similar issues"
                }
            ]
        )
        
        # Documentation Template
        self.templates["documentation"] = PromptTemplate(
            name="Documentation Writer",
            description="Generate clear technical documentation",
            category="Writing",
            blocks=[
                {
                    "type": "system",
                    "content": "You are a technical writer. You create clear, comprehensive "
                              "documentation for {doc_type}."
                },
                {
                    "type": "context",
                    "content": "Documentation Type: {doc_type}\n"
                              "Target Readers: {readers}\n"
                              "Format: {format}"
                },
                {
                    "type": "user",
                    "content": "{user_request}"
                },
                {
                    "type": "instruction",
                    "content": "Create well-structured documentation with:\n"
                              "- Clear headings and sections\n"
                              "- Code examples where relevant\n"
                              "- Usage instructions\n"
                              "- Common pitfalls and tips"
                }
            ]
        )
    
    def _load_custom_templates(self):
        """Load user-created custom templates"""
        templates_file = self.templates_dir / "custom_templates.json"
        
        if templates_file.exists():
            try:
                with open(templates_file, 'r') as f:
                    custom_data = json.load(f)
                
                for template_data in custom_data.get("templates", []):
                    template = PromptTemplate(
                        name=template_data["name"],
                        description=template_data["description"],
                        category=template_data["category"],
                        blocks=template_data["blocks"]
                    )
                    self.templates[template_data["name"].lower().replace(" ", "_")] = template
            
            except Exception as e:
                print(f"⚠️ Failed to load custom templates: {e}")
    
    def get_template(self, template_id: str) -> Optional[PromptTemplate]:
        """Get a template by ID"""
        return self.templates.get(template_id)
    
    def list_templates(self, category: Optional[str] = None) -> List[dict]:
        """List all templates, optionally filtered by category"""
        templates_list = []
        
        for template_id, template in self.templates.items():
            if category is None or template.category == category:
                templates_list.append({
                    "id": template_id,
                    "name": template.name,
                    "description": template.description,
                    "category": template.category
                })
        
        return templates_list
    
    def get_categories(self) -> List[str]:
        """Get list of all template categories"""
        categories = set()
        for template in self.templates.values():
            categories.add(template.category)
        return sorted(list(categories))
    
    def save_custom_template(self, template: PromptTemplate) -> bool:
        """Save a user-created template"""
        try:
            templates_file = self.templates_dir / "custom_templates.json"
            
            # Load existing
            custom_data = {"templates": []}
            if templates_file.exists():
                with open(templates_file, 'r') as f:
                    custom_data = json.load(f)
            
            # Add new template
            template_id = template.name.lower().replace(" ", "_")
            custom_data["templates"].append(template.to_dict())
            
            # Save
            with open(templates_file, 'w') as f:
                json.dump(custom_data, f, indent=2)
            
            # Add to memory
            self.templates[template_id] = template
            
            return True
        
        except Exception as e:
            print(f"❌ Failed to save template: {e}")
            return False
    
    def build_prompt(self, template_id: str, variables: Dict[str, str]) -> Optional[str]:
        """Build a prompt from a template with variables"""
        template = self.get_template(template_id)
        if template:
            return template.build_prompt(variables)
        return None

