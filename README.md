# RunMyModel Desktop

<div align="center">

![RunMyModel Desktop](https://img.shields.io/badge/RunMyModel-Desktop-blue?style=for-the-badge&logo=qt)
![Version](https://img.shields.io/badge/version-0.6.0--PRE--RELEASE-green?style=for-the-badge)
![License](https://img.shields.io/badge/license-MPL--2.0-orange?style=for-the-badge)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-lightgrey?style=for-the-badge)

**Professional-grade local LLM desktop application**

[![GitHub stars](https://img.shields.io/github/stars/NAME9390/RunMyModel?style=social)](https://github.com/NAME9390/RunMyModel/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/NAME9390/RunMyModel?style=social)](https://github.com/NAME9390/RunMyModel/network/members)
[![GitHub issues](https://img.shields.io/github/issues/NAME9390/RunMyModel)](https://github.com/NAME9390/RunMyModel/issues)
[![GitHub pull requests](https://img.shields.io/github/issues-pr/NAME9390/RunMyModel)](https://github.com/NAME9390/RunMyModel/pulls)

</div>

---

## ✨ **Features**

- 🤖 **Local LLM Inference** - Run models locally without cloud dependencies
- 🎨 **Modern Qt6 GUI** - Clean, responsive interface with tabbed layout
- ⚡ **Real-time Streaming** - Token-by-token response generation with speed metrics
- 🚀 **GPU Acceleration** - Automatic CUDA support for NVIDIA GPUs
- 💾 **Session Management** - Save and load chat conversations
- ⚙️ **Configurable Settings** - Adjust temperature, max tokens, and more
- 🎯 **Model Management** - Load/unload models, view model information
- 📊 **Performance Metrics** - Real-time tokens/second and total token count
- 🔒 **100% Offline** - Complete privacy and security
- 🔌 **Plugin System** - Extensible architecture for custom functionality
- 📚 **RAG System** - Knowledge ingestion and retrieval capabilities

## 🚀 **Quick Start**

### Prerequisites

- **Linux** (tested on Arch/CachyOS, Ubuntu, Fedora)
- **Qt6** (Widgets, Core, Gui, Concurrent)
- **GCC/G++** with C++17 support
- **CMake** (for llama.cpp)
- **CUDA** (optional, for GPU acceleration)

### Installation

#### Universal Linux
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

# Run with Arch-optimized script
./run_arch.sh
```

**What's the difference?**
- `run.sh` - Universal script, works on any Linux distro
- `run_arch.sh` - Arch-optimized with automatic CUDA installation and native optimizations

## 📖 **Usage**

### Chat Interface
- **Type and Send**: Type your message and press Enter or click "Send"
- **Real-time Streaming**: Watch AI responses appear token-by-token
- **Stop Generation**: Click "Stop" to interrupt long responses
- **Save Chat**: Save conversation to a text file
- **Clear Chat**: Clear all messages and start fresh

### Settings
- **Temperature**: Control response randomness (0.0 = deterministic, 1.0 = creative)
- **Max Tokens**: Limit response length (1-2048 tokens)
- **GPU Acceleration**: Enable/disable CUDA acceleration

### Model Management
- **Current Model**: View loaded model information
- **Model Path**: See where the model is located
- **Load/Unload**: Manage model loading (auto-loads on startup)

## 🏗️ **Architecture**

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

## 📁 **Project Structure**

```
RunMyModel/
├── run.sh                   # ⭐ Universal launcher
├── run_arch.sh              # ⭐ Arch-optimized launcher
├── build.sh                 # Build helper script
├── app/                     # Main application directory
│   ├── src-cpp/             # C++ source code
│   │   ├── include/         # Header files
│   │   └── src/             # Source files
│   ├── models/              # LLM models (.gguf)
│   ├── sessions/             # Chat session storage
│   ├── memory/               # RAG system storage
│   └── config/               # Configuration files
├── docs/                     # Documentation
│   ├── user/                 # User documentation
│   ├── developer/            # Developer documentation
│   ├── features/             # Feature documentation
│   ├── quick-start/          # Quick start guides
│   └── support/              # Support documentation
├── executables/              # Pre-built executables
└── lib/
    └── llama.cpp/            # llama.cpp library (auto-cloned)
```

## 🛠️ **Development**

### Manual Build
```bash
# Build C++ application
./build.sh

# Run manually
export LD_LIBRARY_PATH="$(pwd)/lib/llama.cpp/build/bin:$LD_LIBRARY_PATH"
./build/RunMyModelDesktop
```

### GPU Acceleration Setup
```bash
# On Arch, run:
./run_arch.sh  # Auto-installs CUDA and rebuilds llama.cpp

# On other distros, install CUDA manually and rebuild llama.cpp with:
cd lib/llama.cpp
rm -rf build && mkdir build && cd build
cmake .. -DGGML_CUDA=ON
make -j$(nproc)
```

## 🐛 **Troubleshooting**

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

## 📊 **Technical Details**

- **LLM Engine:** llama.cpp with GGUF models
- **Model:** TinyLlama-1.1B-Chat-v1.0 (Q4_K_M quantization, ~638MB)
- **GUI Framework:** Qt6 (C++)
- **Threading:** QtConcurrent for non-blocking inference
- **GPU Support:** CUDA (automatic offloading with 99 layers)
- **Token Sampling:** llama.cpp sampler chain
- **Default Settings:** Temperature 0.7, Max Tokens 512

## 🔮 **Roadmap**

- [ ] Session persistence (save/load conversations)
- [ ] Multiple model support (switch between models)
- [ ] System prompt customization
- [ ] Dark/light theme toggle
- [ ] Chat history browser
- [ ] Model download manager (in-app)
- [ ] Windows support
- [ ] AppImage packaging
- [ ] RAG system (knowledge ingestion)
- [ ] API key integration (OpenAI, Anthropic)

## 📄 **License**

MPL 2.0 - See LICENSE file

## 🤝 **Contributing**

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## 📚 **Documentation**

- [User Guide](docs/user/installation.md) - Installation and usage
- [Developer Guide](docs/developer/README.md) - Development setup
- [Features](docs/features/README.md) - Feature documentation
- [Quick Start](docs/quick-start/README.md) - Getting started
- [Support](docs/support/README.md) - Troubleshooting and support

---

<div align="center">

**Built with ❤️ for the local AI community**

[![GitHub](https://img.shields.io/badge/GitHub-NAME9390/RunMyModel-blue?style=flat-square&logo=github)](https://github.com/NAME9390/RunMyModel)
[![License](https://img.shields.io/badge/License-MPL--2.0-orange?style=flat-square)](LICENSE)

*Making AI accessible, one desktop at a time* 🚀

</div>
