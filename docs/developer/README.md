# Developer Documentation

Welcome to RunMyModel Desktop development! This section covers everything developers need to know.

## 📁 Project Structure

```
RunMyModel/
├── 📁 app/                    # Application source code
│   ├── 📁 src-cpp/           # C++ source files
│   │   ├── 📁 include/       # Header files (.h)
│   │   └── 📁 src/          # Implementation files (.cpp)
│   ├── 📄 build.sh          # Build script
│   └── 📄 run.sh            # Run script
├── 📁 executables/          # Distribution packages
├── 📁 models/              # LLM model files
├── 📁 lib/                 # External libraries
│   └── 📁 llama.cpp/       # llama.cpp library
├── 📁 docs/                # Documentation
└── 📄 run.sh              # Main launcher
```

### Key Files

| File | Purpose |
|------|---------|
| `app/src-cpp/include/mainwindow.h` | Main GUI class |
| `app/src-cpp/src/mainwindow.cpp` | GUI implementation |
| `app/src-cpp/include/llama_engine.h` | LLM inference engine |
| `app/src-cpp/src/llama_engine.cpp` | Engine implementation |
| `app/build.sh` | Build configuration |
| `run.sh` | Main launcher script |

## 🛠️ Building from Source

### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential git wget
```

**Arch Linux:**
```bash
sudo pacman -S qt6-base qt6-tools cmake base-devel git wget
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++ git wget
```

### Build Process

```bash
# Clone repository
git clone https://github.com/NAME9390/RunMyModel.git
cd RunMyModel

# Build llama.cpp library
cd lib/llama.cpp
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ../../

# Build application
cd app
./build.sh
```

### Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `CMAKE_BUILD_TYPE` | Build type (Debug/Release) | Release |
| `GGML_CUDA` | Enable CUDA support | ON |
| `GGML_OPENBLAS` | Enable OpenBLAS | OFF |
| `GGML_METAL` | Enable Metal (macOS) | OFF |

## 🔧 Development Setup

### IDE Configuration

**Qt Creator:**
1. Open `app/src-cpp/` as project
2. Configure kit with Qt6
3. Set build directory to `app/build/`

**VS Code:**
```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/app/src-cpp/include",
                "${workspaceFolder}/lib/llama.cpp/include",
                "/usr/include/qt6"
            ],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c17",
            "cppStandard": "c++17"
        }
    ]
}
```

### Debugging

**Enable debug output:**
```bash
export QT_LOGGING_RULES="*=true"
./RunMyModelDesktop
```

**GDB debugging:**
```bash
gdb ./RunMyModelDesktop
(gdb) run
```

## 📝 Contributing Guide

### Code Style

**C++ Guidelines:**
- Follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- Use Qt naming conventions (`m_variableName`, `onSignalName()`)
- Document public APIs with comments
- Use `clang-format` for formatting

**Qt Conventions:**
```cpp
// Member variables
QString m_modelPath;
bool m_isLoaded;

// Signal handlers
void onModelLoaded();
void onErrorOccurred(const QString &error);

// Private slots
private slots:
    void handleTimeout();
```

### Pull Request Process

1. **Fork** the repository
2. **Create** feature branch: `git checkout -b feature/your-feature`
3. **Make** changes with tests
4. **Commit** with clear messages: `git commit -m "feat: add new feature"`
5. **Push** to your fork: `git push origin feature/your-feature`
6. **Create** pull request

### Commit Message Format

```
type(scope): description

feat(ui): add dark theme toggle
fix(engine): resolve memory leak in model loading
docs(readme): update installation instructions
```

**Types:** `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

## 🧪 Testing

### Unit Tests

```bash
# Run tests (when implemented)
cd app
make test
```

### Manual Testing

**Test Cases:**
1. **Model Loading**: Load different GGUF models
2. **GPU Detection**: Test with/without CUDA
3. **Memory Usage**: Monitor RAM consumption
4. **Error Handling**: Test invalid inputs
5. **UI Responsiveness**: Test during inference

### Performance Testing

```bash
# Benchmark different models
./RunMyModelDesktop --benchmark --model models/tinyllama.gguf
```

## 🐛 Debugging Common Issues

### Build Failures

**Qt6 not found:**
```bash
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6
```

**CUDA not detected:**
```bash
# Check CUDA installation
nvcc --version
# Rebuild with explicit CUDA path
cmake .. -DGGML_CUDA=ON -DCUDA_TOOLKIT_ROOT_DIR=/usr/local/cuda
```

### Runtime Issues

**Model loading fails:**
```bash
# Check model file
file models/tinyllama.gguf
# Verify permissions
ls -la models/
```

**GPU not working:**
```bash
# Check CUDA runtime
nvidia-smi
# Test CUDA availability
./RunMyModelDesktop --test-cuda
```

## 📊 Performance Optimization

### CPU Optimization

```bash
# Enable optimizations
export CFLAGS="-O3 -march=native"
export CXXFLAGS="-O3 -march=native"

# Use OpenBLAS
cmake .. -DGGML_OPENBLAS=ON
```

### Memory Optimization

- Use quantized models (Q4_K_M, Q5_K_M)
- Limit context length
- Enable memory mapping
- Use streaming for large responses

### GPU Optimization

```bash
# CUDA optimization flags
export CUDAFLAGS="-O3 -use_fast_math"
# Enable tensor cores (RTX series)
export GGML_CUDA_TENSOR_CORES=1
```

## 🔄 Release Process

### Version Bumping

1. **Update** version in `app/build.sh`
2. **Update** changelog in `CHANGELOG.md`
3. **Tag** release: `git tag v0.6.0-PRE-RELEASE`
4. **Build** executables
5. **Create** GitHub release

### Distribution

**Linux AppImage:**
```bash
cd executables
./build_appimage.sh
```

**Windows MSI:**
```bash
cd executables
./build_msi.sh
```

---

**Need more help?** Check [Support](../support/) or [GitHub Issues](https://github.com/NAME9390/RunMyModel/issues)!
