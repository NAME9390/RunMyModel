#!/bin/bash

# RunMyModel Desktop - Universal Build Script
# This script builds the application for both Linux and Windows platforms

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
    
    if ! command_exists node; then
        print_error "Node.js is not installed. Please install Node.js 18+ first."
        exit 1
    fi
    
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
    
    print_success "Prerequisites check passed"
}

# Function to install dependencies
install_dependencies() {
    print_status "Installing dependencies..."
    npm install
    print_success "Dependencies installed"
}

# Function to build frontend
build_frontend() {
    print_status "Building frontend..."
    npm run build
    print_success "Frontend built successfully"
}

# Function to build for Linux
build_linux() {
    print_status "Building for Linux..."
    
    # Create Linux executable directory
    LINUX_DIR="executables/linux"
    mkdir -p "$LINUX_DIR"
    
    # Copy frontend build
    cp -r dist "$LINUX_DIR/"
    
    # Create a simple launcher script for Linux
    cat > "$LINUX_DIR/runmymodel-desktop.sh" << 'EOF'
#!/bin/bash

# RunMyModel Desktop - Linux Launcher
# This script launches the RunMyModel Desktop application

# Get the directory where this script is located
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Check if we're running in a desktop environment
if [ -z "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ]; then
    echo "Error: No desktop environment detected. Please run this from a desktop environment."
    exit 1
fi

# Try to open the application in the default browser
if command -v xdg-open >/dev/null 2>&1; then
    xdg-open "file://$DIR/dist/index.html"
elif command -v firefox >/dev/null 2>&1; then
    firefox "file://$DIR/dist/index.html"
elif command -v chromium-browser >/dev/null 2>&1; then
    chromium-browser "file://$DIR/dist/index.html"
elif command -v google-chrome >/dev/null 2>&1; then
    google-chrome "file://$DIR/dist/index.html"
else
    echo "Error: No suitable browser found. Please install Firefox, Chrome, or Chromium."
    exit 1
fi
EOF
    
    chmod +x "$LINUX_DIR/runmymodel-desktop.sh"
    
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
EOF
    
    print_success "Linux build completed: $LINUX_DIR"
}

# Function to build for Windows
build_windows() {
    print_status "Building for Windows..."
    
    # Create Windows executable directory
    WINDOWS_DIR="executables/windows"
    mkdir -p "$WINDOWS_DIR"
    
    # Copy frontend build
    cp -r dist "$WINDOWS_DIR/"
    
    # Create Windows batch launcher
    cat > "$WINDOWS_DIR/runmymodel-desktop.bat" << 'EOF'
@echo off
REM RunMyModel Desktop - Windows Launcher
REM This script launches the RunMyModel Desktop application

set "SCRIPT_DIR=%~dp0"
set "APP_URL=file:///%SCRIPT_DIR%dist/index.html"

REM Try to open with default browser
start "" "%APP_URL%"

REM If start command fails, try with specific browsers
if errorlevel 1 (
    echo Trying alternative browsers...
    
    REM Try Chrome
    if exist "C:\Program Files\Google\Chrome\Application\chrome.exe" (
        "C:\Program Files\Google\Chrome\Application\chrome.exe" "%APP_URL%"
        goto :end
    )
    
    REM Try Firefox
    if exist "C:\Program Files\Mozilla Firefox\firefox.exe" (
        "C:\Program Files\Mozilla Firefox\firefox.exe" "%APP_URL%"
        goto :end
    )
    
    REM Try Edge
    if exist "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe" (
        "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe" "%APP_URL%"
        goto :end
    )
    
    echo Error: No suitable browser found. Please install Chrome, Firefox, or Edge.
    pause
)

:end
EOF
    
    # Create Windows installer script (optional)
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
    
    print_success "Windows build completed: $WINDOWS_DIR"
}

# Function to create archive packages
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
    if [ -d "executables/windows" ]; then
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
    echo "RunMyModel Desktop - Universal Build Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --linux-only     Build only for Linux"
    echo "  --windows-only   Build only for Windows"
    echo "  --no-packages    Skip creating distribution packages"
    echo "  --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                    # Build for both platforms"
    echo "  $0 --linux-only       # Build only for Linux"
    echo "  $0 --windows-only     # Build only for Windows"
}

# Main function
main() {
    local BUILD_LINUX=true
    local BUILD_WINDOWS=true
    local CREATE_PACKAGES=true
    
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
            --no-packages)
                CREATE_PACKAGES=false
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
    
    print_status "Starting RunMyModel Desktop build process..."
    
    # Run build steps
    check_prerequisites
    install_dependencies
    build_frontend
    
    if [ "$BUILD_LINUX" = true ]; then
        build_linux
    fi
    
    if [ "$BUILD_WINDOWS" = true ]; then
        build_windows
    fi
    
    if [ "$CREATE_PACKAGES" = true ]; then
        create_packages
    fi
    
    print_success "Build process completed successfully!"
    print_status "Executables are available in the 'executables/' directory"
    
    if [ "$BUILD_LINUX" = true ]; then
        print_status "Linux: executables/linux/runmymodel-desktop.sh"
    fi
    
    if [ "$BUILD_WINDOWS" = true ]; then
        print_status "Windows: executables/windows/runmymodel-desktop.bat"
    fi
}

# Run main function with all arguments
main "$@"
