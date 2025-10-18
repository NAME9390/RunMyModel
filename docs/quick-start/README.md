# Quick Start Guide

<div align="center">

![RunMyModel Desktop](https://img.shields.io/badge/RunMyModel-Desktop-blue?style=for-the-badge&logo=qt)

**Get up and running with RunMyModel Desktop in minutes**

</div>

---

## 🚀 **Installation**

### Prerequisites

Before installing RunMyModel Desktop, ensure you have:

- **Linux** (tested on Arch/CachyOS, Ubuntu, Fedora)
- **Qt6** (Widgets, Core, Gui, Concurrent)
- **GCC/G++** with C++17 support
- **CMake** (for llama.cpp)
- **CUDA** (optional, for GPU acceleration)

### Quick Installation

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
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0.GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf

# Run with Arch-optimized script
./run_arch.sh
```

**What's the difference?**
- `run.sh` - Universal script, works on any Linux distro
- `run_arch.sh` - Arch-optimized with automatic CUDA installation and native optimizations

## 🎯 **First Run**

### What Happens on First Run

When you run the script for the first time:

1. ✅ **Dependencies Check**: Verifies Qt6, GCC, and CMake
2. ✅ **llama.cpp Setup**: Clones and builds llama.cpp library
3. ✅ **Application Build**: Compiles C++ application with Qt6
4. ✅ **GPU Detection**: Detects and enables CUDA if available
5. ✅ **Launch Application**: Starts the GUI application

### Expected Output

```bash
$ ./run.sh
Checking dependencies...
✅ Qt6 found: 6.5.0
✅ GCC found: 12.2.0
✅ CMake found: 3.25.0

Setting up llama.cpp...
✅ Cloning llama.cpp repository
✅ Building llama.cpp with CUDA support
✅ llama.cpp build complete

Building application...
✅ Compiling C++ application
✅ Application build complete

Launching RunMyModel Desktop...
🚀 Application started successfully!
```

## 💬 **Basic Usage**

### Chat Interface

1. **Start a Conversation**:
   - Type your message in the input field
   - Press Enter or click "Send"
   - Watch the AI response appear token-by-token

2. **Stop Generation**:
   - Click "Stop" to interrupt long responses
   - Useful for stopping unwanted or long responses

3. **Save Chat**:
   - Click "Save Chat" to save your conversation
   - Conversations are saved to the `sessions/` directory

4. **Clear Chat**:
   - Click "Clear Chat" to start a new conversation
   - Clears all messages from the current session

### Settings Configuration

1. **Temperature Control**:
   - **0.0**: Deterministic responses (always the same)
   - **0.7**: Balanced creativity (default)
   - **1.0**: High creativity (more varied responses)

2. **Max Tokens**:
   - **512**: Short responses (default)
   - **1024**: Medium responses
   - **2048**: Long responses

3. **GPU Acceleration**:
   - **Enabled**: Use GPU for faster inference (if available)
   - **Disabled**: Use CPU only

### Model Management

1. **View Model Info**:
   - Check the Models tab for current model information
   - See model size, parameters, and quantization level

2. **Load/Unload Model**:
   - Models are automatically loaded on startup
   - Use the Models tab to unload or reload models

## 🔧 **Configuration**

### Basic Settings

Edit the configuration file to customize behavior:

```bash
# Edit configuration
nano app/config/default_config.json
```

**Configuration Options:**
```json
{
  "model": {
    "path": "models/tinyllama.gguf",
    "temperature": 0.7,
    "max_tokens": 512
  },
  "gpu": {
    "enabled": true,
    "layers": 99
  },
  "ui": {
    "theme": "light",
    "font_size": 12
  }
}
```

### Advanced Settings

For advanced users, additional configuration options:

```json
{
  "inference": {
    "batch_size": 1,
    "threads": 4,
    "context_size": 2048
  },
  "memory": {
    "cache_size": 1024,
    "swap_size": 512
  },
  "logging": {
    "level": "info",
    "file": "logs/app.log"
  }
}
```

## 🐛 **Troubleshooting**

### Common Issues

#### Build Fails
```bash
# Check Qt6 installation
pkg-config --modversion Qt6Core

# Check GCC
g++ --version

# Rebuild llama.cpp
cd lib/llama.cpp
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### Model Not Found
```bash
# Download TinyLlama model
mkdir -p models
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf
```

#### GPU Not Working
```bash
# On Arch, run:
./run_arch.sh  # Auto-installs CUDA and rebuilds llama.cpp

# On other distros, install CUDA manually and rebuild llama.cpp with:
cd lib/llama.cpp
rm -rf build && mkdir build && cd build
cmake .. -DGGML_CUDA=ON
make -j$(nproc)
```

#### Application Crashes
```bash
# Check logs
cat build.log

# Ensure model exists
ls -lh models/tinyllama.gguf

# Try rebuilding everything
rm -rf build/ lib/llama.cpp/
./run.sh
```

### Performance Issues

#### Slow Inference
- **Enable GPU**: Use CUDA acceleration if available
- **Reduce Max Tokens**: Lower the maximum token limit
- **Use Quantized Models**: Use Q4_K_M or Q8_0 quantized models
- **Close Other Applications**: Free up system resources

#### High Memory Usage
- **Use Smaller Models**: Switch to smaller models
- **Reduce Context Size**: Lower the context window
- **Enable Memory Optimization**: Use memory-efficient settings

## 📚 **Next Steps**

### Learn More

- **User Manual**: Complete user guide
- **Developer Guide**: Development and customization
- **Features**: Detailed feature documentation
- **API Reference**: Programmatic interface

### Community

- **GitHub Issues**: Report bugs or request features
- **Discussions**: Community discussions and support
- **Contributing**: How to contribute to the project

### Advanced Usage

- **Custom Models**: Using your own models
- **Plugin Development**: Creating custom plugins
- **RAG System**: Knowledge ingestion and retrieval
- **API Integration**: Integrating with external services

## 🎉 **Congratulations!**

You've successfully installed and started using RunMyModel Desktop! 

**What you can do now:**
- ✅ Chat with AI locally
- ✅ Adjust settings for your preferences
- ✅ Save and load conversations
- ✅ Use GPU acceleration (if available)
- ✅ Explore advanced features

**Need help?**
- Check the [User Manual](user/manual.md) for detailed usage
- Visit [Troubleshooting](user/troubleshooting.md) for common issues
- Join our [Community](https://github.com/NAME9390/RunMyModel/discussions) for support

---

<div align="center">

**Built with ❤️ for the local AI community**

[![GitHub](https://img.shields.io/badge/GitHub-NAME9390/RunMyModel-blue?style=flat-square&logo=github)](https://github.com/NAME9390/RunMyModel)
[![License](https://img.shields.io/badge/License-MPL--2.0-orange?style=flat-square)](LICENSE)

*Making AI accessible, one desktop at a time* 🚀

</div>
