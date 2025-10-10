# Contributing to RunMyModel Desktop

Thank you for your interest in contributing to RunMyModel Desktop! This document provides guidelines for contributing to the project.

## 🤝 How to Contribute

### Reporting Issues

Before creating an issue, please:

1. **Search existing issues** to avoid duplicates
2. **Check the documentation** for common solutions
3. **Provide detailed information** about your problem

When creating an issue, include:

- **Description**: Clear description of the problem
- **Steps to Reproduce**: Detailed steps to reproduce the issue
- **Expected Behavior**: What you expected to happen
- **Actual Behavior**: What actually happened
- **Environment**: OS, Qt6 version, GCC version, CUDA version (if applicable)
- **Screenshots**: If applicable, include screenshots

### Suggesting Features

Feature requests are welcome! Please include:

- **Use Case**: Why this feature would be useful
- **Proposed Solution**: How you think it should work
- **Alternatives**: Other solutions you've considered
- **Additional Context**: Any other relevant information

## 🛠️ Development Setup

### Prerequisites

- **Linux**: Arch, Ubuntu, Fedora, or similar
- **Qt6**: Version 6.0 or higher (Widgets, Core, Gui, Concurrent)
- **GCC/G++**: Version 9 or higher with C++17 support
- **CMake**: Version 3.10 or higher (for llama.cpp)
- **Git**: For version control
- **CUDA** (optional): For GPU acceleration

### Getting Started

1. **Fork the repository** on GitHub

2. **Clone your fork**:
   ```bash
   git clone https://github.com/your-username/RunMyModel.git
   cd RunMyModel
   ```

3. **Download the model**:
   ```bash
   mkdir -p models
   wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf
   ```

4. **Build and run**:
   ```bash
   ./run.sh
   ```

### Development Workflow

1. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes**:
   - Write clean, readable code
   - Follow the existing code style
   - Add comments for complex logic
   - Update documentation if needed

3. **Test your changes**:
   ```bash
   # Rebuild and test
   rm -rf build/
   ./run.sh
   
   # Test all features:
   # - Chat functionality
   # - Settings controls
   # - Model loading
   # - Save/clear chat
   ```

4. **Commit your changes**:
   ```bash
   git add .
   git commit -m "feat: brief description of your changes"
   ```

5. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create a Pull Request** on GitHub

## 📝 Code Style Guidelines

### C++

- Use **C++17 standards**
- Follow **Qt6 coding conventions**
- Use **RAII** for resource management
- **Document public methods** with comments
- Use **const correctness** throughout

```cpp
/**
 * Generates a response from the LLM
 * @param prompt The user's input prompt
 * @param maxTokens Maximum number of tokens to generate
 */
void LlamaEngine::generateResponse(const QString &prompt, int maxTokens) {
    // Implementation
}
```

### Qt Guidelines

- Use **signals and slots** for communication
- Implement **QtConcurrent** for threading
- Use **Qt containers** (QString, QVector, etc.)
- Follow **Qt naming conventions** (camelCase for methods, m_ prefix for members)

```cpp
// Good: Qt style
class MainWindow : public QMainWindow {
    Q_OBJECT
    
private:
    LlamaEngine *m_llamaEngine;
    QTextEdit *m_chatDisplay;
    
private slots:
    void onSendButtonClicked();
    void onTokenReceived(const QString &token);
};
```

### Memory Management

- Use **smart pointers** when appropriate
- Follow **Qt parent-child ownership** model
- **Clean up resources** in destructors
- **Avoid memory leaks** with proper RAII

```cpp
// Good: Qt parent manages child widgets
m_chatDisplay = new QTextEdit(this); // 'this' is parent

// Good: Smart pointers for non-Qt objects
std::unique_ptr<ModelConfig> config = std::make_unique<ModelConfig>();
```

## 🧪 Testing

### Manual Testing

Before submitting a PR, please test:

