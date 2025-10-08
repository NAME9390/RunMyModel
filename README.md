# RunMyModel Desktop

A modern desktop application for running Large Language Models (LLMs) locally using Hugging Face models. Built with React and TypeScript, featuring a clean, Open WebUI-inspired interface.

## 🚀 Features

- **Modern UI**: Clean, responsive interface inspired by Open WebUI
- **Local AI**: Run LLMs locally on your machine using Hugging Face models
- **Privacy-Focused**: All processing happens locally - no data sent to external servers
- **Cross-Platform**: Works on Linux and Windows
- **Dark Mode**: Built-in dark/light theme support
- **Markdown Support**: Rich text rendering with syntax highlighting
- **Model Management**: Easy model selection and management

## 📋 Prerequisites

- **Node.js**: Version 18 or higher
- **npm**: Comes with Node.js
- **Modern Browser**: Chrome, Firefox, Edge, or Chromium

## 🛠️ Installation

### Quick Start

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd runmymodel-desktop
   ```

2. **Build the application**:
   ```bash
   ./build.sh
   ```

3. **Run the application**:
   - **Linux**: `./executables/linux/runmymodel-desktop.sh`
   - **Windows**: `executables\windows\runmymodel-desktop.bat`

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
├── src/                    # Source code
│   ├── components/         # React components
│   │   ├── ui/            # Reusable UI components
│   │   ├── ModernChatWindow.tsx
│   │   └── ModernSidebar.tsx
│   ├── store/             # State management
│   ├── services/          # API services
│   └── types/             # TypeScript types
├── executables/           # Built executables (gitignored)
├── dist/                  # Frontend build output
├── build.sh              # Universal build script
├── package.json          # Dependencies and scripts
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

- **Frontend**: React 18, TypeScript, Tailwind CSS
- **Build Tool**: Vite
- **State Management**: Zustand
- **UI Components**: Custom components with Radix UI primitives
- **Styling**: Tailwind CSS with custom design system

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

1. **Build fails**: Ensure Node.js 18+ is installed
2. **App won't start**: Check that a modern browser is installed
3. **Models not loading**: Verify internet connection for initial model downloads

### Getting Help

- Check the [Issues](https://github.com/your-repo/issues) page
- Create a new issue with detailed information about your problem

## 📈 Roadmap

- [ ] Native desktop app wrapper (Electron/Tauri)
- [ ] Model download progress indicators
- [ ] Chat export/import functionality
- [ ] Plugin system for custom models
- [ ] Voice input/output support

## 🙏 Acknowledgments

- Inspired by [Open WebUI](https://github.com/open-webui/open-webui) design patterns
- Built with [Hugging Face](https://huggingface.co/) models
- UI components based on [Radix UI](https://www.radix-ui.com/) primitives