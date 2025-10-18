#!/bin/bash
# RunMyModel 0.3.0 - Build Script (called by run.sh)

cd "$(dirname "$0")"

echo "🔨 Building RunMyModel 0.6.0-PRE-RELEASE..."
echo ""

# Create build directories
mkdir -p build/obj build/moc build/lib

# Find Qt6 moc
MOC_EXECUTABLE=$(which moc-qt6 || which /usr/lib/qt6/moc || which moc)
if [ -z "$MOC_EXECUTABLE" ]; then
    echo "❌ Error: Qt6 moc not found"
    exit 1
fi

echo "📦 Compiling source files..."

# Common compiler flags
COMMON_FLAGS="-c -std=c++17 -fPIC -O2 -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB"
INCLUDE_FLAGS="-I. -Isrc-cpp/include -Ilib/llama.cpp/include -Ilib/llama.cpp/ggml/include"
QT_INCLUDES="-I/usr/include/qt6 -I/usr/include/qt6/QtWidgets -I/usr/include/qt6/QtGui -I/usr/include/qt6/QtCore -I/usr/include/qt6/QtConcurrent -I/usr/include/qt6/QtNetwork"

# Compile main.cpp
g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
    -o build/obj/main.o src-cpp/src/main.cpp

# Compile mainwindow.cpp (skip mainwindow_new.cpp as it's commented out)
g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
    -o build/obj/mainwindow.o src-cpp/src/mainwindow.cpp

# Skip mainwindow_new.cpp as it's commented out to prevent conflicts
# g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
#     -o build/obj/mainwindow_new.o src-cpp/src/mainwindow_new.cpp

# Compile llama_engine.cpp
g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
    -o build/obj/llama_engine.o src-cpp/src/llama_engine.cpp

# Compile existing fine-tune components (if they exist)
if [ -f "src-cpp/src/finetune_panel.cpp" ]; then
    echo "   ✅ Compiling finetune_panel.cpp"
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/finetune_panel.o src-cpp/src/finetune_panel.cpp
fi

if [ -f "src-cpp/src/finetune_engine.cpp" ]; then
    echo "   ✅ Compiling finetune_engine.cpp"
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/finetune_engine.o src-cpp/src/finetune_engine.cpp
fi

# Compile new 0.6.0 components (if they exist)
if [ -f "src-cpp/src/base_model_manager.cpp" ]; then
    echo "   ✅ Compiling base_model_manager.cpp"
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/base_model_manager.o src-cpp/src/base_model_manager.cpp
fi

if [ -f "src-cpp/src/model_manager.cpp" ]; then
    echo "   ✅ Compiling model_manager.cpp"
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/model_manager.o src-cpp/src/model_manager.cpp
fi

if [ -f "src-cpp/src/api_manager.cpp" ]; then
    echo "   ✅ Compiling api_manager.cpp"
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/api_manager.o src-cpp/src/api_manager.cpp
fi

# Compile new 0.6.0 advanced components
if [ -f "src-cpp/src/rag_system.cpp" ]; then
    echo "   ✅ Compiling rag_system.cpp"
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/rag_system.o src-cpp/src/rag_system.cpp
fi

if [ -f "src-cpp/src/session_manager.cpp" ]; then
    echo "   ✅ Compiling session_manager.cpp"
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/session_manager.o src-cpp/src/session_manager.cpp
fi

if [ -f "src-cpp/src/plugin_system.cpp" ]; then
    echo "   ✅ Compiling plugin_system.cpp"
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/plugin_system.o src-cpp/src/plugin_system.cpp
fi

if [ -f "src-cpp/src/model_trainer.cpp" ]; then
    echo "   ✅ Compiling model_trainer.cpp"
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/model_trainer.o src-cpp/src/model_trainer.cpp
fi

echo "🔗 Generating MOC files..."
"$MOC_EXECUTABLE" src-cpp/include/mainwindow.h -o build/moc/moc_mainwindow.cpp
"$MOC_EXECUTABLE" src-cpp/include/llama_engine.h -o build/moc/moc_llama_engine.cpp

# Generate MOC files for existing components
if [ -f "src-cpp/include/finetune_panel.h" ]; then
    "$MOC_EXECUTABLE" src-cpp/include/finetune_panel.h -o build/moc/moc_finetune_panel.cpp
fi

if [ -f "src-cpp/include/finetune_engine.h" ]; then
    "$MOC_EXECUTABLE" src-cpp/include/finetune_engine.h -o build/moc/moc_finetune_engine.cpp
fi

# Generate MOC files for new components
if [ -f "src-cpp/include/base_model_manager.h" ]; then
    "$MOC_EXECUTABLE" src-cpp/include/base_model_manager.h -o build/moc/moc_base_model_manager.cpp
fi

