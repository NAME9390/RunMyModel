#!/bin/bash
# RunMyModel Desktop v0.5.0-PRE-RELEASE - Master Build Script
# Builds both AppImage and MSI packages

set -e

echo "🚀 RunMyModel Desktop v0.5.0-PRE-RELEASE - Master Build Script"
echo "=============================================================="

# Configuration
APP_NAME="RunMyModelDesktop"
APP_VERSION="0.5.0-PRE-RELEASE"
BUILD_ALL=true
BUILD_APPIMAGE=false
BUILD_MSI=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --appimage)
            BUILD_APPIMAGE=true
            BUILD_ALL=false
            shift
            ;;
        --msi)
            BUILD_MSI=true
            BUILD_ALL=false
            shift
            ;;
        --all)
            BUILD_ALL=true
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --appimage    Build only AppImage package"
            echo "  --msi         Build only MSI package"
            echo "  --all         Build all packages (default)"
            echo "  --help        Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                    # Build all packages"
            echo "  $0 --appimage         # Build only AppImage"
            echo "  $0 --msi              # Build only MSI"
            echo "  $0 --all              # Build all packages"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Set build flags based on arguments
if [ "$BUILD_ALL" = true ]; then
    BUILD_APPIMAGE=true
    BUILD_MSI=true
fi

# Check if we're in the right directory
if [ ! -f "build_appimage.sh" ] || [ ! -f "build_msi.sh" ]; then
    echo "❌ Error: This script must be run from the executables directory"
    echo "Current directory: $(pwd)"
    echo "Expected files: build_appimage.sh, build_msi.sh"
    exit 1
fi

# Create build log directory
mkdir -p build_logs
LOG_DIR="build_logs/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$LOG_DIR"

echo "📁 Build logs will be saved to: $LOG_DIR"
echo ""

# Build AppImage if requested
if [ "$BUILD_APPIMAGE" = true ]; then
    echo "🔨 Building AppImage package..."
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    if ./build_appimage.sh 2>&1 | tee "$LOG_DIR/appimage_build.log"; then
        echo "✅ AppImage build completed successfully"
        
        # Move AppImage to final location
        if [ -f "RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage" ]; then
            mv "RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage" "$LOG_DIR/"
            echo "📦 AppImage saved to: $LOG_DIR/RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage"
        fi
    else
        echo "❌ AppImage build failed"
        echo "📋 Check the log: $LOG_DIR/appimage_build.log"
    fi
    
    echo ""
fi

# Build MSI if requested
if [ "$BUILD_MSI" = true ]; then
    echo "🔨 Building MSI package..."
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    if ./build_msi.sh 2>&1 | tee "$LOG_DIR/msi_build.log"; then
        echo "✅ MSI build completed successfully"
        
        # Move MSI to final location
        if [ -f "RunMyModelDesktop-v$APP_VERSION-x64.msi" ]; then
            mv "RunMyModelDesktop-v$APP_VERSION-x64.msi" "$LOG_DIR/"
            echo "📦 MSI saved to: $LOG_DIR/RunMyModelDesktop-v$APP_VERSION-x64.msi"
        fi
    else
        echo "❌ MSI build failed"
        echo "📋 Check the log: $LOG_DIR/msi_build.log"
    fi
    
    echo ""
fi

# Create build summary
echo "📊 Creating build summary..."
cat > "$LOG_DIR/build_summary.txt" << EOF
RunMyModel Desktop v$APP_VERSION - Build Summary
===============================================

Build Date: $(date)
Build Host: $(hostname)
Build User: $(whoami)
Build OS: $(uname -a)

Build Configuration:
- AppImage: $BUILD_APPIMAGE
- MSI: $BUILD_MSI

Build Results:
EOF

if [ "$BUILD_APPIMAGE" = true ]; then
    if [ -f "$LOG_DIR/RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage" ]; then
        APPIMAGE_SIZE=$(du -h "$LOG_DIR/RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage" | cut -f1)
        echo "- AppImage: ✅ Success ($APPIMAGE_SIZE)" >> "$LOG_DIR/build_summary.txt"
    else
        echo "- AppImage: ❌ Failed" >> "$LOG_DIR/build_summary.txt"
    fi
fi

if [ "$BUILD_MSI" = true ]; then
    if [ -f "$LOG_DIR/RunMyModelDesktop-v$APP_VERSION-x64.msi" ]; then
        MSI_SIZE=$(du -h "$LOG_DIR/RunMyModelDesktop-v$APP_VERSION-x64.msi" | cut -f1)
        echo "- MSI: ✅ Success ($MSI_SIZE)" >> "$LOG_DIR/build_summary.txt"
    else
        echo "- MSI: ❌ Failed" >> "$LOG_DIR/build_summary.txt"
    fi
