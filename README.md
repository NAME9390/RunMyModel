# RunMyModel Desktop

A powerful desktop application that allows you to run large language models locally using Ollama. Chat with models like LLaMA, Mistral, CodeLlama, and more in a beautiful interface.

![RunMyModel Desktop](https://img.shields.io/badge/Version-1.0.0-blue)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey)

## ✨ Features

- **Local LLM Execution**: Run models locally using Ollama CLI integration
- **ChatGPT-Style Interface**: Beautiful, responsive chat interface with markdown support
- **Model Management**: Download, manage, and switch between different models
- **System Prompts**: Pre-built prompts for different use cases (Creative, Coding, Research, etc.)
- **Real-time Progress**: Track model downloads and generation progress
- **Dark/Light Themes**: Toggle between themes for comfortable usage
- **Cross-Platform**: Works on Windows, macOS, and Linux
- **Offline Capable**: Once models are downloaded, works completely offline
- **Professional Design**: Clean, modern UI with RunMyModel.org branding

## 🚀 Quick Start

### Prerequisites

- Node.js 18+ and npm
- Rust (for Tauri development)
- Git
- curl (for downloading dependencies)

### One-Command Setup & Packaging

```bash
# Clone and setup everything automatically
git clone https://github.com/runmymodel/runmymodel-desktop.git
cd runmymodel-desktop
./setup-and-package.sh
```

This script will:
- ✅ Install all dependencies
- ✅ Download Ollama binaries automatically
- ✅ Create placeholder icons
- ✅ Build the application
- ✅ Package for all platforms (.AppImage, .exe, .msi, .dmg)
- ✅ Create ready-to-distribute packages

### Manual Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/runmymodel/runmymodel-desktop.git
   cd runmymodel-desktop
   ```

2. **Install dependencies**
   ```bash
   npm install
   ```

3. **Download Ollama binaries**
   ```bash
   ./download-ollama.sh
   ```
   
   Or manually download from [GitHub releases](https://github.com/ollama/ollama/releases) and place in `src-tauri/binaries/`

4. **Create application icons**
   
   Create the required icon files in `src-tauri/icons/`:
   - `32x32.png`, `128x128.png`, `128x128@2x.png`
   - `icon.icns` (macOS), `icon.ico` (Windows)

### Development

```bash
# Start development server
npm run tauri:dev

# Build for production
npm run tauri:build

# Build for specific platforms
npm run build:windows
npm run build:linux
npm run build:macos
npm run build:macos-arm
```

## 🏗️ Building Executables

### One-Command Packaging (Recommended)

```bash
# Package everything automatically
./setup-and-package.sh
```

### Individual Platform Packaging

```bash
# Package all platforms
./package-all.sh

# Package specific platforms
./package-linux.sh      # Creates .AppImage and .deb
./package-windows.bat    # Creates .exe and .msi
./package-macos.sh       # Creates .app and .dmg
```

### Manual Build

```bash
# Install dependencies
npm install

# Build frontend
npm run build

# Build Tauri app
npm run tauri:build
```

### Output Locations

- **Linux**: `src-tauri/target/x86_64-unknown-linux-gnu/release/bundle/`
- **Windows**: `src-tauri/target/x86_64-pc-windows-msvc/release/bundle/`
- **macOS Intel**: `src-tauri/target/x86_64-apple-darwin/release/bundle/`
- **macOS ARM**: `src-tauri/target/aarch64-apple-darwin/release/bundle/`
- **Packages**: `packages/` (after running packaging scripts)

## 🎯 Supported Models

RunMyModel Desktop supports all models available in Ollama's library:

### Popular Models
- **LLaMA 3.2** (3B, 1B) - Meta's latest language model
- **Mistral 7B** - High-performance open-source model
- **CodeLlama 7B** - Specialized for code generation
- **Phi-3 Mini** - Microsoft's efficient small model
- **Gemma 2B** - Google's lightweight model
- **TinyLlama 1.1B** - Ultra-lightweight model
- **Qwen 2.5** - Alibaba's multilingual model
- **DeepSeek Coder** - Specialized coding assistant
- **Starling LM** - Fine-tuned conversational model

### Model Categories
- **General Purpose**: LLaMA, Mistral, Gemma
- **Code Generation**: CodeLlama, DeepSeek Coder, StarCoder
- **Multimodal**: LLaVA, BakLLaVA
- **Specialized**: Medical, Legal, Creative writing models
- **Enterprise**: Granite, Ring, Ultra-Large models

## 🎨 System Prompts

Choose from pre-built system prompts or create your own:

- **Creative Writer**: Storytelling and artistic expression
- **Code Assistant**: Programming and debugging help
- **Data Analyst**: Data analysis and research
- **Research Assistant**: Academic research and fact-checking
- **General Assistant**: Helpful, harmless, and honest AI

## ⚙️ Configuration

The app stores configuration in local storage:

```json
{
  "theme": "light|dark",
  "currentModel": "llama3.2:3b",
  "sidebarCollapsed": false,
  "chats": [...],
  "systemPrompts": [...]
}
```

## 🛠️ Technology Stack

### Frontend
- **React 18** - UI framework
- **TypeScript** - Type safety
- **Tailwind CSS** - Styling
- **Zustand** - State management
- **React Markdown** - Markdown rendering
- **Lucide React** - Icons

### Backend
- **Tauri** - Desktop app framework
- **Rust** - System integration
- **Ollama CLI** - LLM execution engine

### Build Tools
- **Vite** - Frontend bundler
- **Tauri CLI** - Desktop app builder
- **Cross-platform** - Windows, macOS, Linux

## 📁 Project Structure

```
runmymodel-desktop/
├── src/                          # Frontend source
│   ├── components/               # React components
│   │   ├── Sidebar.tsx          # Main sidebar
│   │   ├── ChatWindow.tsx       # Chat interface
│   │   ├── ModelManager.tsx     # Model management
│   │   ├── SystemPromptSelector.tsx
│   │   ├── AboutModal.tsx       # About dialog
│   │   └── SplashScreen.tsx     # Loading screen
│   ├── store/                   # State management
│   │   ├── appStore.ts          # App state
│   │   ├── chatStore.ts         # Chat state
│   │   └── modelStore.ts        # Model state
│   ├── services/                # Services
│   │   └── ollamaService.ts     # Ollama integration
│   ├── types/                   # TypeScript types
│   │   └── system.ts            # System types
│   └── lib/                     # Utilities
│       └── utils.ts             # Helper functions
├── src-tauri/                   # Backend source
│   ├── src/
│   │   └── main.rs              # Rust backend
│   ├── binaries/                # Ollama binaries
│   ├── icons/                   # App icons
│   ├── Cargo.toml               # Rust dependencies
│   └── tauri.conf.json          # Tauri config
├── build.sh                     # Linux/macOS build script
├── build.bat                    # Windows build script
└── package.json                 # Node.js dependencies
```

## 🔧 Development

### Adding New Features

1. **Frontend**: Add components in `src/components/`
2. **Backend**: Add Tauri commands in `src-tauri/src/main.rs`
3. **State**: Update stores in `src/store/`
4. **Types**: Add types in `src/types/`

### Debugging

```bash
# Enable debug logging
RUST_LOG=debug npm run tauri:dev

# Check console logs
# Open DevTools in the app (Ctrl+Shift+I)
```

### Testing

```bash
# Type checking
npm run type-check

# Linting
npm run lint

# Build verification
npm run build
```

## 🚀 Deployment

### Creating Installers

The built executables can be packaged into installers:

- **Windows**: Use NSIS or WiX for `.msi` installers
- **macOS**: Use `create-dmg` for `.dmg` files
- **Linux**: Use `appimagetool` for `.AppImage` files

### Distribution

1. Build executables for all platforms
2. Create installers/packages
3. Test on target systems
4. Upload to release platforms
5. Update documentation

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request


## 🙏 Acknowledgments

- [Ollama](https://ollama.ai) - For the amazing LLM runtime
- [Tauri](https://tauri.app) - For the desktop app framework
- [RunMyModel.org](https://runmymodel.org) - For inspiration and support

## 📞 Support

- **Website**: [RunMyModel.org](https://runmymodel.org)

## 🔗 Links

- [RunMyModel.org](https://runmymodel.org) - Main website
- [Ollama](https://ollama.ai) - LLM runtime
- [Tauri](https://tauri.app) - Desktop app framework
- [Model Library](https://ollama.ai/library) - Available models

---

Made with ❤️ by [RunMyModel.org](https://runmymodel.org)
