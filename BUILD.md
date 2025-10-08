# RunMyModel Desktop - Build Instructions

This project builds a Qt/C++ desktop application with React frontend, creating both AppImage and Windows .exe distributions.

## Quick Build

To build both AppImage and Windows .exe:

```bash
chmod +x build_all.sh
./build_all.sh
```

## Individual Builds

### AppImage (Linux)
```bash
chmod +x build_appimage.sh
./build_appimage.sh
```

### Windows .exe
```bash
chmod +x build_windows.sh
./build_windows.sh
```

### Standard Qt Build
```bash
chmod +x build.sh
./build.sh --clean
```

## Prerequisites

### Required
- Node.js 18+
- npm
- Qt6 (qt6-base, qt6-webengine, qt6-webchannel)
- CMake
- C++ compiler (g++)

### Optional (for distributions)
- **AppImage**: linuxdeploy
- **Windows .exe**: NSIS

## Installation Commands

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install nodejs npm qt6-base-dev qt6-webengine-dev qt6-webchannel-dev cmake build-essential nsis
```

### Arch Linux
```bash
sudo pacman -S nodejs npm qt6-base qt6-webengine qt6-webchannel cmake base-devel nsis
```

### Install linuxdeploy (for AppImage)
```bash
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
sudo mv linuxdeploy-x86_64.AppImage /usr/local/bin/linuxdeploy
```

## Build Outputs

After building, you'll find:

- **AppImage**: `RunMyModelDesktop-*.AppImage`
- **Windows Installer**: `RunMyModelDesktop-Setup.exe`
- **Linux Package**: `executables/linux/`
- **Windows Package**: `executables/windows/`

## Project Structure

```
runmymodel-desktop/
├── src/                    # React frontend
├── src-cpp/               # Qt/C++ backend
├── build/                 # Qt build output
├── dist/                  # Frontend build output
├── executables/           # Distribution packages
├── appimage/              # AppImage configuration
├── windows/               # Windows installer configuration
├── build.sh              # Main build script
├── build_appimage.sh      # AppImage build script
├── build_windows.sh      # Windows .exe build script
└── build_all.sh          # Complete build script
```

## Development

### Frontend Development
```bash
npm run dev
```

### Qt Development
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./RunMyModelDesktop
```
