# RunMyModel Desktop

A modern Qt desktop application for running Large Language Models (LLMs) locally using Hugging Face models. Built with Qt6, C++, React, and TypeScript, featuring a clean, Open WebUI-inspired interface.

**Version**: 0.2.0 ALPHA

## 🚀 Features

- **Native Desktop App**: Built with Qt6 for true native performance
- **Modern UI**: Clean, responsive React interface inspired by Open WebUI
- **Local AI**: Run LLMs locally on your machine using Hugging Face models
- **Privacy-Focused**: All processing happens locally - no data sent to external servers
- **Cross-Platform**: Works on Linux and Windows
- **360+ Models**: Access to hundreds of pre-configured models from Hugging Face
- **Dark Mode**: Built-in dark/light theme support
- **Markdown Support**: Rich text rendering with syntax highlighting
- **Model Management**: Easy model selection, download, and management

## 📋 Prerequisites

### For Users
- **Linux**: Qt6 runtime libraries
- **Windows**: Bundled dependencies (no installation required)

### For Developers
- **Node.js**: Version 18 or higher
- **npm**: Comes with Node.js
- **Qt6**: Qt6 development packages (qt6-base-dev, qt6-webengine-dev)
- **CMake**: Version 3.16 or higher
- **C++ Compiler**: g++ or clang with C++17 support

## 🛠️ Installation

### Quick Start

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd runmymodel-desktop
   ```

2. **Build the application**:
   ```bash
   ./proper_build.sh
   ```
   Or use the full build script for packaging:
   ```bash
   ./build.sh
   ```

3. **Run the application**:
   - **From build directory**: `./build/RunMyModelDesktop`
   - **Linux package**: `./executables/linux/runmymodel-desktop.sh`
   - **Windows package**: `executables\windows\runmymodel-desktop.bat`

### Development Setup

1. **Install dependencies**:
   ```bash
   npm install
   ```

2. **Start development server**:
   ```bash
   npm run dev
   ```

3. **Build for production**:
   ```bash
   npm run build
   ```

## 📦 Building

The project includes a universal build script that creates executables for both Linux and Windows:

```bash
# Build for both platforms
./build.sh

# Build only for Linux
./build.sh --linux-only

# Build only for Windows
./build.sh --windows-only

# Skip creating distribution packages
./build.sh --no-packages
```

### Build Output

After building, you'll find the executables in the `executables/` directory:

```
executables/
├── linux/
│   ├── runmymodel-desktop.sh
│   ├── runmymodel-desktop.desktop
│   └── dist/
└── windows/
    ├── runmymodel-desktop.bat
    ├── install.bat
    └── dist/
```

## 🎯 Usage

### Linux

1. **Run directly**:
   ```bash
   ./executables/linux/runmymodel-desktop.sh
   ```

2. **Install system-wide**:
   ```bash
   # Copy desktop file to applications directory
   cp executables/linux/runmymodel-desktop.desktop ~/.local/share/applications/
   
   # Make executable
   chmod +x ~/.local/share/applications/runmymodel-desktop.desktop
   ```

### Windows

1. **Run directly**:
   ```cmd
   executables\windows\runmymodel-desktop.bat
   ```

2. **Install system-wide**:
   ```cmd
   executables\windows\install.bat
   ```

## 🏗️ Project Structure

```
runmymodel-desktop/
├── src/                    # Frontend source code (React/TypeScript)
│   ├── components/         # React components
│   │   ├── ui/            # Reusable UI components
│   │   ├── ModernChatWindow.tsx
│   │   └── ModernSidebar.tsx
│   ├── store/             # State management (Zustand)
│   ├── services/          # API services
│   └── types/             # TypeScript types
├── src-cpp/               # C++ backend source code
│   ├── include/           # C++ header files
│   │   ├── backend.h
│   │   ├── web_server.h
│   │   ├── model_manager.h
│   │   └── huggingface_client.h
│   └── src/               # C++ implementation files
├── build/                 # CMake build output (gitignored)
├── executables/           # Packaged executables (gitignored)
├── dist/                  # Frontend build output
├── llms.txt              # Model definitions (360+ models)
├── CMakeLists.txt        # CMake configuration
├── proper_build.sh       # Quick build script
├── build.sh              # Full build script with packaging
├── package.json          # Frontend dependencies
└── README.md             # This file
```

## 🔧 Development

### Available Scripts

- `npm run dev` - Start development server
- `npm run build` - Build for production
- `npm run preview` - Preview production build
- `npm run lint` - Run ESLint
- `npm run type-check` - Run TypeScript type checking

### Tech Stack

- **Desktop Framework**: Qt6 (C++)
- **Frontend**: React 18, TypeScript, Tailwind CSS
- **Build Tool**: Vite (frontend), CMake (C++ backend)
- **State Management**: Zustand
- **UI Components**: Custom components with Radix UI primitives
- **Styling**: Tailwind CSS with custom design system
- **Backend**: C++ with Qt6 WebEngine, Network, and WebChannel

## 📄 License

This project is licensed under the Mozilla Public License 2.0 (MPL-2.0). See the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Commit your changes: `git commit -am 'Add feature'`
4. Push to the branch: `git push origin feature-name`
5. Submit a pull request

## 🐛 Troubleshooting

### Common Issues

1. **Build fails**: 
   - Ensure Node.js 18+ is installed
   - Ensure Qt6 development packages are installed
   - Ensure CMake 3.16+ is installed
2. **App won't start**: 
   - Check that Qt6 runtime libraries are installed
   - Check that `llms.txt` is in the same directory as the executable
3. **0 models available**: 
   - Ensure `llms.txt` is in the build directory
   - Check console output for parsing errors
4. **Blank screen**: 
   - Ensure `dist/` folder is next to the executable
   - Check browser console for errors (Ctrl+Shift+I)

### Getting Help

- Check the [Issues](https://github.com/your-repo/issues) page
- Create a new issue with detailed information about your problem

## 📈 Roadmap

- [x] Native Qt desktop application
- [x] 360+ pre-configured models from Hugging Face
- [ ] Actual model download implementation
- [ ] Model inference engine (llama.cpp integration)
- [ ] Chat export/import functionality
- [ ] Plugin system for custom models
- [ ] Voice input/output support
- [ ] AppImage and Windows installer packaging

## 🙏 Acknowledgments

- Inspired by [Open WebUI](https://github.com/open-webui/open-webui) design patterns
- Built with [Hugging Face](https://huggingface.co/) models
- UI components based on [Radix UI](https://www.radix-ui.com/) primitives