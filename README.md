# RunMyModel Desktop v0.5.0-PRE-RELEASE - Executables Directory

This directory contains build scripts and tools for creating distributable executables for RunMyModel Desktop.

## 📦 Available Packages

### AppImage (Linux)
- **File**: `RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage`
- **Platform**: Linux x86_64
- **Usage**: Portable executable that runs on most Linux distributions
- **Build Script**: `build_appimage.sh`

### MSI (Windows)
- **File**: `RunMyModelDesktop-v0.5.0-PRE-RELEASE-x64.msi`
- **Platform**: Windows x64
- **Usage**: Windows installer package
- **Build Script**: `build_msi.sh`

## 🛠️ Build Scripts

### Quick Build
```bash
# Build all packages
./build_all.sh

# Build only AppImage
./build_all.sh --appimage

# Build only MSI
./build_all.sh --msi
```

### Individual Builds
```bash
# Build AppImage
./build_appimage.sh

# Build MSI (requires Windows or Wine)
./build_msi.sh

# Setup cross-compilation environment
./setup_cross_compile.sh
```

## 📋 Prerequisites

### For AppImage Build
- Linux system (Ubuntu/Debian, Arch Linux, or Fedora)
- Qt6 development packages
- GCC/G++ with C++17 support
- CMake 3.10+
- linuxdeploy and linuxdeploy-plugin-qt

### For MSI Build
- Windows system OR Linux with Wine
- NSIS (Nullsoft Scriptable Install System)
- Qt6 for Windows
- Visual Studio Build Tools (for Windows)

### For Cross-Compilation
- Linux system
- MinGW-w64 toolchain
- Wine for Windows compatibility
- Qt6 for Windows

## 🔧 Build Process

### 1. AppImage Build
1. Builds the main application using `../app/build.sh`
2. Creates AppDir structure with all dependencies
3. Generates desktop file and AppStream metadata
4. Creates AppRun script for portable execution
5. Packages everything into a single AppImage file

### 2. MSI Build
1. Builds the main application (Windows executable)
2. Creates Windows installer structure
3. Generates NSIS installer script
4. Includes Qt libraries and dependencies
5. Creates Windows shortcuts and registry entries
6. Packages everything into an MSI installer

### 3. Cross-Compilation Setup
1. Installs MinGW-w64 toolchain
2. Downloads Qt for Windows
3. Sets up Wine environment
4. Creates cross-compilation scripts
5. Provides Windows library dependencies

## 📁 Directory Structure

```
executables/
├── build_all.sh              # Master build script
├── build_appimage.sh         # AppImage build script
├── build_msi.sh              # MSI build script
├── setup_cross_compile.sh    # Cross-compilation setup
├── build_logs/               # Build logs and artifacts
│   └── YYYYMMDD_HHMMSS/      # Timestamped build directory
│       ├── *.AppImage        # AppImage package
│       ├── *.msi             # MSI package
│       ├── build_summary.txt # Build summary
│       ├── RELEASE_NOTES.md  # Release notes
│       └── *.log             # Build logs
└── README.md                 # This file
```

## 🚀 Usage Instructions

### Building Packages

1. **Navigate to executables directory**:
   ```bash
   cd executables
   ```

2. **Build all packages**:
   ```bash
   ./build_all.sh
   ```

3. **Check build results**:
   ```bash
   ls -la build_logs/latest/
   ```

### Distributing Packages

1. **AppImage Distribution**:
   - Upload to GitHub Releases
   - Make executable: `chmod +x RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage`
   - Run: `./RunMyModelDesktop-v0.5.0-PRE-RELEASE-x86_64.AppImage`

2. **MSI Distribution**:
   - Upload to GitHub Releases
   - Users double-click to install
   - Creates Start Menu and Desktop shortcuts

## 🔍 Troubleshooting

### Common Issues

1. **AppImage Build Fails**:
   - Check Qt6 installation
   - Verify linuxdeploy is installed
   - Ensure all dependencies are available

2. **MSI Build Fails**:
   - Verify NSIS installation
   - Check Windows build environment
   - Ensure Qt for Windows is installed

3. **Cross-Compilation Issues**:
   - Verify MinGW-w64 installation
   - Check Wine configuration
   - Ensure Qt for Windows is properly installed

### Getting Help

- Check build logs in `build_logs/` directory
- Review error messages in terminal output
- Consult main project documentation
- Check GitHub Issues for known problems

## 📊 Build Statistics

### Package Sizes (Approximate)
- **AppImage**: ~150MB (includes Qt6 and all dependencies)
- **MSI**: ~200MB (includes Qt6, Windows libraries, and installer)

### Build Times (Approximate)
- **AppImage**: 5-10 minutes
- **MSI**: 10-15 minutes
- **Cross-Compilation Setup**: 30-60 minutes (one-time)

## 🔄 Continuous Integration

These build scripts are designed to work with CI/CD systems:

- **GitHub Actions**: Use Linux runners for AppImage, Windows runners for MSI
- **GitLab CI**: Use Docker containers with Qt6 and build tools
- **Jenkins**: Use dedicated build agents with required dependencies

## 📝 Notes

- All build scripts include comprehensive error handling
- Build logs are automatically saved with timestamps
- Release notes are generated automatically
- Cross-compilation is experimental and may require manual intervention
- For production builds, consider using dedicated build environments

---

**RunMyModel Desktop v0.5.0-PRE-RELEASE** - Professional AI for your desktop

*Built with ❤️ by the RunMyModel team*