if [ -f "src-cpp/include/model_manager.h" ]; then
    "$MOC_EXECUTABLE" src-cpp/include/model_manager.h -o build/moc/moc_model_manager.cpp
fi

if [ -f "src-cpp/include/api_manager.h" ]; then
    "$MOC_EXECUTABLE" src-cpp/include/api_manager.h -o build/moc/moc_api_manager.cpp
fi

echo "🔗 Compiling MOC files..."
# Compile main MOC files
g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
    -o build/obj/moc_mainwindow.o build/moc/moc_mainwindow.cpp

g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
    -o build/obj/moc_llama_engine.o build/moc/moc_llama_engine.cpp

# Compile existing MOC files (if they exist)
if [ -f "build/moc/moc_finetune_panel.cpp" ]; then
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/moc_finetune_panel.o build/moc/moc_finetune_panel.cpp
fi

if [ -f "build/moc/moc_finetune_engine.cpp" ]; then
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/moc_finetune_engine.o build/moc/moc_finetune_engine.cpp
fi

# Compile new MOC files (if they exist)
if [ -f "build/moc/moc_base_model_manager.cpp" ]; then
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/moc_base_model_manager.o build/moc/moc_base_model_manager.cpp
fi

if [ -f "build/moc/moc_model_manager.cpp" ]; then
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/moc_model_manager.o build/moc/moc_model_manager.cpp
fi

if [ -f "build/moc/moc_api_manager.cpp" ]; then
    g++ $COMMON_FLAGS $INCLUDE_FLAGS $QT_INCLUDES \
        -o build/obj/moc_api_manager.o build/moc/moc_api_manager.cpp
fi


echo "🔗 Linking..."

# Check if CUDA libraries exist
if [ -f "lib/llama.cpp/build/bin/libggml-cuda.so" ]; then
    echo "   ✅ CUDA support detected, linking with GPU acceleration"
    CUDA_LIBS="-lggml-cuda -lcuda -lcudart -lcublas"
else
    echo "   ⚠️  CUDA not found, linking CPU-only (run ./rebuild_with_cuda.sh for GPU)"
    CUDA_LIBS=""
fi

# Collect all object files
OBJECT_FILES="build/obj/main.o build/obj/mainwindow.o build/obj/llama_engine.o build/obj/moc_mainwindow.o build/obj/moc_llama_engine.o"

# Add existing component object files if they exist
if [ -f "build/obj/finetune_panel.o" ]; then
    OBJECT_FILES="$OBJECT_FILES build/obj/finetune_panel.o"
fi

if [ -f "build/obj/finetune_engine.o" ]; then
    OBJECT_FILES="$OBJECT_FILES build/obj/finetune_engine.o"
fi

if [ -f "build/obj/moc_finetune_panel.o" ]; then
    OBJECT_FILES="$OBJECT_FILES build/obj/moc_finetune_panel.o"
fi

if [ -f "build/obj/moc_finetune_engine.o" ]; then
    OBJECT_FILES="$OBJECT_FILES build/obj/moc_finetune_engine.o"
fi

# Add new 0.5.0 component object files if they exist
if [ -f "build/obj/base_model_manager.o" ]; then
    OBJECT_FILES="$OBJECT_FILES build/obj/base_model_manager.o"
fi

if [ -f "build/obj/model_manager.o" ]; then
    OBJECT_FILES="$OBJECT_FILES build/obj/model_manager.o"
fi

if [ -f "build/obj/api_manager.o" ]; then
    OBJECT_FILES="$OBJECT_FILES build/obj/api_manager.o"
fi

if [ -f "build/obj/moc_base_model_manager.o" ]; then
    OBJECT_FILES="$OBJECT_FILES build/obj/moc_base_model_manager.o"
fi

if [ -f "build/obj/moc_model_manager.o" ]; then
    OBJECT_FILES="$OBJECT_FILES build/obj/moc_model_manager.o"
fi

if [ -f "build/obj/moc_api_manager.o" ]; then
    OBJECT_FILES="$OBJECT_FILES build/obj/moc_api_manager.o"
fi

echo "   🔗 Linking object files: $(echo $OBJECT_FILES | wc -w) files"

g++ -o build/RunMyModelDesktop \
    $OBJECT_FILES \
    -L/usr/lib -Llib/llama.cpp/build/bin -L/opt/cuda/lib64 \
    -lQt6Widgets -lQt6Gui -lQt6Core -lQt6Concurrent -lQt6Network -lpthread \
    -lllama -lggml -lggml-base -lggml-cpu $CUDA_LIBS -lGLX -lOpenGL

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "🔗 Copying llama.cpp libraries..."
cp -v lib/llama.cpp/build/bin/*.so build/lib/ 2>/dev/null || echo "   Libraries already in place"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ BUILD SUCCESSFUL!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Binary: build/RunMyModelDesktop"
echo "Libraries: build/lib/*.so"
echo ""

