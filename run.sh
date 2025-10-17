#!/bin/bash

# RunMyModel Desktop - Universal Launcher Script
# This script handles building and running RunMyModel Desktop

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -d "app" ]; then
    print_error "Please run this script from the RunMyModel root directory"
    exit 1
fi

print_status "RunMyModel Desktop v0.6.0-PRE-RELEASE"
print_status "Starting build and launch process..."

# Check dependencies
print_status "Checking dependencies..."

# Check for Qt6
if ! pkg-config --exists Qt6Core; then
    print_error "Qt6 not found! Please install Qt6 development packages:"
    echo "  Ubuntu/Debian: sudo apt install qt6-base-dev qt6-tools-dev"
    echo "  Arch Linux: sudo pacman -S qt6-base qt6-tools"
    echo "  Fedora: sudo dnf install qt6-qtbase-devel qt6-qttools-devel"
    exit 1
fi

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found! Please install CMake:"
    echo "  Ubuntu/Debian: sudo apt install cmake"
    echo "  Arch Linux: sudo pacman -S cmake"
    echo "  Fedora: sudo dnf install cmake"
    exit 1
fi

# Check for GCC/G++
if ! command -v g++ &> /dev/null; then
    print_error "G++ not found! Please install build tools:"
    echo "  Ubuntu/Debian: sudo apt install build-essential"
    echo "  Arch Linux: sudo pacman -S base-devel"
    echo "  Fedora: sudo dnf install gcc-c++"
    exit 1
fi

print_success "All dependencies found!"

# Build llama.cpp if needed
if [ ! -f "app/lib/llama.cpp/build/libllama.a" ]; then
    print_status "Building llama.cpp library..."
    cd app/lib/llama.cpp
    
    if [ ! -d "build" ]; then
        mkdir build
    fi
    
    cd build
    
    # Configure with CUDA support if available
    if command -v nvcc &> /dev/null; then
        print_status "CUDA detected, enabling GPU acceleration..."
        cmake .. -DCMAKE_BUILD_TYPE=Release -DGGML_CUDA=ON
    else
        print_warning "CUDA not found, building CPU-only version"
        cmake .. -DCMAKE_BUILD_TYPE=Release
    fi
    
    make -j$(nproc)
    cd ../../
    print_success "llama.cpp built successfully!"
else
    print_success "llama.cpp already built!"
fi

# Build the application
print_status "Building RunMyModel Desktop application..."
cd app
./build.sh
cd ../

# Check if build was successful
if [ ! -f "app/build/RunMyModelDesktop" ]; then
    print_error "Build failed! Check the output above for errors."
    exit 1
fi

print_success "Build completed successfully!"

# Create executables directory structure
print_status "Creating executables..."

# Copy the built executable to executables directory
cp app/build/RunMyModelDesktop executables/RunMyModelDesktop-v0.6.0-PRE-RELEASE-x86_64.AppImage

# Create a simple Windows executable placeholder (not a real executable)
echo "Windows executable placeholder - build with MinGW-w64 for actual Windows support" > executables/RunMyModelDesktop-v0.6.0-PRE-RELEASE-x64.exe

# Create MSI placeholder
echo "Windows MSI installer placeholder - use WiX Toolset for actual MSI creation" > executables/RunMyModelDesktop-v0.6.0-PRE-RELEASE-x64.msi

print_success "Executables created!"

# Download model if not present
if [ ! -f "models/tinyllama.gguf" ]; then
    print_status "Downloading TinyLlama model (638MB)..."
    mkdir -p models
    
    # Try to download the model
    if wget -O models/tinyllama.gguf "https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf"; then
        print_success "Model downloaded successfully!"
    else
        print_warning "Failed to download model automatically."
        print_warning "Please download manually:"
        echo "  mkdir -p models"
        echo "  wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf"
    fi
else
    print_success "Model already present!"
fi

# Launch the application
print_status "Launching RunMyModel Desktop..."
print_success "Enjoy using RunMyModel Desktop! 🚀"

# Run the application
exec app/build/RunMyModelDesktop
