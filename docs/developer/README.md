# Developer Documentation

<div align="center">

![RunMyModel Desktop](https://img.shields.io/badge/RunMyModel-Desktop-blue?style=for-the-badge&logo=qt)

**Developer documentation for RunMyModel Desktop**

</div>

---

## 🛠️ **Development Setup**

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

## 🏗️ **Architecture Overview**

### System Components

```
┌─────────────────────────────────────────┐
│         C++ Qt6 GUI Application         │
│  ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │   Chat   │ │ Settings │ │ Models  │ │
│  └──────────┘ └──────────┘ └─────────┘ │
│                                         │
│              MainWindow                 │
│                   │                     │
│              ┌────▼────┐                │
│              │ Llama   │                │
│              │ Engine  │                │
│              └────┬────┘                │
└───────────────────┼─────────────────────┘
                    │
               ┌────▼────┐
               │llama.cpp│
               │ Library │
               └────┬────┘
                    │
        ┌───────────┴───────────┐
        │                       │
   ┌────▼────┐             ┌────▼────┐
   │   CPU   │             │   GPU   │
   │Inference│             │ (CUDA)  │
   └─────────┘             └─────────┘
```

### Core Classes

- **MainWindow**: Main GUI window and application controller
- **LlamaEngine**: LLM inference engine wrapper
- **ModelManager**: Model loading and management
- **SessionManager**: Chat session persistence
- **RAGSystem**: Knowledge ingestion and retrieval
- **PluginSystem**: Extensible plugin architecture

## 🔧 **Build System**

### Build Process

1. **Dependencies Check**: Verify Qt6, GCC, and CMake
2. **llama.cpp Setup**: Clone and build llama.cpp library
3. **Application Build**: Compile C++ application with Qt6
4. **GPU Support**: Enable CUDA if available

### Build Scripts

- **`run.sh`**: Universal build script for any Linux distro
- **`run_arch.sh`**: Arch-optimized build with CUDA auto-install
- **`build.sh`**: Manual build script for development

### Manual Build

```bash
# Build C++ application
./build.sh

# Run manually
export LD_LIBRARY_PATH="$(pwd)/lib/llama.cpp/build/bin:$LD_LIBRARY_PATH"
./build/RunMyModelDesktop
```

## 🧪 **Testing Guide**

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

```bash
# Clean build
rm -rf build/
./build.sh

# Check for compilation warnings
# (Fix any warnings that appear)

# Test on your platform
./build/RunMyModelDesktop
```

## 🐛 **Debugging**

### Debugging Techniques

1. **Qt Debugging**:
   - Use `qDebug()` for logging
   - Enable Qt debug output
   - Use Qt Creator debugger

2. **Memory Debugging**:
   - Use valgrind for memory leak detection
   - Enable AddressSanitizer in debug builds
   - Check for proper resource cleanup

3. **Performance Debugging**:
   - Use Qt profiler
   - Monitor CPU and memory usage
   - Profile llama.cpp inference

### Common Issues

**Build fails:**
```bash
# Check Qt6 installation
pkg-config --modversion Qt6Core

# Check GCC
g++ --version

# Rebuild llama.cpp
cd lib/llama.cpp
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**GPU not being used:**
```bash
# On Arch, run:
./run_arch.sh  # Auto-installs CUDA and rebuilds llama.cpp

# On other distros, install CUDA manually and rebuild llama.cpp with:
cd lib/llama.cpp
rm -rf build && mkdir build && cd build
cmake .. -DGGML_CUDA=ON
make -j$(nproc)
```

## 📝 **Code Style Guidelines**

### C++ Standards

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

## 🔌 **Plugin System**

### Plugin Architecture

The plugin system allows extending RunMyModel Desktop with custom functionality:

- **Plugin Interface**: Standardized plugin API
- **Dynamic Loading**: Load plugins at runtime
- **Configuration**: Plugin-specific settings
- **Lifecycle Management**: Proper plugin initialization and cleanup

### Creating Plugins

1. **Implement Plugin Interface**:
   ```cpp
   class MyPlugin : public PluginInterface {
   public:
       QString getName() const override;
       QString getVersion() const override;
       bool initialize() override;
       void cleanup() override;
   };
   ```

2. **Register Plugin**:
   ```cpp
   REGISTER_PLUGIN(MyPlugin)
   ```

3. **Build as Shared Library**:
   ```cmake
   add_library(myplugin SHARED myplugin.cpp)
   target_link_libraries(myplugin Qt6::Core)
   ```

## 📊 **Performance Optimization**

### Inference Optimization

- **GPU Acceleration**: Use CUDA for faster inference
- **Model Quantization**: Use quantized models for better performance
- **Batch Processing**: Process multiple tokens efficiently
- **Memory Management**: Optimize memory usage during inference

### UI Optimization

- **Threading**: Use QtConcurrent for non-blocking operations
- **Lazy Loading**: Load UI components on demand
- **Caching**: Cache frequently used data
- **Responsive Design**: Keep UI responsive during heavy operations

## 🔒 **Security Considerations**

### Data Privacy

- **Local Processing**: All data stays on the local machine
- **No Network Calls**: No data sent to external servers
- **Session Storage**: Secure local session storage
- **Model Security**: Validate model files before loading

### Code Security

- **Input Validation**: Validate all user inputs
- **Memory Safety**: Use RAII and smart pointers
- **Error Handling**: Proper error handling and recovery
- **Resource Cleanup**: Ensure proper resource cleanup

## 📚 **API Reference**

### Core Classes

#### MainWindow
```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private slots:
    void onSendButtonClicked();
    void onTokenReceived(const QString &token);
    void onSettingsChanged();
    
private:
    void setupUI();
    void setupConnections();
    void loadModel();
    
    LlamaEngine *m_llamaEngine;
    QTextEdit *m_chatDisplay;
    QLineEdit *m_inputLine;
    QPushButton *m_sendButton;
};
```

#### LlamaEngine
```cpp
class LlamaEngine : public QObject {
    Q_OBJECT
    
public:
    explicit LlamaEngine(QObject *parent = nullptr);
    ~LlamaEngine();
    
    bool loadModel(const QString &modelPath);
    void unloadModel();
    void generateResponse(const QString &prompt, int maxTokens = 512);
    
signals:
    void tokenReceived(const QString &token);
    void responseComplete();
    void errorOccurred(const QString &error);
    
private:
    void initializeLlama();
    void cleanupLlama();
    
    llama_model *m_model;
    llama_context *m_context;
    bool m_modelLoaded;
};
```

## 🚀 **Deployment**

### Build Targets

- **Linux**: Native Linux builds
- **Windows**: Cross-compilation support
- **AppImage**: Portable Linux application
- **MSI**: Windows installer package

### Packaging

```bash
# Create AppImage
./create_appimage.sh

# Create Windows installer
./create_msi.sh

# Create distribution packages
./create_packages.sh
```

---

<div align="center">

**Built with ❤️ for the local AI community**

[![GitHub](https://img.shields.io/badge/GitHub-NAME9390/RunMyModel-blue?style=flat-square&logo=github)](https://github.com/NAME9390/RunMyModel)
[![License](https://img.shields.io/badge/License-MPL--2.0-orange?style=flat-square)](LICENSE)

*Making AI accessible, one desktop at a time* 🚀

</div>
