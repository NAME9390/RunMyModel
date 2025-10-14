#!/bin/bash
# RunMyModel Desktop v0.5.0-PRE-RELEASE - Cross-Compilation Setup
# Sets up environment for building Windows executables on Linux

set -e

echo "🚀 Setting up Cross-Compilation Environment for RunMyModel Desktop"
echo "=================================================================="

# Configuration
MINGW_VERSION="12.2.0"
QT_VERSION="6.5.0"
BUILD_DIR="cross_compile"
EXECUTABLES_DIR="../executables"

# Check if we're on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo "❌ This script is designed for Linux. For Windows builds, use build_msi.sh on Windows."
    exit 1
fi

# Check dependencies
echo "🔍 Checking dependencies..."
if ! command -v apt-get &> /dev/null && ! command -v pacman &> /dev/null && ! command -v dnf &> /dev/null; then
    echo "❌ Package manager not found. Please install dependencies manually."
    exit 1
fi

# Install dependencies
echo "📦 Installing cross-compilation dependencies..."
if command -v apt-get &> /dev/null; then
    # Ubuntu/Debian
    sudo apt-get update
    sudo apt-get install -y \
        mingw-w64 \
        wine \
        wine32 \
        wine64 \
        libwine-dev \
        winetricks \
        nsis \
        imagemagick \
        wget \
        curl \
        git
elif command -v pacman &> /dev/null; then
    # Arch Linux
    sudo pacman -S --noconfirm \
        mingw-w64-gcc \
        mingw-w64-binutils \
        wine \
        winetricks \
        nsis \
        imagemagick \
        wget \
        curl \
        git
elif command -v dnf &> /dev/null; then
    # Fedora/RHEL
    sudo dnf install -y \
        mingw64-gcc \
        mingw64-binutils \
        wine \
        winetricks \
        nsis \
        ImageMagick \
        wget \
        curl \
        git
fi

