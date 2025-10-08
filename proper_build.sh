#!/bin/bash

# Proper build script with error checking
set -e

echo "🔧 Building RunMyModel Desktop..."

# Clean and build frontend
echo "Building frontend..."
npm run build

# Clean and build Qt app
echo "Building Qt application..."
rm -rf build/
mkdir -p build
cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Copy frontend files and llms.txt
echo "Copying frontend files and model list..."
cp -r ../dist .
cp ../llms.txt .

cd ..

echo "✅ Build successful!"
echo "📊 Models available: $(wc -l < llms.txt | xargs)"
echo "🚀 Run the app: ./build/RunMyModelDesktop"
