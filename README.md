# RunMyModel Desktop v0.5.0-PRE-RELEASE

**A professional-grade local LLM desktop application built entirely in C++ with Qt6.**

## ✨ Features

- 🤖 **Local LLM Inference** - Run TinyLlama-1.1B locally with llama.cpp
- 🎨 **Modern Qt6 GUI** - Clean tabbed interface (Chat / Settings / Models)
- ⚡ **Real-time Streaming** - Token-by-token response generation with speed metrics
- 💾 **Chat History** - Save and clear conversations
- ⚙️ **Configurable Settings** - Adjust temperature, max tokens, and more
- 🎯 **Model Management** - Load/unload models, view model info
- 🚀 **GPU Acceleration** - Automatic CUDA support for NVIDIA GPUs
- 📊 **Performance Metrics** - Real-time tokens/second and total token count
- 🔒 **100% Offline** - Pure C++ with no cloud dependencies
- 🏗️ **Advanced Fine-tuning** - Built-in model training capabilities
- 🔧 **Professional Build System** - Complete packaging for distribution

## 🚀 Quick Start

### Requirements

- **Linux** (tested on Arch/CachyOS, Ubuntu, Fedora)
- **Qt6** (Widgets, Core, Gui, Concurrent)
- **GCC/G++** with C++17 support
- **llama.cpp** library (auto-downloaded by build scripts)
- **CUDA** (optional, for GPU acceleration)

### Installation & Run

#### Universal (Any Linux)
```bash
# Clone repository
git clone https://github.com/NAME9390/RunMyModel.git
cd RunMyModel

# Download TinyLlama model (first time only)
mkdir -p models
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf

# Run with ONE command
./run.sh
```

#### Arch Linux / CachyOS (Optimized)
```bash
# Clone repository
git clone https://github.com/NAME9390/RunMyModel.git
cd RunMyModel

# Download TinyLlama model (first time only)
mkdir -p models
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf

# Run with Arch-optimized script (auto-installs CUDA if you have NVIDIA GPU)
./run_arch.sh
```

**What's the difference?**
- `run.sh` - Universal script, works on any Linux distro
- `run_arch.sh` - Arch-optimized with:
  - Automatic CUDA installation for NVIDIA GPUs
  - Native compiler optimizations (`-march=native -O3`)
  - Better linker flags for performance
  - Automatic llama.cpp rebuild with GPU support

**Both scripts automatically:**
1. ✅ Check Qt6 and compiler
2. ✅ Clone and build llama.cpp (first run only)
3. ✅ Build C++ application
4. ✅ Launch GUI application
5. ✅ Enable GPU acceleration (if CUDA available)

## 📦 Executables Directory

This directory contains build scripts and tools for creating distributable executables for RunMyModel Desktop.

### Available Packages

#### AppImage (Linux)
- **File**: `RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage`
- **Platform**: Linux x86_64
- **Usage**: Portable executable that runs on most Linux distributions
- **Build Script**: `build_appimage.sh`

#### MSI (Windows)
- **File**: `RunMyModelDesktop-v0.5.0-PRE-RELEASE-x64.msi`
- **Platform**: Windows x64
- **Usage**: Windows installer package
- **Build Script**: `build_msi.sh`

#### EXE (Windows Demo)
- **File**: `RunMyModelDesktop-v0.5.0-PRE-RELEASE-x64.exe`
- **Platform**: Windows x64
- **Usage**: Simple Windows executable (demo version)

## 🛠️ Build Scripts

### Quick Build
```bash
# Build all packages
./build_all.sh

# Build only AppImage
./build_all.sh --appimage

# Build only MSI
./build_all.sh --msi
```

### Individual Builds
```bash
# Build AppImage
./build_appimage.sh

# Build MSI (requires Windows or Wine)
./build_msi.sh

# Setup cross-compilation environment
./setup_cross_compile.sh
```

## 📖 Usage

### 1. Chat Tab 💬

- **Type and Send**: Type your message and press Enter or click "Send"
- **Real-time Streaming**: Watch AI responses appear token-by-token
- **Stop Generation**: Click "Stop" to interrupt long responses
- **Save Chat**: Save conversation to a text file
- **Clear Chat**: Clear all messages and start fresh
- **Performance Metrics**: View tokens/second and total tokens in real-time

### 2. Settings Tab ⚙️

**Temperature**: Control response randomness (0.0 = deterministic, 1.0 = creative)
**Max Tokens**: Limit response length (1-2048 tokens)

### 3. Models Tab 📦

- **Current Model**: View loaded model information
- **Model Path**: See where the model is located
- **Load/Unload**: Manage model loading (auto-loads on startup)

## 🏗️ Architecture

```
┌─────────────────────────────────────────┐
│         C++ Qt6 GUI Application         │
│  ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │   Chat   │ │ Settings │ │ Models  │ │
│  └──────────┘ └──────────┘ └─────────┘ │
│                                         │
│              MainWindow                 │
│                   │                     │
│              ┌────▼────┐                │
│              │ Llama   │                │
│              │ Engine  │                │
│              └────┬────┘                │
└───────────────────┼─────────────────────┘
                    │
               ┌────▼────┐
               │llama.cpp│
               │ Library │
               └────┬────┘
                    │
        ┌───────────┴───────────┐
        │                       │
   ┌────▼────┐             ┌────▼────┐
   │   CPU   │             │   GPU   │
   │Inference│             │ (CUDA)  │
   └─────────┘             └─────────┘
```

