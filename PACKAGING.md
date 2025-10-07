# RunMyModel Desktop - Packaging Guide

This guide explains how to create distributable packages (.AppImage, .exe, .msi, .dmg) for RunMyModel Desktop.

## 🚀 Quick Start

### Package All Platforms
```bash
# Linux/macOS
./package-all.sh

# Windows
package-all.bat
```

### Package Individual Platforms
```bash
# Linux (AppImage + DEB)
npm run package:linux
./package-linux.sh

# Windows (EXE + MSI)
npm run package:windows
package-windows.bat

# macOS (APP + DMG)
npm run package:macos
./package-macos.sh
```

## 📦 Package Types

### Linux Packages
- **AppImage** - Portable executable, no installation required
- **DEB** - Debian/Ubuntu package for system integration

### Windows Packages
- **EXE** - Standalone executable
- **MSI** - Windows installer with system integration

### macOS Packages
- **APP** - Application bundle
- **DMG** - Disk image for distribution

## 🛠️ Prerequisites

### Required Tools
- Node.js 18+ and npm
- Rust (latest stable)
- Tauri CLI: `npm install -g @tauri-apps/cli`

### Platform-Specific Requirements

#### Linux
- `appimagetool` (for AppImage creation)
- `dpkg` (for DEB packages)

#### Windows
- Windows SDK (for MSI creation)
- NSIS (optional, for custom installers)

#### macOS
- Xcode Command Line Tools
- `create-dmg` (optional, for custom DMG)

## 📁 Package Structure

After packaging, you'll find files in the `packages/` directory:

```
packages/
├── RunMyModel-Desktop-1.0.0-x86_64.AppImage    # Linux AppImage
├── runmymodel-desktop_1.0.0_amd64.deb          # Linux DEB
├── RunMyModel Desktop_1.0.0_x64_en-US.msi     # Windows MSI
├── RunMyModel Desktop.exe                      # Windows EXE
├── RunMyModel Desktop_1.0.0_x64.dmg            # macOS Intel DMG
├── RunMyModel Desktop_1.0.0_aarch64.dmg       # macOS ARM DMG
├── RunMyModel Desktop.app                      # macOS App Bundle
└── README.txt                                   # Package information
```

## 🔧 Customization

### App Icons
Place icon files in `src-tauri/icons/`:
- `32x32.png` - Small icon
- `128x128.png` - Medium icon
- `128x128@2x.png` - High-res icon
- `icon.icns` - macOS icon bundle
- `icon.ico` - Windows icon file

### Ollama Binaries
Place Ollama binaries in `src-tauri/binaries/`:
- `ollama-windows-amd64.exe` - Windows
- `ollama-linux-amd64` - Linux
- `ollama-darwin-amd64` - macOS Intel
- `ollama-darwin-arm64` - macOS ARM

### Package Metadata
Edit `src-tauri/tauri.conf.json` to customize:
- App name and version
- Bundle identifier
- Copyright information
- System requirements

## 🚀 Distribution

### Testing Packages
1. **Test on target systems**
   - Linux: Ubuntu 18.04+, Fedora, Arch
   - Windows: Windows 10+, Windows 11
   - macOS: macOS 10.13+, Intel and Apple Silicon

2. **Verify functionality**
   - App launches correctly
   - Ollama integration works
   - Models can be downloaded
   - Chat interface functions

### Release Platforms

#### GitHub Releases
1. Create a new release
2. Upload all package files
3. Add release notes
4. Tag with version number

#### Package Managers
- **Snap Store** (Linux)
- **Microsoft Store** (Windows)
- **Mac App Store** (macOS)

#### Direct Distribution
- Website downloads
- CDN hosting
- File sharing services

## 🔍 Troubleshooting

### Common Issues

#### Build Failures
```bash
# Clear caches
npm run clean
cargo clean

# Reinstall dependencies
rm -rf node_modules
npm install
```

#### Missing Binaries
- Ensure Ollama binaries are in `src-tauri/binaries/`
- Check file permissions (Linux/macOS)
- Verify binary architecture

#### Icon Issues
- Check icon file formats
- Ensure all required sizes exist
- Verify file paths in config

#### Package Size
- Optimize assets
- Remove unnecessary files
- Use compression

### Debug Mode
```bash
# Enable debug logging
RUST_LOG=debug npm run tauri:dev

# Check build logs
npm run tauri:build -- --verbose
```

## 📋 Package Checklist

Before releasing packages:

- [ ] All platforms build successfully
- [ ] App icons display correctly
- [ ] Ollama binaries included
- [ ] App launches on target systems
- [ ] Model download works
- [ ] Chat interface functions
- [ ] No console errors
- [ ] Package sizes reasonable
- [ ] Installation instructions clear
- [ ] System requirements documented

## 🎯 Advanced Packaging

### Custom Installers
- **NSIS** (Windows) - Custom installer scripts
- **create-dmg** (macOS) - Custom DMG layouts
- **AppImageKit** (Linux) - Custom AppImage features

### Code Signing
- **Windows** - Authenticode certificates
- **macOS** - Apple Developer certificates
- **Linux** - GPG signatures

### Auto-Updates
- Implement update checking
- Download and install updates
- Version management

## 📚 Resources

- [Tauri Bundle Documentation](https://tauri.app/v1/guides/building/)
- [AppImage Documentation](https://docs.appimage.org/)
- [Windows MSI Creation](https://docs.microsoft.com/en-us/windows/msi/)
- [macOS DMG Creation](https://developer.apple.com/documentation/)

---

**Happy Packaging! 🎉**
