# Contributing to RunMyModel Desktop

Thank you for your interest in contributing to RunMyModel Desktop! This document provides guidelines and information for contributors.

## 📋 **Table of Contents**

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Contributing Process](#contributing-process)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Documentation](#documentation)
- [Pull Request Process](#pull-request-process)
- [Issue Reporting](#issue-reporting)
- [Release Process](#release-process)

## 🤝 **Code of Conduct**

### Our Pledge

We are committed to providing a welcoming and inspiring community for all. Please read and follow our code of conduct:

- **Be respectful** - Treat everyone with respect and kindness
- **Be inclusive** - Welcome newcomers and diverse perspectives
- **Be constructive** - Provide helpful feedback and suggestions
- **Be patient** - Remember that everyone is learning and growing

### Reporting Issues

If you experience or witness unacceptable behavior, please report it by:
- Creating a private issue with the "conduct" label
- Contacting the maintainers directly
- Using GitHub's reporting tools

## 🚀 **Getting Started**

### Prerequisites

Before contributing, ensure you have:

- **Git** - Version control
- **C++17 Compiler** - GCC 7+ or Clang 5+
- **Qt6** - Widgets, Core, Gui, Concurrent modules
- **CMake** - Version 3.10 or higher
- **Python 3.6+** - For build scripts (optional)

### Development Environment

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install qt6-base-dev qt6-tools-dev qt6-tools-dev-tools
sudo apt install python3 python3-pip
```

#### Linux (Arch/CachyOS)
```bash
sudo pacman -S base-devel cmake git
sudo pacman -S qt6-base qt6-tools qt6-tools-dev
sudo pacman -S python python-pip
```

#### Windows
- Install Visual Studio 2019+ with C++ support
- Install Qt6 from [qt.io](https://www.qt.io/download)
- Install Git for Windows
- Install CMake

## 🛠️ **Development Setup**

### 1. Fork and Clone

```bash
# Fork the repository on GitHub, then:
git clone https://github.com/YOUR_USERNAME/RunMyModel.git
cd RunMyModel
git remote add upstream https://github.com/NAME9390/RunMyModel.git
```

### 2. Build the Project

```bash
# Quick setup and build
./run.sh

# Or manual build
cd app
./build.sh
```

### 3. Verify Installation

```bash
# Test the application
./build/RunMyModelDesktop

# Run tests (if available)
make test
```

## 🔄 **Contributing Process**

### 1. Choose an Issue

- Browse [open issues](https://github.com/NAME9390/RunMyModel/issues)
- Look for issues labeled `good first issue` or `help wanted`
- Comment on the issue to express interest
- Wait for maintainer approval before starting

### 2. Create a Branch

```bash
# Create a feature branch
git checkout -b feature/your-feature-name

# Or bugfix branch
git checkout -b bugfix/issue-number-description
```

### 3. Make Changes

- Write clean, well-documented code
- Follow our coding standards
- Add tests for new functionality
- Update documentation as needed

### 4. Test Your Changes

```bash
# Build and test
./run.sh

# Run specific tests
make test

# Manual testing
./build/RunMyModelDesktop
```

### 5. Commit Changes

```bash
# Stage changes
git add .

# Commit with descriptive message
git commit -m "feat: add new feature description

- Detailed description of changes
- Any breaking changes
- Related issues: #123"
```

### 6. Push and Create PR

```bash
# Push to your fork
git push origin feature/your-feature-name

# Create pull request on GitHub
```

## 📝 **Coding Standards**

### C++ Style Guide

We follow modern C++ best practices:

#### Naming Conventions
```cpp
// Classes: PascalCase
class MainWindow { };

// Functions: camelCase
void loadModel() { }

// Variables: camelCase
QString modelPath;

// Constants: UPPER_CASE
const int MAX_TOKENS = 2048;

// Private members: m_ prefix
QString m_modelPath;
```

#### Code Formatting
```cpp
// Use 4 spaces for indentation
if (condition) {
    doSomething();
}

// Braces on same line for functions
void function() {
    // code
}

// Braces on new line for classes
class MyClass
{
public:
    MyClass();
};
```

#### Documentation
```cpp
/**
 * @brief Loads a GGUF model from the specified path
 * @param path The file path to the GGUF model
 * @return true if successful, false otherwise
 * @throws ModelLoadException if model is invalid
 */
bool loadModel(const QString& path);
```

### Qt-Specific Guidelines

#### Signal/Slot Connections
```cpp
// Use new-style connections
connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);

// Avoid old-style connections
connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
```

#### Memory Management
```cpp
// Use smart pointers when possible
std::unique_ptr<QWidget> widget = std::make_unique<QWidget>();

// Use Qt's parent-child system
QWidget* child = new QWidget(parent);
```

#### Threading
```cpp
// Use QtConcurrent for background tasks
QFuture<void> future = QtConcurrent::run([this]() {
    // Background work
});
```

## 🧪 **Testing**

### Unit Tests

```cpp
// Example test structure
#include <QtTest>

class TestLlamaEngine : public QObject
{
    Q_OBJECT

private slots:
    void testModelLoading();
    void testInference();
    void testErrorHandling();
};
```

### Integration Tests

```cpp
// Test complete workflows
void testChatWorkflow() {
    // Setup
    MainWindow window;
    
    // Test
    window.loadModel("test_model.gguf");
    window.sendMessage("Hello");
    
    // Verify
    QVERIFY(window.hasResponse());
}
```

### Manual Testing Checklist

- [ ] Application starts without errors
- [ ] Model loads successfully
- [ ] Chat functionality works
- [ ] Settings are saved/loaded
- [ ] GPU acceleration works (if available)
- [ ] Error handling works correctly
- [ ] UI is responsive
- [ ] Memory usage is reasonable

## 📚 **Documentation**

### Code Documentation

- Use Doxygen-style comments for public APIs
- Document all parameters and return values
- Include usage examples for complex functions
- Update documentation when changing APIs

### User Documentation

- Update README.md for user-facing changes
- Add troubleshooting information
- Include screenshots for UI changes
- Update installation instructions

### Developer Documentation

- Document build requirements
- Explain architecture decisions
- Provide debugging guides
- Include performance considerations

## 🔀 **Pull Request Process**

### Before Submitting

- [ ] Code follows style guidelines
- [ ] Tests pass locally
- [ ] Documentation is updated
- [ ] No merge conflicts
- [ ] Commit messages are clear

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Manual testing completed

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] No new warnings introduced
```

### Review Process

1. **Automated Checks** - CI/CD runs tests and checks
2. **Code Review** - Maintainers review code quality
3. **Testing** - Manual testing by maintainers
4. **Approval** - At least one maintainer approval required
5. **Merge** - Maintainer merges the PR

## 🐛 **Issue Reporting**

### Bug Reports

Use the bug report template:

```markdown
## Bug Description
Clear description of the bug

## Steps to Reproduce
1. Go to '...'
2. Click on '....'
3. See error

## Expected Behavior
What should happen

## Actual Behavior
What actually happens

## Environment
- OS: [e.g., Ubuntu 20.04]
- Qt Version: [e.g., 6.5.0]
- Compiler: [e.g., GCC 9.4.0]

## Additional Context
Screenshots, logs, etc.
```

### Feature Requests

Use the feature request template:

```markdown
## Feature Description
Clear description of the feature

## Use Case
Why is this feature needed?

## Proposed Solution
How should it work?

## Alternatives Considered
Other approaches considered

## Additional Context
Screenshots, mockups, etc.
```

## 🚀 **Release Process**

### Version Numbering

We follow [Semantic Versioning](https://semver.org/):
- **MAJOR**: Breaking changes
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes (backward compatible)

### Release Types

- **ALPHA**: Early development, unstable
- **BETA**: Feature complete, testing phase
- **PRE-RELEASE**: Release candidate
- **STABLE**: Production release

### Release Checklist

- [ ] All tests pass
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] Version numbers updated
- [ ] Release notes written
- [ ] Packages built and tested
- [ ] GitHub release created

## 🏷️ **Labels and Milestones**

### Issue Labels

- `bug` - Something isn't working
- `enhancement` - New feature or request
- `documentation` - Documentation improvements
- `good first issue` - Good for newcomers
- `help wanted` - Extra attention needed
- `priority: high` - High priority
- `priority: low` - Low priority

### Pull Request Labels

- `ready for review` - Ready for maintainer review
- `needs testing` - Requires additional testing
- `breaking change` - Contains breaking changes
- `documentation` - Documentation changes only

## 📞 **Getting Help**

### Communication Channels

- **GitHub Issues** - Bug reports and feature requests
- **GitHub Discussions** - General questions and ideas
- **Pull Requests** - Code review discussions
- **Email** - Contact maintainers directly

### Resources

- [Qt Documentation](https://doc.qt.io/)
- [llama.cpp Documentation](https://github.com/ggerganov/llama.cpp)
- [C++ Best Practices](https://github.com/lefticus/cppbestpractices)
- [Git Workflow](https://www.atlassian.com/git/tutorials/comparing-workflows)

## 🙏 **Recognition**

Contributors will be recognized in:
- CONTRIBUTORS.md file
- Release notes
- Project documentation
- GitHub contributors page

## 📄 **License**

By contributing to RunMyModel Desktop, you agree that your contributions will be licensed under the Mozilla Public License Version 2.0.

---

**Thank you for contributing to RunMyModel Desktop!** 🚀

Your contributions help make local AI accessible to everyone.
