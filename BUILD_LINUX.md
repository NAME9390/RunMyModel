# RunMyModel Desktop - Linux Build Guide

This guide explains how to build RunMyModel Desktop for Linux systems, creating both AppImage and DEB packages.

## Prerequisites

### Arch Linux (Recommended)
The build script is optimized for Arch Linux. Install the following packages:

```bash
sudo pacman -S base-devel curl openssl pkg-config gtk3 webkit2gtk librsvg appmenu-gtk-module libayatana-appindicator-gtk3 nodejs npm rust
```

### Ubuntu/Debian
```bash
sudo apt install build-essential curl libssl-dev pkg-config libgtk-3-dev libwebkit2gtk-4.1-dev librsvg2-dev nodejs npm
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
```

### Fedora
```bash
sudo dnf install gcc-c++ curl openssl-devel pkgconfig gtk3-devel webkit2gtk3-devel librsvg2-devel nodejs npm
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
```

## Building

### Quick Build (Recommended)
```bash
./build_linux.sh
```

### Manual Build Steps
1. Install dependencies: `npm install`
2. Build frontend: `npm run build`
3. Build Tauri app: `npm run build:linux`

## Output

The build process creates several distribution formats:

- **AppImage**: `RunMyModel Desktop_1.0.0_amd64.AppImage` - Portable, no installation required
- **DEB Package**: `RunMyModel Desktop_1.0.0_amd64.deb` - For Debian/Ubuntu systems
- **Binary**: `runmymodel-desktop-linux` - Raw executable
- **Archive**: `runmymodel-desktop-linux.tar.gz` - Compressed binary

All files are placed in the `final-packages/` directory.

## Testing

### AppImage
```bash
chmod +x final-packages/*.AppImage
./final-packages/*.AppImage
```

### DEB Package
```bash
sudo dpkg -i final-packages/*.deb
sudo apt-get install -f  # Install dependencies if needed
```

### Binary
```bash
chmod +x final-packages/runmymodel-desktop-linux
./final-packages/runmymodel-desktop-linux
```

## Troubleshooting

### Build Fails
1. Ensure all dependencies are installed
2. Check that Rust and Node.js are properly installed
3. Verify you're in the correct directory (`runmymodel-desktop/`)

### AppImage Won't Run
1. Make sure it's executable: `chmod +x *.AppImage`
2. Check if your system supports AppImages
3. Try running from terminal to see error messages

### DEB Installation Issues
1. Install dependencies: `sudo apt-get install -f`
2. Check for conflicting packages
3. Verify architecture compatibility (amd64)

## Development

For development builds:
```bash
npm run tauri:dev
```

This starts the development server with hot reloading.
