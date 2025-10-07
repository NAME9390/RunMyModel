#!/bin/bash

# Comprehensive Linux build script for RunMyModel Desktop
set -e

echo "🚀 RunMyModel Desktop - Linux Build Script"
echo "=========================================="

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
    echo "   Ubuntu/Debian: sudo apt install build-essential curl libssl-dev pkg-config libgtk-3-dev libwebkit2gtk-4.1-dev librsvg2-dev"
    echo "   Fedora: sudo dnf install gcc-c++ curl openssl-devel pkgconfig gtk3-devel webkit2gtk3-devel librsvg2-devel"
    exit 1
fi

# Check Rust installation
if ! command_exists rustc; then
    echo "❌ Rust is not installed. Installing Rust..."
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
    source ~/.cargo/env
fi

# Check Node.js installation
if ! command_exists node; then
    echo "📦 Installing Node.js..."
    sudo pacman -S nodejs npm --noconfirm
fi

# Check required system dependencies
REQUIRED_PACKAGES=(
    "base-devel"
    "curl"
    "openssl"
    "pkg-config"
    "gtk3"
    "webkit2gtk"
    "librsvg"
    "appmenu-gtk-module"
    "libayatana-appindicator-gtk3"
)

echo "📦 Checking and installing system dependencies..."
for package in "${REQUIRED_PACKAGES[@]}"; do
    if ! package_installed "$package"; then
        echo "Installing $package..."
        sudo pacman -S "$package" --noconfirm
    fi
done

echo "✅ System requirements satisfied!"

# Navigate to project directory
cd /home/charlie/Documents/RunMyModel/runmymodel-desktop

# Check if we're in the right directory
if [ ! -f "package.json" ] || [ ! -f "src-tauri/Cargo.toml" ]; then
    echo "❌ Not in the correct project directory. Please run this script from the runmymodel-desktop folder."
    exit 1
fi

echo "📁 Working in: $(pwd)"

# Clean previous builds
echo "🧹 Cleaning previous builds..."
rm -rf dist/ src-tauri/target/ node_modules/ final-packages/

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

# Build Tauri app for Linux
echo "🔨 Building Tauri app for Linux..."
echo "This may take several minutes..."

# Try using npm script first
if npm run build:linux; then
    echo "✅ Build completed using npm script!"
else
    echo "⚠️  npm script failed, trying cargo tauri directly..."
    cargo tauri build --target x86_64-unknown-linux-gnu
fi

# Check if build was successful
BINARY_PATH="src-tauri/target/x86_64-unknown-linux-gnu/release/runmymodel-desktop"
APPIMAGE_PATH="src-tauri/target/x86_64-unknown-linux-gnu/release/bundle/appimage/RunMyModel Desktop_1.0.0_amd64.AppImage"
DEB_PATH="src-tauri/target/x86_64-unknown-linux-gnu/release/bundle/deb/RunMyModel Desktop_1.0.0_amd64.deb"

if [ -f "$BINARY_PATH" ]; then
    echo "✅ Linux binary created successfully!"
    echo "📁 Binary location: $BINARY_PATH"
    
    # Get file size
    SIZE=$(du -h "$BINARY_PATH" | cut -f1)
    echo "📊 Binary size: $SIZE"
else
    echo "❌ Linux binary not found. Build may have failed."
    echo "Check the build output above for errors."
    exit 1
fi

# Create final packages directory
echo "📦 Creating final packages..."
mkdir -p final-packages

# Copy AppImage if it exists
if [ -f "$APPIMAGE_PATH" ]; then
    echo "✅ AppImage created successfully!"
    echo "📁 AppImage location: $APPIMAGE_PATH"
    
    # Get file size
    SIZE=$(du -h "$APPIMAGE_PATH" | cut -f1)
    echo "📊 AppImage size: $SIZE"
    
    # Copy to final packages
    cp "$APPIMAGE_PATH" "final-packages/"
    echo "📦 AppImage copied to final-packages/"
else
    echo "⚠️  AppImage not found, but binary was created."
fi

# Copy DEB package if it exists
if [ -f "$DEB_PATH" ]; then
    echo "✅ DEB package created successfully!"
    echo "📁 DEB location: $DEB_PATH"
    
    # Get file size
    SIZE=$(du -h "$DEB_PATH" | cut -f1)
    echo "📊 DEB size: $SIZE"
    
    # Copy to final packages
    cp "$DEB_PATH" "final-packages/"
    echo "📦 DEB package copied to final-packages/"
else
    echo "⚠️  DEB package not found, but binary was created."
fi

# Copy binary to final packages
cp "$BINARY_PATH" "final-packages/runmymodel-desktop-linux"
echo "📦 Binary copied to final-packages/runmymodel-desktop-linux"

# Create a tar.gz archive
echo "📦 Creating tar.gz archive..."
cd final-packages/
tar -czf "runmymodel-desktop-linux.tar.gz" runmymodel-desktop-linux
cd ..

echo ""
echo "🎉 Linux build completed successfully!"
echo ""
echo "📋 Summary:"
echo "   Binary: $BINARY_PATH"
if [ -f "$APPIMAGE_PATH" ]; then
    echo "   AppImage: $APPIMAGE_PATH"
fi
if [ -f "$DEB_PATH" ]; then
    echo "   DEB Package: $DEB_PATH"
fi
echo "   Final packages: final-packages/"
echo ""
echo "📦 Distribution packages:"
echo "   - AppImage: Portable, no installation required"
echo "   - DEB: For Debian/Ubuntu-based systems"
echo "   - Binary: Raw executable for manual installation"
echo "   - Tar.gz: Compressed binary for distribution"
echo ""
echo "🚀 To test the AppImage:"
echo "   chmod +x final-packages/*.AppImage"
echo "   ./final-packages/*.AppImage"
echo ""
echo "📦 To install the DEB package:"
echo "   sudo dpkg -i final-packages/*.deb"
echo ""
echo "🔧 To install dependencies for DEB package:"
echo "   sudo apt-get install -f"
