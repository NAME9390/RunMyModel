#!/usr/bin/env python3
"""
RunMyModel 0.3.0 BETA - Python FastAPI Backend
Main entry point for the AI backend server
"""

import os
import sys
from pathlib import Path
from fastapi import FastAPI, WebSocket
from fastapi.middleware.cors import CORSMiddleware
import uvicorn

# Add backend-python to path
sys.path.insert(0, str(Path(__file__).parent))

from api import routes
from models.loader import ModelManager
from utils.config import Config

# Initialize FastAPI app
app = FastAPI(
    title="RunMyModel Backend",
    description="AI Model Management and Inference Backend",
    version="0.3.0-BETA"
)

# Add CORS middleware for local development
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Allow all origins (it's localhost only)
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Initialize global model manager
model_manager = ModelManager()
config = Config()

# Include API routes
app.include_router(routes.router, prefix="/api")

@app.on_event("startup")
async def startup_event():
    """Initialize backend on startup"""
    print("🚀 RunMyModel Backend 0.3.0 BETA Starting...")
    print(f"📁 Models directory: {config.models_dir}")
    print(f"📁 Config directory: {config.config_dir}")
    
    # Ensure directories exist
    config.models_dir.mkdir(parents=True, exist_ok=True)
    config.prompts_dir.mkdir(parents=True, exist_ok=True)
    config.chats_dir.mkdir(parents=True, exist_ok=True)
    
    # Scan for available models
    available = model_manager.scan_available_models()
    print(f"✅ Found {len(available)} available models")

@app.on_event("shutdown")
async def shutdown_event():
    """Cleanup on shutdown"""
    print("🛑 Shutting down backend...")
    model_manager.unload_all()
    print("✅ All models unloaded")

@app.get("/")
async def root():
    """Root endpoint"""
    return {
        "name": "RunMyModel Backend",
        "version": "0.3.0-BETA",
        "status": "running"
    }

@app.get("/health")
async def health_check():
    """Health check endpoint"""
    return {
        "status": "healthy",
        "models_loaded": len(model_manager.loaded_models),
        "memory_usage_mb": model_manager.get_memory_usage()
    }

if __name__ == "__main__":
    # Run the server
    uvicorn.run(
        "main:app",
        host="127.0.0.1",
        port=5000,
        reload=False,  # Disable in production
        log_level="info"
    )

