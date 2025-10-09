# RunMyModel 0.3.0 BETA - Architecture Document

## 🎯 Vision

**RunMyModel 0.3.0 BETA**: A Prompt Architect and Model Runner hybrid desktop application.

Transform from "simple LLM launcher" → "Visual prompt design tool + Local AI execution platform"

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                      RunMyModel Desktop 0.3.0 BETA              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │          C++ Qt6 Frontend (Cross-Platform UI)            │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │  • Visual Prompt Architect                               │  │
│  │  • Open WebUI-inspired Design                            │  │
│  │  • Model Browser & Manager                               │  │
│  │  • Chat Interface with Streaming                         │  │
│  │  • Settings & Configuration                              │  │
│  └───────────────────┬──────────────────────────────────────┘  │
│                      │                                          │
│                      │ REST API / gRPC                          │
│                      │ (HTTP on localhost:5000)                 │
│                      ▼                                          │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │       Python FastAPI Backend (AI Logic)                  │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │  • llama-cpp-python (GGUF Model Loading)                 │  │
│  │  • Prompt Template Engine                                │  │
│  │  • Model Management (list, load, unload)                 │  │
│  │  • Inference Engine (streaming generation)               │  │
│  │  • Configuration Management                              │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              File System Storage                         │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │  ~/Documents/rmm/                                        │  │
│  │  ├── models/          (GGUF files)                       │  │
│  │  ├── prompts/         (Saved prompt templates)           │  │
│  │  ├── chats/           (Chat history)                     │  │
│  │  └── config.json      (App configuration)                │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## 🧩 Component Details

### 1. C++ Qt6 Frontend

**Responsibilities:**
- Modern, responsive UI (Open WebUI-inspired)
- Visual prompt architect (drag-drop blocks)
- Real-time chat interface
- Model browser with GPU filtering
- Download manager with progress
- Settings panel

**Key Files:**
- `src-cpp/main.cpp` - Application entry
- `src-cpp/ui/mainwindow.cpp` - Main window
- `src-cpp/ui/prompt_architect.cpp` - Visual prompt builder
- `src-cpp/ui/chat_window.cpp` - Chat interface
- `src-cpp/api/backend_client.cpp` - Python backend API client

### 2. Python FastAPI Backend

**Responsibilities:**
- GGUF model loading via llama-cpp-python
- Text generation with streaming
- Prompt template management
- Model metadata and caching
- Configuration management

**Key Files:**
```
backend-python/
├── main.py              # FastAPI server entry
├── models/
│   ├── loader.py        # Model loading/unloading
│   └── inference.py     # Text generation
├── prompts/
│   ├── templates.py     # Prompt template engine
│   └── architect.py     # Prompt builder logic
├── api/
│   ├── routes.py        # REST endpoints
│   └── schemas.py       # Pydantic models
└── requirements.txt     # Python dependencies
```

### 3. Bridge Layer

**Options:**
1. **REST API** (Recommended for 0.3.0)
   - Python FastAPI backend on `localhost:5000`
   - C++ uses `QNetworkAccessManager` for HTTP requests
   - Simple, debuggable, cross-platform

2. **gRPC** (Future consideration)
   - Higher performance
   - Bidirectional streaming
   - More complex setup

## 🎨 UI Design (Open WebUI-inspired)

### Main Interface Layout

```
┌───────────────────────────────────────────────────────────────────┐
│  RunMyModel 0.3.0 BETA                               [_ □ ✕]       │
├─────────────────────────────────────────────────────────────────┬─┤
│                                                                 │ │
│  ┌─────────────────────────────────────────────────────────┐   │ │
│  │                    Prompt Architect                      │   │ │
│  │  ┌──────────────────────────────────────────────────┐   │   │ │
│  │  │  🧱 System Prompt                                 │   │   │ │
│  │  │     You are a helpful AI assistant specialized   │   │   │ │
│  │  │     in [SELECT: coding, writing, analysis]       │   │   │ │
│  │  └──────────────────────────────────────────────────┘   │   │ │
│  │  ┌──────────────────────────────────────────────────┐   │   │ │
│  │  │  💬 User Context                                  │   │   │ │
│  │  │     Task: [INPUT]                                │   │   │ │
│  │  │     Format: [DROPDOWN]                           │   │   │ │
│  │  └──────────────────────────────────────────────────┘   │   │ │
│  │  ┌──────────────────────────────────────────────────┐   │   │ │
│  │  │  ⚙️ Parameters                                    │   │   │ │
│  │  │     Temperature: [SLIDER: 0.7]                   │   │   │ │
│  │  │     Max Tokens: [INPUT: 2048]                    │   │   │ │
│  │  └──────────────────────────────────────────────────┘   │   │ │
│  │                                                          │   │ │
│  │  [▶ Generate]  [💾 Save Template]  [🔄 Reset]          │   │ │
│  └─────────────────────────────────────────────────────────┘   │ │
│                                                                 │ │
│  ┌─────────────────────────────────────────────────────────┐   │ │
│  │                    Chat Output                           │   │ │
│  │  ┌────────────────────────────────────────────────────┐  │   │ │
│  │  │  👤 You: Write a Python function...               │  │   │ │
│  │  │                                                    │  │   │ │
│  │  │  🤖 Assistant: Here's a Python function that...   │  │   │ │
│  │  │  ```python                                         │  │   │ │
│  │  │  def example():                                    │  │   │ │
│  │  │      return "Hello, World!"                        │  │   │ │
│  │  │  ```                                               │  │   │ │
│  │  └────────────────────────────────────────────────────┘  │   │ │
│  │  [Input...]                              [Send 🚀]       │   │ │
│  └─────────────────────────────────────────────────────────┘   │ │
│                                                                 │ │
└─────────────────────────────────────────────────────────────────┴─┘
```

