#!/bin/bash
# RunMyModel 0.4.0 - Arch Linux Optimized Launcher
# Features: Auto CUDA setup, native optimizations, better performance

set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

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
echo -e "${MAGENTA}🧠 RunMyModel 0.4.0 - Arch Linux Edition${NC}"
echo -e "${CYAN}   Optimized for Arch/CachyOS with Auto CUDA${NC}"
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
    echo -e "${RED}❌ G++ not found! Install with: sudo pacman -S gcc${NC}"
    exit 1
fi
echo -e "${GREEN}✅ G++: $(g++ --version | head -1 | cut -d' ' -f3)${NC}"

if ! pkg-config --exists Qt6Core 2>/dev/null; then
    echo -e "${RED}❌ Qt6 not found! Install with: sudo pacman -S qt6-base${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Qt6: $(pkg-config --modversion Qt6Core)${NC}"

# GPU Detection and CUDA Setup
GPU_DETECTED=false
if command -v nvidia-smi &> /dev/null; then
    GPU_NAME=$(nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null | head -1)
    if [ ! -z "$GPU_NAME" ]; then
        GPU_DETECTED=true
        echo -e "${MAGENTA}🎮 GPU Detected: $GPU_NAME${NC}"
        
        # Check and install CUDA if needed
        if ! command -v nvcc &> /dev/null; then
            echo -e "${YELLOW}⚠️  CUDA not found. Installing...${NC}"
            sudo pacman -S --noconfirm cuda nvidia-utils
            
            if [ $? -eq 0 ]; then
                echo -e "${GREEN}✅ CUDA installed successfully${NC}"
            else
                echo -e "${RED}❌ Failed to install CUDA${NC}"
                GPU_DETECTED=false
            fi
        fi
        
        if command -v nvcc &> /dev/null; then
            CUDA_VERSION=$(nvcc --version | grep release | awk '{print $5}' | sed 's/,//')
            echo -e "${GREEN}✅ CUDA: $CUDA_VERSION${NC}"
            
            # Set CUDA environment
            export PATH="/opt/cuda/bin:$PATH"
            export LD_LIBRARY_PATH="/opt/cuda/lib64:$LD_LIBRARY_PATH"
            export CXXFLAGS="-march=native -O3 -pipe"
            export LDFLAGS="-Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now"
            
            # Rebuild llama.cpp with CUDA if needed
            if [ ! -f "lib/llama.cpp/build/bin/libggml-cuda.so" ]; then
                echo -e "${MAGENTA}🔥 Building llama.cpp with CUDA support...${NC}"
                
                if [ ! -d "lib/llama.cpp" ]; then
                    echo -e "${YELLOW}📦 Cloning llama.cpp...${NC}"
                    git clone https://github.com/ggerganov/llama.cpp lib/llama.cpp
                fi
                
                cd lib/llama.cpp
                rm -rf build
                mkdir build && cd build
                
                cmake .. \
                    -DCMAKE_BUILD_TYPE=Release \
                    -DGGML_CUDA=ON \
                    -DCMAKE_CUDA_ARCHITECTURES=native \
                    -DCMAKE_CXX_FLAGS="$CXXFLAGS" \
                    -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"
                
                make -j$(nproc)
                
                cd ../../..
                echo -e "${GREEN}✅ llama.cpp built with CUDA!${NC}"
            else
                echo -e "${GREEN}✅ llama.cpp already built with CUDA${NC}"
            fi
        fi
    fi
fi

if [ "$GPU_DETECTED" = false ]; then
    echo -e "${BLUE}ℹ️  No NVIDIA GPU detected or CUDA unavailable (CPU mode)${NC}"
fi

echo ""

# Build llama.cpp if not exists (CPU-only)
if [ ! -d "lib/llama.cpp" ]; then
    echo -e "${YELLOW}📦 Cloning llama.cpp...${NC}"
    git clone https://github.com/ggerganov/llama.cpp lib/llama.cpp
fi

if [ ! -f "lib/llama.cpp/build/bin/libllama.so" ]; then
    echo -e "${YELLOW}🔨 Building llama.cpp (CPU)...${NC}"
    
    cd lib/llama.cpp
    rm -rf build
    mkdir build && cd build
    
    export CXXFLAGS="-march=native -O3 -pipe"
    export LDFLAGS="-Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now"
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="$CXXFLAGS" \
        -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"
    
    make -j$(nproc)
    
    cd ../../..
    echo -e "${GREEN}✅ llama.cpp built (CPU)${NC}"
fi

# Build application
if [ ! -f "build/RunMyModelDesktop" ]; then
    echo -e "${YELLOW}🔨 Building application with Arch optimizations...${NC}"
    "$SCRIPT_DIR/build.sh" || exit 1
else
    echo -e "${GREEN}✅ Binary already built${NC}"
fi

# Check model
if [ ! -f "models/tinyllama.gguf" ]; then
    echo -e "${RED}❌ Model not found: models/tinyllama.gguf${NC}"
    echo -e "${YELLOW}💡 Download with:${NC}"
    echo "   wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf"
    exit 1
fi
echo -e "${GREEN}✅ Model found: $(du -h models/tinyllama.gguf | cut -f1)${NC}"
echo ""

# Launch
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${GREEN}✅ READY TO LAUNCH${NC}"
if [ "$GPU_DETECTED" = true ]; then
    echo -e "${MAGENTA}🎮 GPU Acceleration: ENABLED${NC}"
else
    echo -e "${BLUE}💻 CPU Mode: ENABLED${NC}"
fi
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

export LD_LIBRARY_PATH="$(pwd)/build/lib:$(pwd)/lib/llama.cpp/build/bin:/opt/cuda/lib64:$LD_LIBRARY_PATH"
export PATH="/opt/cuda/bin:$PATH"
export QT_QPA_PLATFORMTHEME="${QT_QPA_PLATFORMTHEME:-qt6ct}"

./build/RunMyModelDesktop

echo ""
echo -e "${GREEN}✅ Application closed${NC}"
echo ""

