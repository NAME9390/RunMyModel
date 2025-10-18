#!/bin/bash

# RunMyModel Desktop - Universal Launcher Script
# Works on any Linux distribution

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}✅${NC} $1"
}

print_info() {
    echo -e "${BLUE}ℹ️${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠️${NC} $1"
}

print_error() {
    echo -e "${RED}❌${NC} $1"
}

print_header() {
    echo -e "${BLUE}🚀${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check dependencies
check_dependencies() {
    print_header "Checking dependencies..."
    
    # Check Qt6
    if command_exists pkg-config && pkg-config --exists Qt6Core; then
        QT_VERSION=$(pkg-config --modversion Qt6Core)
        print_status "Qt6 found: $QT_VERSION"
    else
        print_error "Qt6 not found. Please install Qt6 development packages."
        echo "Ubuntu/Debian: sudo apt install qt6-base-dev qt6-tools-dev"
        echo "Arch Linux: sudo pacman -S qt6-base qt6-tools"
        echo "Fedora: sudo dnf install qt6-qtbase-devel qt6-qttools-devel"
        exit 1
    fi
    
    # Check GCC
    if command_exists g++; then
        GCC_VERSION=$(g++ --version | head -n1)
        print_status "GCC found: $GCC_VERSION"
    else
        print_error "GCC not found. Please install GCC with C++17 support."
        exit 1
    fi
    
    # Check CMake
    if command_exists cmake; then
        CMAKE_VERSION=$(cmake --version | head -n1)
        print_status "CMake found: $CMAKE_VERSION"
    else
        print_error "CMake not found. Please install CMake."
        exit 1
    fi
    
    # Check Git
    if command_exists git; then
        GIT_VERSION=$(git --version)
        print_status "Git found: $GIT_VERSION"
    else
        print_error "Git not found. Please install Git."
        exit 1
    fi
}

# Function to setup llama.cpp
setup_llama_cpp() {
    print_header "Setting up llama.cpp..."
    
    # Create lib directory if it doesn't exist
    mkdir -p lib
    
    # Check if llama.cpp already exists
    if [ -d "lib/llama.cpp" ]; then
        print_info "llama.cpp already exists, updating..."
        cd lib/llama.cpp
        git pull
        cd ../..
    else
        print_info "Cloning llama.cpp repository..."
        cd lib
        git clone https://github.com/ggerganov/llama.cpp.git
        cd ..
    fi
    
    # Build llama.cpp
    print_info "Building llama.cpp..."
    cd lib/llama.cpp
    
    # Create build directory
    rm -rf build
    mkdir build
    cd build
    
    # Configure with CMake
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release"
    
    # Check for CUDA
    if command_exists nvcc; then
        print_info "CUDA found, enabling GPU acceleration..."
        CMAKE_ARGS="$CMAKE_ARGS -DGGML_CUDA=ON"
    else
        print_warning "CUDA not found, using CPU only"
    fi
    
    # Configure and build
    cmake .. $CMAKE_ARGS
    make -j$(nproc)
    
    print_status "llama.cpp build complete"
    cd ../../..
}

# Function to build application
build_application() {
    print_header "Building application..."
    
    # Create build directory
    mkdir -p build
    
    # Set library path
    export LD_LIBRARY_PATH="$(pwd)/lib/llama.cpp/build/bin:$LD_LIBRARY_PATH"
    
    # Build application
    cd build
    
    # Generate Makefile with qmake
    qmake6 ../app/src-cpp/RunMyModelDesktop.pro
    
    # Compile
    make -j$(nproc)
    
    print_status "Application build complete"
    cd ..
}

# Function to check model
check_model() {
    print_header "Checking model..."
    
    # Create models directory if it doesn't exist
    mkdir -p models
    
    # Check if model exists
    if [ -f "models/tinyllama.gguf" ]; then
        print_status "Model found: models/tinyllama.gguf"
    else
        print_warning "Model not found. Please download TinyLlama model:"
        echo "mkdir -p models"
        echo "wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf"
        echo ""
        echo "Or run the application and it will prompt you to download the model."
    fi
}

# Function to launch application
launch_application() {
    print_header "Launching RunMyModel Desktop..."
    
    # Set library path
    export LD_LIBRARY_PATH="$(pwd)/lib/llama.cpp/build/bin:$LD_LIBRARY_PATH"
    
    # Check if executable exists
    if [ -f "build/RunMyModelDesktop" ]; then
        print_status "Application started successfully!"
        ./build/RunMyModelDesktop
    else
        print_error "Executable not found. Build may have failed."
        exit 1
    fi
}

# Main execution
main() {
    echo "RunMyModel Desktop - Universal Launcher"
    echo "======================================"
    echo ""
    
    # Check dependencies
    check_dependencies
    
    # Setup llama.cpp
    setup_llama_cpp
    
    # Build application
    build_application
    
    # Check model
    check_model
    
    # Launch application
    launch_application
}

# Run main function
main "$@"
