# RunMyModel Desktop - Development Setup

This guide will help you set up RunMyModel Desktop for development.

## Prerequisites

- Node.js 18+ and npm
- Rust (latest stable)
- Git
- Ollama CLI (for testing)

## Quick Setup

1. **Clone and install**
   ```bash
   git clone https://github.com/runmymodel/runmymodel-desktop.git
   cd runmymodel-desktop
   npm install
   ```

2. **Download Ollama binaries**
   
   Get the latest Ollama binaries from [GitHub releases](https://github.com/ollama/ollama/releases):
   
   ```bash
   # Create binaries directory
   mkdir -p src-tauri/binaries
   
   # Download binaries (replace URLs with latest versions)
   curl -L -o src-tauri/binaries/ollama-linux-amd64 https://github.com/ollama/ollama/releases/latest/download/ollama-linux-amd64
   curl -L -o src-tauri/binaries/ollama-windows-amd64.exe https://github.com/ollama/ollama/releases/latest/download/ollama-windows-amd64.exe
   curl -L -o src-tauri/binaries/ollama-darwin-amd64 https://github.com/ollama/ollama/releases/latest/download/ollama-darwin-amd64
   curl -L -o src-tauri/binaries/ollama-darwin-arm64 https://github.com/ollama/ollama/releases/latest/download/ollama-darwin-arm64
   
   # Make Linux binary executable
   chmod +x src-tauri/binaries/ollama-linux-amd64
   chmod +x src-tauri/binaries/ollama-darwin-amd64
   chmod +x src-tauri/binaries/ollama-darwin-arm64
   ```

3. **Create application icons**
   
   You'll need to create icon files in `src-tauri/icons/`:
   - `32x32.png`
   - `128x128.png`
   - `128x128@2x.png`
   - `icon.icns` (macOS)
   - `icon.ico` (Windows)
   
   For now, you can use placeholder icons or create simple ones using online tools.

4. **Start development**
   ```bash
   npm run tauri:dev
   ```

## Development Commands

```bash
# Development server
npm run tauri:dev

# Build frontend only
npm run build

# Build for production
npm run tauri:build

# Build for specific platforms
npm run build:windows
npm run build:linux
npm run build:macos
npm run build:macos-arm

# Type checking
npm run type-check

# Linting
npm run lint
```

## Project Structure

```
runmymodel-desktop/
├── src/                    # Frontend (React + TypeScript)
│   ├── components/         # React components
│   ├── store/             # Zustand stores
│   ├── services/          # API services
│   ├── types/             # TypeScript types
│   └── lib/               # Utilities
├── src-tauri/             # Backend (Rust + Tauri)
│   ├── src/               # Rust source
│   ├── binaries/          # Ollama binaries
│   ├── icons/             # App icons
│   └── Cargo.toml         # Rust dependencies
├── build.sh               # Build script (Linux/macOS)
├── build.bat              # Build script (Windows)
└── package.json           # Node.js dependencies
```

## Troubleshooting

### Common Issues

1. **Rust not found**
   ```bash
   # Install Rust
   curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
   source ~/.cargo/env
   ```

2. **Tauri CLI not found**
   ```bash
   # Install Tauri CLI
   npm install -g @tauri-apps/cli
   ```

3. **Ollama binary not found**
   - Ensure binaries are in `src-tauri/binaries/`
   - Check file permissions (Linux/macOS)
   - Verify binary architecture matches your system

4. **Build errors**
   - Clear node_modules: `rm -rf node_modules && npm install`
   - Clear Rust cache: `cargo clean`
   - Check Rust version: `rustc --version`

### Debug Mode

```bash
# Enable debug logging
RUST_LOG=debug npm run tauri:dev

# Check console logs in DevTools (Ctrl+Shift+I)
```

## Next Steps

1. **Test the app**: Try downloading and running a model
2. **Make changes**: Modify components or add features
3. **Build**: Create production builds for your platform
4. **Contribute**: Submit pull requests for improvements

## Resources

- [Tauri Documentation](https://tauri.app/v1/guides/)
- [React Documentation](https://react.dev/)
- [Ollama Documentation](https://ollama.ai/docs)
- [RunMyModel.org](https://runmymodel.org)

Happy coding! 🚀
