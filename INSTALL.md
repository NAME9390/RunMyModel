# RunMyModel Desktop 0.3.0 BETA - Installation Guide

## 🚀 Quick Start

### Prerequisites

**Linux:**
- Qt6 (Widgets, Network, Core, Gui)
- Python 3.10+
- GCC/G++ compiler
- Make, qmake

**Windows:**
- Qt6 (same modules)
- Python 3.10+
- MSVC or MinGW
- Visual Studio Build Tools

### Installation Steps

#### 1. Install System Dependencies

**Ubuntu/Debian:**
```bash
sudo apt install build-essential qt6-base-dev python3 python3-pip python3-venv
```

**Arch Linux:**
```bash
sudo pacman -S base-devel qt6-base python python-pip
```

**Windows:**
- Install Python from python.org
- Install Qt6 from qt.io
- Install Visual Studio Build Tools

#### 2. Clone Repository

```bash
git clone https://github.com/runmymodel/runmymodel-desktop
cd runmymodel-desktop
```

#### 3. Install Python Dependencies

```bash
# Create virtual environment (recommended)
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r backend-python/requirements.txt
```

**Important:** `llama-cpp-python` will compile llama.cpp on installation. This may take a few minutes.

For GPU acceleration (NVIDIA CUDA):
```bash
CMAKE_ARGS="-DLLAMA_CUBLAS=on" pip install llama-cpp-python --force-reinstall --no-cache-dir
```

For GPU acceleration (AMD ROCm):
```bash
CMAKE_ARGS="-DLLAMA_HIPBLAS=on" pip install llama-cpp-python --force-reinstall --no-cache-dir
```

For Apple Metal (macOS):
```bash
CMAKE_ARGS="-DLLAMA_METAL=on" pip install llama-cpp-python --force-reinstall --no-cache-dir
```

#### 4. Build C++ Frontend

```bash
# Generate Makefile
qmake6 RunMyModelDesktop.pro

# Or on some systems:
qmake RunMyModelDesktop.pro

# Compile
make -j$(nproc)
```

#### 5. Run the Application

```bash
./build/RunMyModelDesktop
```

The Python backend will start automatically on `localhost:5000`.

## 📦 Directory Structure

After setup, your directory should look like:

```
runmymodel-desktop/
├── backend-python/          # Python FastAPI backend
│   ├── main.py              # Server entry point
│   ├── models/              # Model management
│   ├── api/                 # REST API routes
│   ├── utils/               # Configuration
│   └── requirements.txt     # Python dependencies
├── src-cpp/                 # C++ Qt6 frontend
│   ├── src/                 # Implementation files
│   └── include/             # Header files
├── build/                   # Compiled binaries
│   └── RunMyModelDesktop    # Main executable
├── llms.txt                 # Model catalog
└── ~/Documents/rmm/         # User data (created on first run)
    ├── models/              # Downloaded GGUF models
    ├── prompts/             # Saved prompts
    ├── chats/               # Chat history
    └── config.json          # App configuration
```

## 🧪 Testing

### 1. Test Python Backend

```bash
cd backend-python
python main.py
```

Visit http://localhost:5000 - should show:
```json
{
  "name": "RunMyModel Backend",
  "version": "0.3.0-BETA",
  "status": "running"
}
```

### 2. Test C++ Frontend

```bash
./build/RunMyModelDesktop
```

Check console output:
```
🚀 RunMyModel Desktop v0.3.0-BETA
🔧 Architecture: C++ Qt6 Frontend + Python FastAPI Backend
🧠 AI Engine: llama.cpp (GGUF models)
📡 Starting Python backend...
✅ Backend is ready!
```

### 3. Download a Test Model

1. Go to "Browse Models"
2. Search for a small model (e.g., "TinyLlama")
3. Click "Download"
4. Wait for completion

### 4. Test Inference

1. Go to "Chat"
2. Select the downloaded model
3. Type a message
4. Watch for streaming response!

## 🐛 Troubleshooting

### Python Backend Fails to Start

**Error:** `ModuleNotFoundError: No module named 'fastapi'`

**Solution:**
```bash
pip install -r backend-python/requirements.txt
```

### llama-cpp-python Installation Fails

**Error:** `CMake not found`

**Solution (Ubuntu):**
```bash
sudo apt install cmake
```

**Solution (Windows):**
- Install CMake from cmake.org
- Add to PATH

### Qt6 Not Found

**Error:** `Could not find qmake spec 'linux-g++'`

**Solution:**
```bash
# Find your Qt installation
which qmake6
# Use full path
/usr/bin/qmake6 RunMyModelDesktop.pro
```

### Backend Timeout

**Error:** `Python backend took too long to start`

**Possible causes:**
1. Python dependencies not installed
2. Port 5000 already in use
3. Firewall blocking localhost

**Solution:**
```bash
# Check port
lsof -i :5000

# Kill conflicting process
kill <PID>

# Test backend manually
cd backend-python && python main.py
```

### Model Download Fails

**Error:** `No GGUF file found`

**Cause:** Model doesn't have GGUF quantizations on Hugging Face

**Solution:**
- Use "Custom Models" to add local GGUF files
- Download GGUF manually from HuggingFace
- Try a different model

### GPU Not Detected

**Error:** `No GPU detected or VRAM info unavailable`

**Solution (NVIDIA):**
```bash
# Install nvidia-smi
sudo apt install nvidia-utils

# Test
nvidia-smi
```

**Solution (AMD):**
```bash
# Install rocm-smi
sudo apt install rocm-smi

# Test
rocm-smi
```

## 📚 Advanced Configuration

### Custom Python Path

If Python backend doesn't start, edit `src-cpp/src/backend_client.cpp`:

```cpp
QString BackendClient::findPythonExecutable()
{
    return "/full/path/to/python3";
}
```

### Custom Backend Port

Edit `backend-python/main.py`:

```python
uvicorn.run("main:app", host="127.0.0.1", port=5001)
```

And `src-cpp/src/backend_client.cpp`:

```cpp
m_baseUrl = "http://127.0.0.1:5001";
```

### Model Storage Location

Models are stored in `~/Documents/rmm/`.

To change, edit `backend-python/utils/config.py`:

```python
self.base_dir = Path("/your/custom/path")
```

## 🎯 Next Steps

- Read `ARCHITECTURE_0.3.0.md` for technical details
- Check `DEVELOPMENT.md` for contribution guide
- Report issues on GitHub

## 📄 License

MIT License - See LICENSE file

## 🙏 Credits

- [llama.cpp](https://github.com/ggerganov/llama.cpp) - Core inference engine
- [FastAPI](https://fastapi.tiangolo.com/) - Python backend framework
- [Qt6](https://www.qt.io/) - C++ UI framework
- [Hugging Face](https://huggingface.co/) - Model repository

---

**Need Help?**
- Open an issue: https://github.com/runmymodel/runmymodel-desktop/issues
- Discussions: https://github.com/runmymodel/runmymodel-desktop/discussions