# Create build directory
echo "📁 Creating build directory..."
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Download and setup MinGW-w64
echo "🔧 Setting up MinGW-w64..."
if [ ! -d "mingw64" ]; then
    echo "Downloading MinGW-w64..."
    wget -q https://github.com/niXman/mingw-builds-binaries/releases/download/${MINGW_VERSION}-rt_v10-rev2/winlibs-x86_64-posix-seh-gcc-${MINGW_VERSION}-mingw-w64-10.0.0-r2.zip
    unzip -q winlibs-x86_64-posix-seh-gcc-${MINGW_VERSION}-mingw-w64-10.0.0-r2.zip
    mv mingw64 mingw64-temp
    mkdir -p mingw64
    mv mingw64-temp/* mingw64/
    rm -rf mingw64-temp
    rm winlibs-x86_64-posix-seh-gcc-${MINGW_VERSION}-mingw-w64-10.0.0-r2.zip
fi

# Setup environment variables
echo "🌍 Setting up environment variables..."
export PATH="$(pwd)/mingw64/bin:$PATH"
export CC="x86_64-w64-mingw32-gcc"
export CXX="x86_64-w64-mingw32-g++"
export AR="x86_64-w64-mingw32-ar"
export RANLIB="x86_64-w64-mingw32-ranlib"
export STRIP="x86_64-w64-mingw32-strip"
export WINDRES="x86_64-w64-mingw32-windres"

# Download Qt for Windows
echo "📦 Downloading Qt for Windows..."
if [ ! -d "qt-windows" ]; then
    wget -q https://download.qt.io/official_releases/qt/6.5/${QT_VERSION}/qt-opensource-windows-x86-${QT_VERSION}.exe
    echo "Qt installer downloaded. Please run it manually to install Qt for Windows."
    echo "Installation path should be: $(pwd)/qt-windows"
fi

# Setup Wine environment
echo "🍷 Setting up Wine environment..."
export WINEPREFIX="$(pwd)/wine_prefix"
winecfg &
echo "Please configure Wine in the GUI that opened, then press Enter to continue..."
read -p "Press Enter to continue..."

# Download Windows libraries
echo "📚 Downloading Windows libraries..."
mkdir -p windows_libs

# Download Visual C++ Redistributable
wget -q https://aka.ms/vs/17/release/vc_redist.x64.exe -O windows_libs/vc_redist.x64.exe

# Download Windows SDK (if needed)
echo "Windows SDK download would go here..."

# Create cross-compilation script
echo "📝 Creating cross-compilation script..."
cat > build_windows.sh << 'EOF'
#!/bin/bash
# Cross-compilation script for Windows

set -e

echo "🔨 Cross-compiling RunMyModel Desktop for Windows..."

# Set environment
export PATH="$(pwd)/mingw64/bin:$PATH"
export CC="x86_64-w64-mingw32-gcc"
export CXX="x86_64-w64-mingw32-g++"
export AR="x86_64-w64-mingw32-ar"
export RANLIB="x86_64-w64-mingw32-ranlib"
export STRIP="x86_64-w64-mingw32-strip"
export WINDRES="x86_64-w64-mingw32-windres"

# Qt paths (adjust based on actual installation)
export QT_WINDOWS_PATH="$(pwd)/qt-windows"
export QT_BIN="$QT_WINDOWS_PATH/bin"
export QT_LIB="$QT_WINDOWS_PATH/lib"
export QT_INCLUDE="$QT_WINDOWS_PATH/include"

# Build flags
export CFLAGS="-I$QT_INCLUDE -I$QT_INCLUDE/QtCore -I$QT_INCLUDE/QtGui -I$QT_INCLUDE/QtWidgets"
export CXXFLAGS="$CFLAGS -std=c++17"
export LDFLAGS="-L$QT_LIB -lQt6Core -lQt6Gui -lQt6Widgets -lQt6Concurrent -lQt6Network"

# Create build directory
mkdir -p build_windows
cd build_windows

# Copy source files
cp -r ../../app/src-cpp/* .

# Build the application
echo "Compiling for Windows..."
$CXX $CXXFLAGS -o RunMyModelDesktop.exe \
    src/main.cpp \
    src/mainwindow.cpp \
    src/llama_engine.cpp \
    $LDFLAGS

echo "✅ Windows executable created: RunMyModelDesktop.exe"
EOF

chmod +x build_windows.sh

# Create packaging script
echo "📦 Creating packaging script..."
cat > package_windows.sh << 'EOF'
#!/bin/bash
# Package Windows executable

set -e

echo "📦 Packaging Windows executable..."

# Create package directory
mkdir -p package
cd package

# Copy executable
cp ../build_windows/RunMyModelDesktop.exe .

# Copy Qt libraries
cp -r ../qt-windows/lib/* .
cp -r ../qt-windows/plugins .

# Copy Windows libraries
cp ../windows_libs/* .

# Create installer script
cat > installer.nsi << 'INSTALLER'
!define APP_NAME "RunMyModel Desktop"
!define APP_VERSION "0.5.0-PRE-RELEASE"
!define APP_PUBLISHER "RunMyModel Team"

Name "${APP_NAME} ${APP_VERSION}"
OutFile "RunMyModelDesktop-v${APP_VERSION}-x64.exe"
InstallDir "$PROGRAMFILES64\RunMyModel"
RequestExecutionLevel admin

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  File "RunMyModelDesktop.exe"
  File /r "lib"
  File /r "plugins"
  
  CreateDirectory "$SMPROGRAMS\RunMyModel"
  CreateShortCut "$SMPROGRAMS\RunMyModel\RunMyModel Desktop.lnk" "$INSTDIR\RunMyModelDesktop.exe"
  CreateShortCut "$DESKTOP\RunMyModel Desktop.lnk" "$INSTDIR\RunMyModelDesktop.exe"
SectionEnd
INSTALLER

# Build installer
makensis installer.nsi

echo "✅ Windows package created"
EOF

chmod +x package_windows.sh

# Create README for cross-compilation
echo "📖 Creating cross-compilation README..."
cat > README_CROSS_COMPILE.md << 'EOF'
# Cross-Compilation Setup for RunMyModel Desktop

This directory contains the cross-compilation environment for building Windows executables on Linux.

## Prerequisites

- Linux system (Ubuntu/Debian, Arch Linux, or Fedora)
- Internet connection for downloading dependencies
- At least 5GB free disk space

## Setup

1. Run the setup script:
   ```bash
   ./setup_cross_compile.sh
   ```

2. Follow the prompts to install dependencies

3. Configure Wine when prompted

4. Install Qt for Windows (manual step)

## Building

1. Build the Windows executable:
   ```bash
   ./build_windows.sh
   ```

2. Package the executable:
   ```bash
   ./package_windows.sh
   ```

## Output

The final Windows installer will be created in the `package/` directory.

## Troubleshooting

### Common Issues

1. **Qt not found**: Make sure Qt for Windows is installed in the correct path
2. **Wine configuration**: Ensure Wine is properly configured for Windows 10
3. **Missing libraries**: Check that all required Windows libraries are downloaded

### Getting Help

- Check the main project documentation
- Review the build logs for specific error messages
- Ensure all dependencies are properly installed

## Notes

- This is a simplified cross-compilation setup
- For production builds, consider using a dedicated Windows build environment
- Some features may require Windows-specific testing
EOF

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Cross-compilation environment setup complete!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📁 Build directory: $(pwd)"
echo "🔧 Next steps:"
echo "   1. Install Qt for Windows manually"
echo "   2. Run: ./build_windows.sh"
echo "   3. Run: ./package_windows.sh"
echo ""
echo "📖 See README_CROSS_COMPILE.md for detailed instructions"
echo ""
