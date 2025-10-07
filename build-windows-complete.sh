#!/bin/bash

# Comprehensive Windows build script for RunMyModel Desktop
set -e

echo "🚀 RunMyModel Desktop - Windows Cross-Compilation Script"
echo "========================================================"

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check if package is installed (Arch Linux)
package_installed() {
    pacman -Q "$1" >/dev/null 2>&1
}

echo "🔍 Checking system requirements..."

# Check if we're on Arch Linux
if ! command_exists pacman; then
    echo "❌ This script is designed for Arch Linux. Please install dependencies manually:"
    echo "   Ubuntu/Debian: sudo apt install mingw-w64 gcc-mingw-w64-x86-64 wine"
    exit 1
fi

# Check Rust installation
if ! command_exists rustc; then
    echo "❌ Rust is not installed. Installing Rust..."
    sudo pacman -S rust --noconfirm
fi

# Check mingw-w64 installation
if ! package_installed mingw-w64-gcc; then
    echo "📦 Installing mingw-w64 for Windows cross-compilation..."
    sudo pacman -S mingw-w64-gcc mingw-w64-headers mingw-w64-crt --noconfirm
fi

# Check Wine installation
if ! command_exists wine; then
    echo "🍷 Installing Wine for testing..."
    sudo pacman -S wine-staging --noconfirm
fi

echo "✅ System requirements satisfied!"

# Navigate to project directory
cd /home/charlie/Documents/RunMyModel/runmymodel-desktop

# Check if we're in the right directory
if [ ! -f "package.json" ] || [ ! -f "src-tauri/Cargo.toml" ]; then
    echo "❌ Not in the correct project directory. Please run this script from the runmymodel-desktop folder."
    exit 1
fi

echo "📁 Working in: $(pwd)"

# Create cargo config for Windows cross-compilation
echo "⚙️  Configuring Cargo for Windows cross-compilation..."
mkdir -p ~/.cargo
cat > ~/.cargo/config.toml << 'EOF'
[target.x86_64-pc-windows-msvc]
linker = "x86_64-w64-mingw32-gcc"
EOF

# Install Node.js dependencies
echo "📦 Installing Node.js dependencies..."
npm install

# Build React frontend
echo "🔨 Building React frontend..."
npm run build

# Check if frontend build was successful
if [ ! -d "dist" ]; then
    echo "❌ Frontend build failed. Please check the build output above."
    exit 1
fi

echo "✅ Frontend build completed!"

# Build Tauri app for Windows
echo "🔨 Building Tauri app for Windows..."
echo "This may take several minutes..."

# Try using npm script first
if npm run build:windows; then
    echo "✅ Build completed using npm script!"
else
    echo "⚠️  npm script failed, trying cargo tauri directly..."
    cargo tauri build --target x86_64-pc-windows-msvc
fi

# Check if build was successful
EXE_PATH="src-tauri/target/x86_64-pc-windows-msvc/release/runmymodel-desktop.exe"
MSI_PATH="src-tauri/target/x86_64-pc-windows-msvc/release/bundle/msi/RunMyModel Desktop_1.0.0_x64_en-US.msi"

if [ -f "$EXE_PATH" ]; then
    echo "✅ Windows executable created successfully!"
    echo "📁 Executable location: $EXE_PATH"
    
    # Get file size
    SIZE=$(du -h "$EXE_PATH" | cut -f1)
    echo "📊 Executable size: $SIZE"
    
    # Test with Wine if available
    if command_exists wine; then
        echo "🍷 Testing executable with Wine..."
        echo "Note: This may take a moment to start..."
        timeout 10s wine "$EXE_PATH" || echo "Wine test completed (timeout or app started)"
    else
        echo "⚠️  Wine not available for testing. Install with: sudo pacman -S wine-staging"
    fi
else
    echo "❌ Windows executable not found. Build may have failed."
    echo "Check the build output above for errors."
    exit 1
fi

if [ -f "$MSI_PATH" ]; then
    echo "✅ Windows installer (MSI) created successfully!"
    echo "📁 Installer location: $MSI_PATH"
    
    # Get file size
    SIZE=$(du -h "$MSI_PATH" | cut -f1)
    echo "📊 Installer size: $SIZE"
else
    echo "⚠️  MSI installer not found, but executable was created."
fi

echo ""
echo "🎉 Windows build completed successfully!"
echo ""
echo "📋 Summary:"
echo "   Executable: $EXE_PATH"
if [ -f "$MSI_PATH" ]; then
    echo "   Installer: $MSI_PATH"
fi
echo ""
echo "🍷 To test with Wine:"
echo "   wine $EXE_PATH"
echo ""
echo "📦 To distribute:"
echo "   Copy the .exe file or .msi installer to Windows machines"
echo "   The MSI installer provides a proper Windows installation experience"
