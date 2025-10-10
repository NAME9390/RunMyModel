#!/bin/bash
# RunMyModel 0.4.0 - Pure C++ Self-Learning LLM Platform
# No Python dependencies!

set -e
cd "$(dirname "$0")"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${MAGENTA}🧠 RunMyModel 0.4.0 - Pure C++ Platform${NC}"
echo -e "${CYAN}   100% C++ - No Python Required!${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Cleanup
echo -e "${YELLOW}🧹 Cleaning up existing processes...${NC}"
pkill -9 -f "RunMyModelDesktop" 2>/dev/null || true
sleep 1
echo -e "${GREEN}✅ Cleanup complete${NC}"
echo ""

# Check requirements
echo -e "${YELLOW}🔍 Checking requirements...${NC}"

if ! command -v g++ &> /dev/null; then
    echo -e "${RED}❌ G++ not found!${NC}"
    exit 1
fi
echo -e "${GREEN}✅ G++: $(g++ --version | head -1 | cut -d' ' -f3)${NC}"

if ! pkg-config --exists Qt6Core 2>/dev/null; then
    echo -e "${RED}❌ Qt6 not found!${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Qt6: $(pkg-config --modversion Qt6Core)${NC}"

# Check GPU
if command -v nvidia-smi &> /dev/null; then
    GPU_NAME=$(nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null | head -1)
    echo -e "${MAGENTA}🎮 GPU Detected: $GPU_NAME${NC}"
    
    if command -v nvcc &> /dev/null; then
        CUDA_VERSION=$(nvcc --version | grep release | awk '{print $5}' | sed 's/,//')
        echo -e "${GREEN}✅ CUDA: $CUDA_VERSION${NC}"
    fi
fi

echo ""

# Build llama.cpp with CUDA if GPU detected
if command -v nvidia-smi &> /dev/null && command -v nvcc &> /dev/null; then
    if [ ! -f "lib/llama.cpp/build/bin/libggml-cuda.so" ]; then
        echo -e "${MAGENTA}🔥 Rebuilding llama.cpp with CUDA support...${NC}"
        
        cd lib/llama.cpp
        rm -rf build
        mkdir build && cd build
        
        export CMAKE_ROOT=/usr/share/cmake
        cmake .. -DCMAKE_BUILD_TYPE=Release -DGGML_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=native
        make -j$(nproc)
        
        cd ../../..
        echo -e "${GREEN}✅ llama.cpp built with CUDA!${NC}"
    fi
fi

# Build application
if [ ! -f "build/RunMyModelDesktop" ]; then
    echo -e "${YELLOW}🔨 Building application...${NC}"
    ./build.sh || exit 1
else
    echo -e "${GREEN}✅ Binary already built${NC}"
fi

# Check model
if [ ! -f "models/tinyllama.gguf" ]; then
    echo -e "${RED}❌ Model not found: models/tinyllama.gguf${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Model found: $(du -h models/tinyllama.gguf | cut -f1)${NC}"
echo ""

# Launch
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${GREEN}✅ READY TO LAUNCH${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

export LD_LIBRARY_PATH="$(pwd)/build/lib:$(pwd)/lib/llama.cpp/build/bin:/opt/cuda/lib64:$LD_LIBRARY_PATH"
export PATH="/opt/cuda/bin:$PATH"

./build/RunMyModelDesktop

echo ""
echo -e "${GREEN}✅ Application closed${NC}"
echo ""

