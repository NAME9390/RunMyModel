"""
Configuration management for RunMyModel backend
"""

import os
from pathlib import Path
import json

class Config:
    """Global configuration"""
    
    def __init__(self):
        # Base directory: ~/Documents/rmm/
        self.base_dir = Path.home() / "Documents" / "rmm"
        self.models_dir = self.base_dir / "models"
        self.prompts_dir = self.base_dir / "prompts"
        self.chats_dir = self.base_dir / "chats"
        self.config_file = self.base_dir / "config.json"
        
        # Default settings
        self.default_settings = {
            "max_models_loaded": 2,
            "default_context_size": 4096,
            "default_temperature": 0.7,
            "default_top_p": 0.9,
            "default_top_k": 40,
            "gpu_layers": -1,  # -1 = auto (use all available)
            "threads": os.cpu_count(),
        }
        
        self.settings = self.load_settings()
    
    def load_settings(self) -> dict:
        """Load settings from config file"""
        if self.config_file.exists():
            try:
                with open(self.config_file, 'r') as f:
                    loaded = json.load(f)
                    return {**self.default_settings, **loaded}
            except Exception as e:
                print(f"⚠️ Failed to load config: {e}")
        
        return self.default_settings.copy()
    
    def save_settings(self):
        """Save current settings to file"""
        self.base_dir.mkdir(parents=True, exist_ok=True)
        with open(self.config_file, 'w') as f:
            json.dump(self.settings, f, indent=2)
    
    def get(self, key: str, default=None):
        """Get a setting value"""
        return self.settings.get(key, default)
    
    def set(self, key: str, value):
        """Set a setting value"""
        self.settings[key] = value
        self.save_settings()