1. **Core Functionality**:
   - Chat message sending
   - Response generation
   - Stop generation button
   - Settings changes (temperature, max tokens)

2. **UI/UX**:
   - All tabs work correctly
   - Buttons respond properly
   - Scrolling works in chat
   - Progress indicators update

3. **Edge Cases**:
   - Empty messages
   - Very long messages
   - Rapid message sending
   - Model loading/unloading

4. **Performance**:
   - No memory leaks (use valgrind if needed)
   - Responsive UI during generation
   - Proper cleanup on exit

### Build Testing

Run the following before submitting:

```bash
# Clean build
rm -rf build/
./build.sh

# Check for compilation warnings
# (Fix any warnings that appear)

# Test on your platform
./build/RunMyModelDesktop
```

## 📋 Pull Request Guidelines

### Before Submitting

- [ ] **Code follows** the project's style guidelines
- [ ] **Code compiles** without errors or warnings
- [ ] **Tested manually** on your platform
- [ ] **Documentation is updated** if necessary
- [ ] **Commits are clear** and well-described
- [ ] **PR description** explains the changes clearly

### PR Description Template

```markdown
## Description
Brief description of the changes

## Type of Change
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update

## Testing
- [ ] Tested on Linux (specify distro)
- [ ] Tested with CPU inference
- [ ] Tested with GPU inference (if applicable)
- [ ] Manual testing completed

## Screenshots (if applicable)
Add screenshots to help explain your changes

## Additional Notes
Any additional information about the changes
```

## 🏷️ Commit Message Format

Use clear, descriptive commit messages:

```
type: brief description

Detailed description of the changes (if needed)

- Bullet point for specific changes
- Another bullet point if needed
```

### Commit Types

- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code style changes (formatting, etc.)
- **refactor**: Code refactoring
- **perf**: Performance improvements
- **test**: Adding or updating tests
- **chore**: Maintenance tasks
- **build**: Build system changes

### Examples

```
feat: add temperature slider to settings tab

fix: resolve memory leak in llama engine cleanup

docs: update README with GPU setup instructions

perf: optimize token streaming with better buffering
```

## 🐛 Bug Reports

When reporting bugs, please include:

1. **Clear title** describing the issue
2. **Detailed description** of the problem
3. **Steps to reproduce** the issue
4. **Expected vs actual behavior**
5. **Environment details**:
   - OS and version
   - Qt6 version (`qmake --version`)
   - GCC version (`g++ --version`)
   - CUDA version (if applicable)
   - Model file being used
6. **Error messages or logs** if applicable
7. **Screenshots** showing the problem

## 💡 Feature Requests

When suggesting features:

1. **Clear title** describing the feature
2. **Detailed description** of the proposed feature
3. **Use case** explaining why it would be useful
4. **Proposed implementation** (if you have ideas)
5. **Additional context** or examples
6. **Mockups** or sketches (if UI-related)

## 📞 Getting Help

If you need help:

1. **Check the README** first for usage instructions
2. **Search existing issues** for similar problems
3. **Ask in discussions** for general questions
4. **Create an issue** for bugs or feature requests

## 🎯 Areas for Contribution

We especially welcome contributions in:

- **Session persistence**: Save/load chat history
- **Model management**: Download and switch between models
- **UI/UX improvements**: Better themes, responsive design
- **Performance**: Optimization of inference or UI
- **Documentation**: Improve guides and examples
- **Testing**: Add automated tests
- **Platform support**: Windows builds, AppImage packaging
- **RAG system**: Knowledge ingestion and retrieval

## 📄 License

By contributing to RunMyModel Desktop, you agree that your contributions will be licensed under the Mozilla Public License 2.0 (MPL-2.0).

## 🙏 Recognition

Contributors will be recognized in:

- **README.md**: Listed as contributors
- **Release notes**: Mentioned in relevant releases
- **GitHub**: Shown in the contributors section

Thank you for contributing to RunMyModel Desktop! 🚀
