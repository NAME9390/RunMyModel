#!/bin/bash

# RunMyModel Desktop - Qt Desktop Application Build Script
# This script builds the Qt/C++ desktop application with React frontend

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check prerequisites
check_prerequisites() {
    print_status "Checking prerequisites..."
    
    # Check Node.js
    if ! command_exists node; then
        print_error "Node.js is not installed. Please install Node.js 18+ first."
        exit 1
    fi
    
    # Check npm
    if ! command_exists npm; then
        print_error "npm is not installed. Please install npm first."
        exit 1
    fi
    
    # Check Node.js version
    NODE_VERSION=$(node -v | cut -d'v' -f2 | cut -d'.' -f1)
    if [ "$NODE_VERSION" -lt 18 ]; then
        print_error "Node.js version 18+ is required. Current version: $(node -v)"
        exit 1
    fi
    
    # Check Qt6
    if ! command_exists qmake6; then
        if ! command_exists qmake; then
            print_error "Qt6 is not installed. Please install Qt6 development packages."
            print_error "On Ubuntu/Debian: sudo apt install qt6-base-dev qt6-webengine-dev"
            print_error "On Arch: sudo pacman -S qt6-base qt6-webengine"
            exit 1
        fi
    fi
    
    # Check CMake
    if ! command_exists cmake; then
        print_error "CMake is not installed. Please install CMake first."
        exit 1
    fi
    
    # Check C++ compiler
    if ! command_exists g++; then
        print_error "g++ compiler is not installed. Please install build-essential."
        exit 1
    fi
    
    print_success "Prerequisites check passed"
}

# Function to install dependencies
install_dependencies() {
    print_status "Installing Node.js dependencies..."
    npm install
    print_success "Node.js dependencies installed"
}

# Function to build frontend
build_frontend() {
    print_status "Building React frontend..."
    npm run build
    print_success "Frontend built successfully"
}

# Function to build Qt application
build_qt_app() {
    print_status "Building Qt desktop application..."
    
    # Create build directory
    BUILD_DIR="build"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure with CMake
    print_status "Configuring with CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release
    
    # Build the application
    print_status "Compiling Qt application..."
    make -j$(nproc)
    
    cd ..
    print_success "Qt application built successfully"
}

# Function to create Linux executable package
create_linux_package() {
    print_status "Creating Linux executable package..."
    
    LINUX_DIR="executables/linux"
    mkdir -p "$LINUX_DIR"
    
    # Copy the Qt executable
    cp "build/RunMyModelDesktop" "$LINUX_DIR/"
    
    # Copy frontend files and llms.txt
    cp -r dist "$LINUX_DIR/"
    cp llms.txt "$LINUX_DIR/"
    
    # Create launcher script
    cat > "$LINUX_DIR/runmymodel-desktop.sh" << 'EOF'
#!/bin/bash

# RunMyModel Desktop - Linux Launcher
# This script launches the Qt desktop application

# Get the directory where this script is located
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Change to the application directory
cd "$DIR"

# Run the Qt application
./RunMyModelDesktop "$@"
EOF
    
    chmod +x "$LINUX_DIR/runmymodel-desktop.sh"
    chmod +x "$LINUX_DIR/RunMyModelDesktop"
    
    # Create desktop entry
    cat > "$LINUX_DIR/runmymodel-desktop.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=RunMyModel Desktop
Comment=Run LLMs locally with Hugging Face
Exec=$LINUX_DIR/runmymodel-desktop.sh
Icon=$LINUX_DIR/dist/favicon.svg
Terminal=false
Categories=Network;Chat;
Path=$LINUX_DIR
EOF
    
    print_success "Linux package created: $LINUX_DIR"
}

# Function to create Windows executable package
create_windows_package() {
    print_status "Creating Windows executable package..."
    
    WINDOWS_DIR="executables/windows"
    mkdir -p "$WINDOWS_DIR"
    
    # Copy the Qt executable (if built for Windows)
    if [ -f "build/RunMyModelDesktop.exe" ]; then
        cp "build/RunMyModelDesktop.exe" "$WINDOWS_DIR/"
    else
        print_warning "Windows executable not found. Skipping Windows package."
        return
    fi
    
    # Copy frontend files
    cp -r dist "$WINDOWS_DIR/"
    
    # Create Windows batch launcher
    cat > "$WINDOWS_DIR/runmymodel-desktop.bat" << 'EOF'
@echo off
REM RunMyModel Desktop - Windows Launcher
REM This script launches the Qt desktop application

set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

REM Run the Qt application
RunMyModelDesktop.exe %*
EOF
    
    # Create Windows installer script
    cat > "$WINDOWS_DIR/install.bat" << 'EOF'
@echo off
REM RunMyModel Desktop - Windows Installer
REM This script installs RunMyModel Desktop to the user's system

echo Installing RunMyModel Desktop...

REM Create application directory
set "APP_DIR=%USERPROFILE%\RunMyModelDesktop"
mkdir "%APP_DIR%" 2>nul

REM Copy files
xcopy /E /I /Y "%~dp0*" "%APP_DIR%"

REM Create desktop shortcut
set "DESKTOP=%USERPROFILE%\Desktop"
echo [InternetShortcut] > "%DESKTOP%\RunMyModel Desktop.url"
echo URL=file:///%APP_DIR%/dist/index.html >> "%DESKTOP%\RunMyModel Desktop.url"
echo IconFile=%APP_DIR%\dist\favicon.svg >> "%DESKTOP%\RunMyModel Desktop.url"

echo Installation completed!
echo RunMyModel Desktop has been installed to: %APP_DIR%
echo A shortcut has been created on your desktop.
pause
EOF
    
    print_success "Windows package created: $WINDOWS_DIR"
}