## 📋 API Endpoints (Python Backend)

### Model Management
- `GET /api/models` - List available models
- `GET /api/models/loaded` - List loaded models
- `POST /api/models/load` - Load a model into memory
- `POST /api/models/unload` - Unload a model
- `GET /api/models/{id}/info` - Get model metadata

### Inference
- `POST /api/chat/completions` - Generate text (streaming)
- `POST /api/completions` - Single completion
- `GET /api/chat/history` - Get chat history
- `DELETE /api/chat/history/{id}` - Clear chat

### Prompts
- `GET /api/prompts/templates` - List prompt templates
- `POST /api/prompts/templates` - Save new template
- `GET /api/prompts/templates/{id}` - Get specific template
- `POST /api/prompts/build` - Build prompt from components

### System
- `GET /api/system/info` - GPU, RAM, disk info
- `GET /api/system/health` - Backend health check

## 🔧 Key Features for 0.3.0 BETA

### ✅ Must Have
1. **Visual Prompt Architect**
   - Template-based prompt building
   - Drag-drop components
   - Save/load templates

2. **Real Model Execution**
   - llama-cpp-python integration
   - Streaming text generation
   - Token-by-token display

3. **Model Management**
   - Browse Hugging Face models
   - Download GGUF files
   - Load/unload models dynamically

4. **Modern UI**
   - Open WebUI-inspired design
   - Dark/light theme
   - Responsive layout

5. **Cross-Platform Builds**
   - Linux .AppImage
   - Windows .exe
   - Bundled Python runtime

### 🔄 Nice to Have (Post-0.3.0)
- Multi-modal support (images, audio)
- Fine-tuning interface
- Model comparison tool
- API key management for external services
- Plugin system

## 📦 Packaging Strategy

### Linux .AppImage
```bash
# Bundle:
- RunMyModelDesktop (C++ binary)
- Python 3.11 runtime
- backend-python/ (all Python files)
- llama.cpp shared libraries
- Qt6 libraries

# Structure:
AppDir/
├── usr/
│   ├── bin/RunMyModelDesktop
│   ├── lib/python3.11/
│   ├── lib/libQt6*.so
│   └── share/applications/runmymodel.desktop
└── AppRun (launch script)
```

### Windows .exe
```bash
# Using PyInstaller + NSIS:
- RunMyModelDesktop.exe (C++ binary)
- python311.dll + embedded Python
- backend-python/ (frozen)
- Qt6 DLLs
- llama.cpp DLLs

# Installer structure:
RunMyModel-0.3.0-Setup.exe
├── RunMyModelDesktop.exe
├── python/
├── backend/
└── uninstall.exe
```

## 🚀 Implementation Roadmap

### Phase 1: Backend Foundation (Week 1)
1. Create Python FastAPI backend
2. Integrate llama-cpp-python
3. Implement basic inference endpoint
4. Test with sample model

### Phase 2: Frontend Redesign (Week 2)
1. Redesign main window layout
2. Create prompt architect UI
3. Implement HTTP client for backend
4. Wire up chat interface

### Phase 3: Integration (Week 3)
1. Connect frontend to backend
2. Implement streaming responses
3. Add model loading UI
4. Test end-to-end flow

### Phase 4: Polish & Package (Week 4)
1. Add prompt templates
2. Improve error handling
3. Create .AppImage build
4. Create .exe build
5. Write user documentation

## 🎯 Success Criteria for 0.3.0 BETA

- ✅ User can visually build prompts
- ✅ User can run GGUF models locally
- ✅ Chat interface streams responses token-by-token
- ✅ Works on Linux and Windows
- ✅ Self-contained executables (no manual Python install)
- ✅ Modern, intuitive UI
- ✅ GPU acceleration works (if available)

## 📝 Notes

- Keep alpha features (model browser, downloads) but integrate into new arch
- Python backend should be a subprocess launched by C++ on startup
- Communication via REST API on localhost (simple, debuggable)
- All AI logic in Python, all UI logic in C++
- Version 0.3.0 is "feature complete beta" - ready for user testing

