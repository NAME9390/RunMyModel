#!/bin/bash

# Build script for Windows cross-compilation
set -e

echo "🚀 Building RunMyModel Desktop for Windows..."

# Check if Rust is properly installed
if ! command -v rustup &> /dev/null; then
    echo "❌ Rust is not installed or corrupted. Please fix Rust installation first."
    exit 1
fi

# Check if Windows target is installed
if ! rustup target list --installed | grep -q "x86_64-pc-windows-msvc"; then
    echo "📦 Adding Windows target..."
    rustup target add x86_64-pc-windows-msvc
fi

# Check if mingw-w64 is installed
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "❌ mingw-w64 is not installed. Please install it first:"
    echo "   Arch: sudo pacman -S mingw-w64-gcc"
    echo "   Ubuntu: sudo apt install gcc-mingw-w64-x86-64"
    exit 1
fi

# Build frontend
echo "🔨 Building React frontend..."
npm run build

# Build Tauri app for Windows
echo "🔨 Building Tauri app for Windows..."
cargo tauri build --target x86_64-pc-windows-msvc

echo "✅ Build complete!"
echo ""
echo "📁 Output files:"
echo "   Executable: src-tauri/target/x86_64-pc-windows-msvc/release/runmymodel-desktop.exe"
echo "   Installer: src-tauri/target/x86_64-pc-windows-msvc/release/bundle/msi/RunMyModel Desktop_1.0.0_x64_en-US.msi"
echo ""
echo "🍷 Test with Wine:"
echo "   wine src-tauri/target/x86_64-pc-windows-msvc/release/runmymodel-desktop.exe"
