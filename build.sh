#!/bin/bash
# RunMyModel 0.3.0 - Build Script (called by run.sh)

cd "$(dirname "$0")"

echo "🔨 Building RunMyModel 0.3.0..."
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

# Compile main.cpp
g++ -c -std=c++17 -fPIC -O2 -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB \
    -I. -Isrc-cpp/include \
    -I/usr/include/qt6 -I/usr/include/qt6/QtWidgets -I/usr/include/qt6/QtGui \
    -I/usr/include/qt6/QtCore \
    -o build/obj/main.o src-cpp/src/main.cpp

# Compile mainwindow.cpp
g++ -c -std=c++17 -fPIC -O2 -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB \
    -I. -Isrc-cpp/include \
    -I/usr/include/qt6 -I/usr/include/qt6/QtWidgets -I/usr/include/qt6/QtGui \
    -I/usr/include/qt6/QtCore \
    -o build/obj/mainwindow.o src-cpp/src/mainwindow.cpp

# Compile llama_engine.cpp
g++ -c -std=c++17 -fPIC -O2 -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB \
    -I. -Isrc-cpp/include -Ilib/llama.cpp/include -Ilib/llama.cpp/ggml/include \
    -I/usr/include/qt6 -I/usr/include/qt6/QtWidgets -I/usr/include/qt6/QtGui \
    -I/usr/include/qt6/QtCore -I/usr/include/qt6/QtConcurrent \
    -o build/obj/llama_engine.o src-cpp/src/llama_engine.cpp


echo "🔗 Generating MOC files..."
"$MOC_EXECUTABLE" src-cpp/include/mainwindow.h -o build/moc/moc_mainwindow.cpp
"$MOC_EXECUTABLE" src-cpp/include/llama_engine.h -o build/moc/moc_llama_engine.cpp

# Compile MOC files
g++ -c -std=c++17 -fPIC -O2 -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB \
    -I. -Isrc-cpp/include \
    -I/usr/include/qt6 -I/usr/include/qt6/QtWidgets -I/usr/include/qt6/QtGui \
    -I/usr/include/qt6/QtCore \
    -o build/obj/moc_mainwindow.o build/moc/moc_mainwindow.cpp

g++ -c -std=c++17 -fPIC -O2 -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB \
    -I. -Isrc-cpp/include \
    -I/usr/include/qt6 -I/usr/include/qt6/QtWidgets -I/usr/include/qt6/QtGui \
    -I/usr/include/qt6/QtCore \
    -o build/obj/moc_llama_engine.o build/moc/moc_llama_engine.cpp


echo "🔗 Linking..."

# Check if CUDA libraries exist
if [ -f "lib/llama.cpp/build/bin/libggml-cuda.so" ]; then
    echo "   ✅ CUDA support detected, linking with GPU acceleration"
    CUDA_LIBS="-lggml-cuda -lcuda -lcudart -lcublas"
else
    echo "   ⚠️  CUDA not found, linking CPU-only (run ./rebuild_with_cuda.sh for GPU)"
    CUDA_LIBS=""
fi

g++ -o build/RunMyModelDesktop \
    build/obj/main.o \
    build/obj/mainwindow.o \
    build/obj/llama_engine.o \
    build/obj/moc_mainwindow.o \
    build/obj/moc_llama_engine.o \
    -L/usr/lib -Llib/llama.cpp/build/bin -L/opt/cuda/lib64 \
    -lQt6Widgets -lQt6Gui -lQt6Core -lQt6Concurrent -lpthread \
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

