# Contributing to RunMyModel Desktop

Thank you for your interest in contributing to RunMyModel Desktop! This document provides guidelines and information for contributors.


## 🚀 Getting Started

### Prerequisites

- Node.js 18+ and npm
- Rust (latest stable)
- Git
- Basic knowledge of React, TypeScript, and Rust

### Development Setup

1. **Fork and clone the repository**
   ```bash
   git clone https://github.com/YOUR_USERNAME/runmymodel-desktop.git
   cd runmymodel-desktop
   ```

2. **Install dependencies**
   ```bash
   npm install
   ```

3. **Set up Ollama binaries**
   
   Download Ollama binaries and place them in `src-tauri/binaries/`:
   - `ollama-windows-amd64.exe`
   - `ollama-linux-amd64`
   - `ollama-darwin-amd64`
   - `ollama-darwin-arm64`

4. **Create icons**
   
   Add the required icon files in `src-tauri/icons/`:
   - `32x32.png`
   - `128x128.png`
   - `128x128@2x.png`
   - `icon.icns`
   - `icon.ico`

5. **Start development**
   ```bash
   npm run tauri:dev
   ```

## 📋 Contribution Guidelines

### Code Style

- **TypeScript**: Use strict typing, avoid `any`
- **React**: Use functional components with hooks
- **Rust**: Follow Rust conventions and use `cargo fmt`
- **CSS**: Use Tailwind CSS classes, avoid custom CSS

### Commit Messages

Use conventional commits:
- `feat:` - New features
- `fix:` - Bug fixes
- `docs:` - Documentation changes
- `style:` - Code style changes
- `refactor:` - Code refactoring
- `test:` - Adding tests
- `chore:` - Build process or auxiliary tool changes

Examples:
```
feat: add dark mode toggle
fix: resolve model download progress issue
docs: update README with new features
```

### Pull Request Process

1. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes**
   - Write clean, documented code
   - Add tests if applicable
   - Update documentation

3. **Test your changes**
   ```bash
   npm run type-check
   npm run lint
   npm run build
   ```

4. **Commit your changes**
   ```bash
   git add .
   git commit -m "feat: add your feature"
   ```

5. **Push and create PR**
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create a pull request**
   - Provide a clear description
   - Link any related issues
   - Include screenshots for UI changes

## 🎯 Areas for Contribution

### Frontend (React/TypeScript)
- **UI Components**: New components, improvements to existing ones
- **State Management**: Zustand store improvements
- **Styling**: Tailwind CSS improvements, animations
- **Accessibility**: ARIA labels, keyboard navigation
- **Performance**: Optimization, lazy loading

### Backend (Rust/Tauri)
- **Ollama Integration**: New commands, better error handling
- **System Detection**: GPU detection, system info
- **File Management**: Model storage, configuration
- **Performance**: Async operations, memory management

### Documentation
- **README**: Updates, new sections
- **Code Comments**: Inline documentation
- **API Docs**: Tauri command documentation
- **User Guides**: Tutorials, troubleshooting

### Testing
- **Unit Tests**: Component testing
- **Integration Tests**: Tauri command testing
- **E2E Tests**: Full application testing
- **Cross-platform Testing**: Windows, macOS, Linux

## 🐛 Bug Reports

When reporting bugs, please include:

1. **Environment**
   - Operating System and version
   - Node.js version
   - Rust version
   - App version

2. **Steps to Reproduce**
   - Clear, numbered steps
   - Expected vs actual behavior

3. **Additional Context**
   - Screenshots or videos
   - Console logs
   - System specifications

## 💡 Feature Requests

When suggesting features:

1. **Describe the feature**
   - What it does
   - Why it's useful
   - How it fits with existing features

2. **Provide context**
   - Use cases
   - User stories
   - Mockups or examples

3. **Consider implementation**
   - Technical feasibility
   - Potential challenges
   - Alternative approaches

## 🔧 Development Tips

### Debugging

```bash
# Enable debug logging
RUST_LOG=debug npm run tauri:dev

# Check Rust logs
tail -f ~/.local/share/runmymodel-desktop/logs/app.log
```

### Performance

- Use React DevTools Profiler
- Monitor memory usage
- Test with large models
- Optimize bundle size

### Cross-platform Testing

- Test on different OS versions
- Verify icon rendering
- Check file path handling
- Test Ollama binary compatibility

## 📚 Resources

### Documentation
- [Tauri Documentation](https://tauri.app/v1/guides/)
- [React Documentation](https://react.dev/)
- [Tailwind CSS](https://tailwindcss.com/docs)
- [Ollama Documentation](https://ollama.ai/docs)

### Tools
- [Rust Book](https://doc.rust-lang.org/book/)
- [TypeScript Handbook](https://www.typescriptlang.org/docs/)
- [Zustand Documentation](https://github.com/pmndrs/zustand)


---

Thank you for contributing to RunMyModel Desktop! 🎉

good luck... 
