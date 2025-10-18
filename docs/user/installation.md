# Installation Guide

This guide will help you install RunMyModel Desktop on your system.

## 📋 **System Requirements**

### Minimum Requirements
- **OS**: Linux (Ubuntu 18.04+, Arch Linux, Fedora) or Windows 10+
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 2GB free space
- **CPU**: x86_64 architecture

### Recommended Requirements
- **OS**: Linux (Ubuntu 20.04+, Arch Linux, Fedora) or Windows 11
- **RAM**: 16GB or more
- **Storage**: 10GB free space
- **GPU**: NVIDIA GPU with CUDA support
- **CPU**: Multi-core processor

## 🐧 **Linux Installation**

### Method 1: AppImage (Recommended)

The AppImage is a portable executable that works on most Linux distributions.

#### Download
```bash
# Download the latest AppImage
wget https://github.com/NAME9390/RunMyModel/releases/latest/download/RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage

# Make it executable
chmod +x RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage
```

#### Run
```bash
# Run the application
./RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage
```

#### Install (Optional)
```bash
# Install to system
sudo mv RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage /usr/local/bin/runmymodel
sudo chmod +x /usr/local/bin/runmymodel

# Create desktop entry
cat > ~/.local/share/applications/runmymodel.desktop << EOF
[Desktop Entry]
Name=RunMyModel Desktop
Comment=Local LLM Desktop Application
Exec=/usr/local/bin/runmymodel
Icon=runmymodel
Terminal=false
Type=Application
Categories=Office;
EOF
```

### Method 2: Build from Source

#### Ubuntu/Debian
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake git
sudo apt install qt6-base-dev qt6-tools-dev qt6-tools-dev-tools
sudo apt install libgl1-mesa-dev libglu1-mesa-dev

# Clone and build
git clone https://github.com/NAME9390/RunMyModel.git
cd RunMyModel
./run.sh
```

#### Arch Linux/CachyOS
```bash
# Install dependencies
sudo pacman -S base-devel cmake git
sudo pacman -S qt6-base qt6-tools qt6-tools-dev
sudo pacman -S mesa

# Clone and build
git clone https://github.com/NAME9390/RunMyModel.git
cd RunMyModel
./run_arch.sh
```

#### Fedora
```bash
# Install dependencies
sudo dnf install gcc-c++ cmake git
sudo dnf install qt6-qtbase-devel qt6-qttools-devel
sudo dnf install mesa-libGL-devel mesa-libGLU-devel

# Clone and build
git clone https://github.com/NAME9390/RunMyModel.git
cd RunMyModel
./run.sh
```

## 🪟 **Windows Installation**

### Method 1: MSI Installer (Recommended)

#### Download and Install
1. Download `RunMyModelDesktop-v0.5.0-PRE-RELEASE-x64.msi` from [releases](https://github.com/NAME9390/RunMyModel/releases)
2. Double-click the MSI file
3. Follow the installation wizard
4. RunMyModel Desktop will be installed to Program Files
5. Desktop and Start Menu shortcuts will be created

### Method 2: Portable EXE

#### Download and Run
1. Download `RunMyModelDesktop-v0.5.0-PRE-RELEASE-x64.exe`
2. Double-click to run (no installation required)
3. Note: This is a demo version with limited functionality

### Method 3: Build from Source

#### Prerequisites
- Visual Studio 2019+ with C++ support
- Qt6 for Windows
- CMake
- Git

#### Build Steps
```cmd
# Clone repository
git clone https://github.com/NAME9390/RunMyModel.git
cd RunMyModel

# Build (Windows build script)
cd app
build.bat
```

## 🍎 **macOS Installation**

### Method 1: Build from Source

#### Prerequisites
- Xcode Command Line Tools
- Qt6 for macOS
- CMake
- Git

#### Build Steps
```bash
# Install dependencies
brew install cmake git
brew install qt6

# Clone and build
git clone https://github.com/NAME9390/RunMyModel.git
cd RunMyModel
./run.sh
```

## 🚀 **GPU Acceleration Setup**

### NVIDIA CUDA (Linux)

#### Ubuntu/Debian
```bash
# Install CUDA toolkit
sudo apt install nvidia-cuda-toolkit

# Verify installation
nvcc --version
nvidia-smi
```

#### Arch Linux
```bash
# Install CUDA
sudo pacman -S cuda

# Verify installation
nvcc --version
nvidia-smi
```

#### Rebuild with CUDA Support
```bash
# Rebuild llama.cpp with CUDA
cd lib/llama.cpp
rm -rf build
mkdir build && cd build
cmake .. -DGGML_CUDA=ON
make -j$(nproc)
```

### NVIDIA CUDA (Windows)

#### Install CUDA Toolkit
1. Download CUDA Toolkit from [NVIDIA](https://developer.nvidia.com/cuda-downloads)
2. Install following the wizard
3. Verify installation in Command Prompt:
   ```cmd
   nvcc --version
   nvidia-smi
   ```

#### Rebuild with CUDA Support
```cmd
# Rebuild llama.cpp with CUDA
cd lib\llama.cpp
rmdir /s build
mkdir build
cd build
cmake .. -DGGML_CUDA=ON
cmake --build . --config Release
```

## 📦 **Model Installation**

### Download TinyLlama Model

#### Linux/macOS
```bash
# Create models directory
mkdir -p models

# Download TinyLlama model (~638MB)
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf
```

#### Windows
```cmd
# Create models directory
mkdir models

# Download TinyLlama model
curl -L -o models/tinyllama.gguf https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf
```

### Other Models

You can use any GGUF format model. Popular options include:

- **Llama 2**: Various sizes available
- **Code Llama**: For code generation
- **Mistral**: High-quality responses
- **Phi**: Microsoft's model

## ✅ **Verification**

### Test Installation

#### Linux/macOS
```bash
# Run the application
./RunMyModelDesktop

# Or if installed system-wide
runmymodel
```

#### Windows
```cmd
# Run from Start Menu or Desktop shortcut
# Or from Command Prompt
cd "C:\Program Files\RunMyModel Desktop"
RunMyModelDesktop.exe
```

### Check GPU Acceleration

1. Open RunMyModel Desktop
2. Go to Settings tab
3. Check if GPU acceleration is enabled
4. Look for CUDA information in the Models tab

## 🔧 **Troubleshooting**

### Common Issues

#### AppImage Won't Run
```bash
# Make sure it's executable
chmod +x RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage

# Check dependencies
ldd RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage
```

#### Build Fails
```bash
# Check Qt6 installation
pkg-config --modversion Qt6Core

# Check compiler
g++ --version

# Clean build
rm -rf build/
./run.sh
```

#### Model Not Found
```bash
# Check model file
ls -la models/tinyllama.gguf

# Download if missing
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf
```

#### GPU Not Working
```bash
# Check CUDA installation
nvcc --version
nvidia-smi

# Rebuild with CUDA
cd lib/llama.cpp
rm -rf build && mkdir build && cd build
cmake .. -DGGML_CUDA=ON
make -j$(nproc)
```

## 📞 **Support**

If you encounter issues during installation:

1. Check the [Troubleshooting Guide](troubleshooting.md)
2. Search [GitHub Issues](https://github.com/NAME9390/RunMyModel/issues)
3. Create a new issue with installation details
4. Join [GitHub Discussions](https://github.com/NAME9390/RunMyModel/discussions)

---

**Next Steps**: After installation, see the [Quick Start Guide](quickstart.md) to get started with RunMyModel Desktop.