fi

echo "" >> "$LOG_DIR/build_summary.txt"
echo "Build Artifacts:" >> "$LOG_DIR/build_summary.txt"
ls -la "$LOG_DIR"/*.AppImage "$LOG_DIR"/*.msi 2>/dev/null >> "$LOG_DIR/build_summary.txt" || true

echo "" >> "$LOG_DIR/build_summary.txt"
echo "Log Files:" >> "$LOG_DIR/build_summary.txt"
ls -la "$LOG_DIR"/*.log >> "$LOG_DIR/build_summary.txt"

# Display build summary
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 BUILD SUMMARY"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cat "$LOG_DIR/build_summary.txt"

# Create release notes
echo ""
echo "📝 Creating release notes..."
cat > "$LOG_DIR/RELEASE_NOTES.md" << EOF
# RunMyModel Desktop v$APP_VERSION - Release Notes

## Build Information
- **Build Date**: $(date)
- **Build Host**: $(hostname)
- **Build OS**: $(uname -a)

## Packages Included

EOF

if [ "$BUILD_APPIMAGE" = true ] && [ -f "$LOG_DIR/RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage" ]; then
    echo "### AppImage Package" >> "$LOG_DIR/RELEASE_NOTES.md"
    echo "- **File**: RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage" >> "$LOG_DIR/RELEASE_NOTES.md"
    echo "- **Platform**: Linux x86_64" >> "$LOG_DIR/RELEASE_NOTES.md"
    echo "- **Size**: $(du -h "$LOG_DIR/RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage" | cut -f1)" >> "$LOG_DIR/RELEASE_NOTES.md"
    echo "- **Usage**: chmod +x RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage && ./RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage" >> "$LOG_DIR/RELEASE_NOTES.md"
    echo "" >> "$LOG_DIR/RELEASE_NOTES.md"
fi

if [ "$BUILD_MSI" = true ] && [ -f "$LOG_DIR/RunMyModelDesktop-v$APP_VERSION-x64.msi" ]; then
    echo "### MSI Package" >> "$LOG_DIR/RELEASE_NOTES.md"
    echo "- **File**: RunMyModelDesktop-v$APP_VERSION-x64.msi" >> "$LOG_DIR/RELEASE_NOTES.md"
    echo "- **Platform**: Windows x64" >> "$LOG_DIR/RELEASE_NOTES.md"
    echo "- **Size**: $(du -h "$LOG_DIR/RunMyModelDesktop-v$APP_VERSION-x64.msi" | cut -f1)" >> "$LOG_DIR/RELEASE_NOTES.md"
    echo "- **Usage**: Double-click to install on Windows" >> "$LOG_DIR/RELEASE_NOTES.md"
    echo "" >> "$LOG_DIR/RELEASE_NOTES.md"
fi

cat >> "$LOG_DIR/RELEASE_NOTES.md" << EOF
## Installation Instructions

### Linux (AppImage)
1. Download the AppImage file
2. Make it executable: \`chmod +x RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage\`
3. Run: \`./RunMyModelDesktop-v$APP_VERSION-x86_64.AppImage\`

### Windows (MSI)
1. Download the MSI file
2. Double-click to run the installer
3. Follow the installation wizard
4. Launch from Start Menu or Desktop shortcut

## System Requirements

### Linux
- Ubuntu 20.04+ or equivalent
- 8GB RAM minimum (16GB recommended)
- x86_64 processor

### Windows
- Windows 10 or later
- 8GB RAM minimum (16GB recommended)
- x64 processor

## Known Issues

- This is a pre-release version
- Some features may not be fully functional
- Performance may vary depending on hardware

## Support

- **Documentation**: [docs/](docs/)
- **Issues**: [GitHub Issues](https://github.com/runmymodel/runmymodel-desktop/issues)
- **Community**: [Discord Server](https://discord.gg/runmymodel)

## Changelog

See [CHANGELOG.md](../../CHANGELOG.md) for detailed changes.

---

**RunMyModel Desktop v$APP_VERSION** - Built with ❤️ by the RunMyModel team
EOF

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ BUILD COMPLETE!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📁 Build artifacts saved to: $LOG_DIR"
echo "📋 Build summary: $LOG_DIR/build_summary.txt"
echo "📝 Release notes: $LOG_DIR/RELEASE_NOTES.md"
echo ""
echo "🚀 Ready for distribution!"
echo ""