# Function to create distribution packages
create_packages() {
    print_status "Creating distribution packages..."
    
    # Create Linux package
    if [ -d "executables/linux" ]; then
        cd executables
        tar -czf "runmymodel-desktop-linux.tar.gz" linux/
        cd ..
        print_success "Linux package created: executables/runmymodel-desktop-linux.tar.gz"
    fi
    
    # Create Windows package
    if [ -d "executables/windows" ] && [ -f "executables/windows/RunMyModelDesktop.exe" ]; then
        cd executables
        if command_exists zip; then
            zip -r "runmymodel-desktop-windows.zip" windows/
            print_success "Windows package created: executables/runmymodel-desktop-windows.zip"
        else
            print_warning "zip command not found. Windows package not created."
        fi
        cd ..
    fi
}

# Function to show usage
show_usage() {
    echo "RunMyModel Desktop - Qt Desktop Application Build Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --linux-only     Build only for Linux"
    echo "  --windows-only   Build only for Windows"
    echo "  --appimage       Create AppImage for Linux"
    echo "  --windows-exe    Create Windows .exe installer"
    echo "  --no-packages    Skip creating distribution packages"
    echo "  --clean          Clean build directories before building"
    echo "  --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                    # Build for both platforms"
    echo "  $0 --linux-only       # Build only for Linux"
    echo "  $0 --appimage         # Create AppImage"
    echo "  $0 --windows-exe      # Create Windows .exe"
    echo "  $0 --clean            # Clean and rebuild"
}

# Function to clean build directories
clean_build() {
    print_status "Cleaning build directories..."
    rm -rf build/
    rm -rf executables/
    print_success "Build directories cleaned"
}

# Function to create AppImage
create_appimage() {
    print_status "Creating AppImage..."
    if [ -f "build_appimage.sh" ]; then
        chmod +x build_appimage.sh
        ./build_appimage.sh
    else
        print_error "AppImage build script not found"
    fi
}

# Function to create Windows .exe
create_windows_exe() {
    print_status "Creating Windows .exe..."
    if [ -f "build_windows.sh" ]; then
        chmod +x build_windows.sh
        ./build_windows.sh
    else
        print_error "Windows build script not found"
    fi
}

# Main function
main() {
    local BUILD_LINUX=true
    local BUILD_WINDOWS=true
    local CREATE_PACKAGES=true
    local CLEAN_BUILD=false
    local CREATE_APPIMAGE=false
    local CREATE_WINDOWS_EXE=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --linux-only)
                BUILD_WINDOWS=false
                shift
                ;;
            --windows-only)
                BUILD_LINUX=false
                shift
                ;;
            --appimage)
                CREATE_APPIMAGE=true
                shift
                ;;
            --windows-exe)
                CREATE_WINDOWS_EXE=true
                shift
                ;;
            --no-packages)
                CREATE_PACKAGES=false
                shift
                ;;
            --clean)
                CLEAN_BUILD=true
                shift
                ;;
            --help)
                show_usage
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
    
    print_status "Starting RunMyModel Desktop Qt build process..."
    
    # Clean if requested
    if [ "$CLEAN_BUILD" = true ]; then
        clean_build
    fi
    
    # Run build steps
    check_prerequisites
    install_dependencies
    build_frontend
    build_qt_app
    
    if [ "$BUILD_LINUX" = true ]; then
        create_linux_package
    fi
    
    if [ "$BUILD_WINDOWS" = true ]; then
        create_windows_package
    fi
    
    if [ "$CREATE_PACKAGES" = true ]; then
        create_packages
    fi
    
    if [ "$CREATE_APPIMAGE" = true ]; then
        create_appimage
    fi
    
    if [ "$CREATE_WINDOWS_EXE" = true ]; then
        create_windows_exe
    fi
    
    print_success "Build process completed successfully!"
    print_status "Executables are available in the 'executables/' directory"
    
    if [ "$BUILD_LINUX" = true ]; then
        print_status "Linux: executables/linux/runmymodel-desktop.sh"
    fi
    
    if [ "$BUILD_WINDOWS" = true ]; then
        print_status "Windows: executables/windows/runmymodel-desktop.bat"
    fi
    
    if [ "$CREATE_APPIMAGE" = true ]; then
        print_status "AppImage: RunMyModelDesktop-*.AppImage"
    fi
    
    if [ "$CREATE_WINDOWS_EXE" = true ]; then
        print_status "Windows Installer: RunMyModelDesktop-Setup.exe"
    fi
}

# Run main function with all arguments
main "$@"