**Pure C++ Stack:**
- **GUI**: Qt6 (Widgets, Core, Gui, Concurrent)
- **LLM Engine**: llama.cpp with GGUF model support
- **Threading**: QtConcurrent for non-blocking inference
- **State**: In-memory chat history and settings

## 📁 Project Structure

```
RunMyModel/
├── run.sh                   # ⭐ Universal launcher (wrapper)
├── run_arch.sh              # ⭐ Arch-optimized launcher (wrapper)
├── app/                     # Application code
│   ├── run.sh               # Actual run script
│   ├── run_arch.sh          # Actual Arch run script
│   ├── build.sh             # Build script
│   └── src-cpp/             # C++ source code
│       ├── include/
│       │   ├── mainwindow.h     # Main GUI window
│       │   └── llama_engine.h   # LLM inference engine
│       └── src/
│           ├── main.cpp         # Application entry point
│           ├── mainwindow.cpp   # GUI implementation
│           └── llama_engine.cpp # llama.cpp integration
├── models/                  # LLM models (.gguf)
│   └── tinyllama.gguf       # Download separately (not in repo)
├── build/                   # Build artifacts (gitignored)
│   └── RunMyModelDesktop    # Compiled executable
├── executables/             # Distribution packages
│   ├── RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage
│   ├── RunMyModelDesktop-v0.5.0-PRE-RELEASE-x64.exe
│   ├── RunMyModelDesktop-v0.5.0-PRE-RELEASE-x64.msi
│   └── build_*.sh           # Build scripts
└── lib/                     # External libraries (gitignored)
    └── llama.cpp/           # llama.cpp library (auto-cloned)
```

## 🛠️ Development

### Manual Build

```bash
# Build C++ application
./build.sh

# Run manually
export LD_LIBRARY_PATH="$(pwd)/lib/llama.cpp/build/bin:$LD_LIBRARY_PATH"
./build/RunMyModelDesktop
```

### Rebuild Application

```bash
# Clean rebuild
rm -rf build/
./run.sh  # or ./run_arch.sh
```

### GPU Acceleration Setup

**For NVIDIA GPUs (Arch Linux):**
```bash
# Run the Arch script - it auto-detects and sets up CUDA
./run_arch.sh
```

**Manual CUDA setup (other distros):**
```bash
# Install CUDA toolkit (Ubuntu example)
sudo apt install nvidia-cuda-toolkit

# Rebuild llama.cpp with CUDA
cd lib/llama.cpp
rm -rf build
mkdir build && cd build
cmake .. -DGGML_CUDA=ON
make -j$(nproc)
```

## 🐛 Troubleshooting

**Build fails:**
```bash
# Check Qt6 installation
pkg-config --modversion Qt6Core  # Universal
pacman -Qi qt6-base              # Arch

# Check GCC
g++ --version

# Rebuild llama.cpp
cd lib/llama.cpp
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**Model not found:**
```bash
# Download TinyLlama (~638MB)
mkdir -p models
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf
```

**GPU not being used:**
```bash
# On Arch, run:
./run_arch.sh  # Auto-installs CUDA and rebuilds llama.cpp

# On other distros, install CUDA manually and rebuild llama.cpp with:
cd lib/llama.cpp
rm -rf build && mkdir build && cd build
cmake .. -DGGML_CUDA=ON
make -j$(nproc)
```

**Application crashes:**
```bash
# Check logs
cat build.log

# Ensure model exists
ls -lh models/tinyllama.gguf

# Try rebuilding everything
rm -rf build/ lib/llama.cpp/
./run.sh
```

## 📊 Technical Details

- **LLM Engine:** llama.cpp with GGUF models
- **Model:** TinyLlama-1.1B-Chat-v1.0 (Q4_K_M quantization, ~638MB)
- **GUI Framework:** Qt6 (C++)
- **Threading:** QtConcurrent for non-blocking inference
- **GPU Support:** CUDA (automatic offloading with 99 layers)
- **Token Sampling:** llama.cpp sampler chain
- **Default Settings:** Temperature 0.7, Max Tokens 512

## 🔮 Future Plans

- [x] Session persistence (save/load conversations)
- [x] Multiple model support (switch between models)
- [x] System prompt customization
- [x] Dark/light theme toggle
- [x] Chat history browser
- [x] Model download manager (in-app)
- [x] Windows support
- [x] AppImage packaging
- [ ] RAG system (knowledge ingestion)
- [ ] API key integration (OpenAI, Anthropic)

## 📄 License

MPL 2.0 - See LICENSE file

## 🤝 Contributing

See CONTRIBUTING.md for guidelines.

---

**Built with ❤️ for the local AI community**

**Just run `./run.sh` (or `./run_arch.sh` on Arch) and you're ready to go!** 🚀