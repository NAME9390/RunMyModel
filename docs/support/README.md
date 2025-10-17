# Support & Troubleshooting

Get help with RunMyModel Desktop issues and find solutions to common problems.

## 🐛 Common Issues

### Build & Installation Issues

<details>
<summary><strong>Build fails with "Qt6 not found"</strong></summary>

**Problem**: CMake can't find Qt6 installation

**Solutions:**
```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-tools-dev

# Arch Linux
sudo pacman -S qt6-base qt6-tools

# Set Qt6 path explicitly
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6
```

**Check installation:**
```bash
pkg-config --modversion Qt6Core
```
</details>

<details>
<summary><strong>llama.cpp build fails</strong></summary>

**Problem**: Compilation errors in llama.cpp

**Solutions:**
```bash
# Clean build directory
cd lib/llama.cpp
rm -rf build
mkdir build && cd build

# Rebuild with verbose output
cmake .. -DCMAKE_BUILD_TYPE=Release -DGGML_CUDA=ON
make -j$(nproc) VERBOSE=1
```

**Common fixes:**
- Update CMake: `sudo apt install cmake`
- Install CUDA toolkit for GPU support
- Use older GCC if compatibility issues
</details>

<details>
<summary><strong>Permission denied on AppImage</strong></summary>

**Problem**: Can't execute AppImage file

**Solution:**
```bash
chmod +x RunMyModelDesktop-v0.6.0-PRE-RELEASE-x86_64.AppImage
./RunMyModelDesktop-v0.6.0-PRE-RELEASE-x86_64.AppImage
```
</details>

### Runtime Issues

<details>
<summary><strong>Model not found</strong></summary>

**Problem**: "Model file not found" error

**Solutions:**
```bash
# Download TinyLlama model
mkdir -p models
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf

# Check file exists
ls -la models/
file models/tinyllama.gguf
```

**Verify model:**
- File size should be ~638MB
- Format should be GGUF
- Check file permissions
</details>

<details>
<summary><strong>Out of memory error</strong></summary>

**Problem**: Application crashes with memory errors

**Solutions:**
```bash
# Check available memory
free -h

# Use smaller model
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q2_K.gguf -O models/tinyllama-small.gguf

# Increase swap space
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

**Memory requirements:**
- TinyLlama-1.1B: 2GB RAM minimum
- Llama-2-7B: 8GB RAM minimum
- GPU models: Additional VRAM needed
</details>

<details>
<summary><strong>GPU not detected</strong></summary>

**Problem**: CUDA/GPU acceleration not working

**Solutions:**
```bash
# Check NVIDIA driver
nvidia-smi

# Install CUDA toolkit
sudo apt install nvidia-cuda-toolkit

# Rebuild with CUDA support
cd lib/llama.cpp
rm -rf build && mkdir build && cd build
cmake .. -DGGML_CUDA=ON
make -j$(nproc)

# Test CUDA
./RunMyModelDesktop --test-cuda
```

**Troubleshooting:**
- Verify NVIDIA driver installation
- Check CUDA version compatibility
- Ensure GPU has sufficient VRAM
</details>

<details>
<summary><strong>Slow performance</strong></summary>

**Problem**: Very slow token generation

**Solutions:**
```bash
# Enable GPU acceleration
./RunMyModelDesktop --gpu-only

# Use optimized build
export CFLAGS="-O3 -march=native"
export CXXFLAGS="-O3 -march=native"

# Reduce context length
# In settings: Max Tokens = 512

# Use quantized model
# Download Q4_K_M or Q2_K versions
```

**Performance tips:**
- Close other applications
- Use SSD storage for models
- Enable hardware acceleration
- Use appropriate model size
</details>

### UI Issues

<details>
<summary><strong>Application won't start</strong></summary>

**Problem**: GUI doesn't appear or crashes immediately

**Solutions:**
```bash
# Run from terminal for error messages
./RunMyModelDesktop

# Check Qt6 installation
ldd ./RunMyModelDesktop | grep Qt

# Install missing dependencies
sudo apt install libqt6gui6 libqt6widgets6 libqt6core6

# Run with debug output
QT_LOGGING_RULES="*=true" ./RunMyModelDesktop
```
</details>

<details>
<summary><strong>Dark theme not working</strong></summary>

**Problem**: Interface appears in light theme

**Solutions:**
- Check system theme settings
- Restart application
- Clear configuration: `rm -rf ~/.config/RunMyModel/`
- Force theme: `QT_STYLE_OVERRIDE=dark ./RunMyModelDesktop`
</details>

## 🐛 Bug Reports

### Before Reporting

1. **Check** this troubleshooting guide
2. **Search** [existing issues](https://github.com/NAME9390/RunMyModel/issues)
3. **Update** to latest version
4. **Test** with default settings

### How to Report

**Use our bug report template:**

1. **Go to**: [GitHub Issues](https://github.com/NAME9390/RunMyModel/issues/new?template=bug_report.md)
2. **Fill out** the template completely
3. **Include** system information
4. **Attach** relevant logs

**Required Information:**
- Operating system and version
- RunMyModel Desktop version
- Steps to reproduce
- Expected vs actual behavior
- Error messages or logs
- System specifications

### Log Collection

**Enable debug logging:**
```bash
export QT_LOGGING_RULES="*=true"
./RunMyModelDesktop > debug.log 2>&1
```

**System information:**
```bash
# Collect system info
uname -a
lscpu
free -h
nvidia-smi  # if NVIDIA GPU
```

## 💬 Discussions

### Community Support

**GitHub Discussions**: [Join the conversation](https://github.com/NAME9390/RunMyModel/discussions)

**Discussion Categories:**
- **General**: General questions and chat
- **Q&A**: Technical questions and answers
- **Ideas**: Feature requests and suggestions
- **Show and Tell**: Share your projects

### Getting Help

**Best practices:**
1. **Search** existing discussions first
2. **Be specific** about your problem
3. **Provide** system information
4. **Include** error messages
5. **Be patient** - community is volunteer-based

**Response times:**
- **Bug reports**: 1-3 days
- **Feature requests**: 1-2 weeks
- **General questions**: 1-7 days
- **Discussions**: Varies

## 🗺️ Roadmap

### Current Version: v0.6.0-PRE-RELEASE

**✅ Completed Features:**
- RAG System - Knowledge ingestion and retrieval
- Multiple Models - Support for different LLM models
- Session Management - Save/load conversation sessions
- Plugin System - Extensible architecture
- Model Training - Fine-tuning capabilities

### Upcoming Versions

**v0.7.0 (Next):**
- [ ] **API Integration** - OpenAI, Anthropic compatibility
- [ ] **Model Hub** - Built-in model browser
- [ ] **Advanced RAG** - Vector database integration
- [ ] **Plugin Marketplace** - Community plugins

**v1.0 (Stable):**
- [ ] **Production Ready** - Full stability and testing
- [ ] **Documentation** - Complete user and developer guides
- [ ] **Performance** - Optimized for all platforms
- [ ] **Security** - Security audit and hardening

**v1.1+ (Future):**
- [ ] **Cloud Sync** - Cross-device synchronization
- [ ] **Mobile Support** - Android/iOS applications
- [ ] **Enterprise Features** - Team collaboration
- [ ] **Custom Models** - Training interface

### Feature Requests

**How to request features:**
1. **Check** existing requests first
2. **Use** feature request template
3. **Describe** use case clearly
4. **Provide** mockups if applicable
5. **Vote** on existing requests

**Popular requests:**
- Web interface
- API server mode
- Model comparison
- Batch processing
- Custom themes

---

**Still need help?** 
- 📧 **Create an issue**: [GitHub Issues](https://github.com/NAME9390/RunMyModel/issues/new)
- 💬 **Start a discussion**: [GitHub Discussions](https://github.com/NAME9390/RunMyModel/discussions)
- 📖 **Read documentation**: [Quick Start](../quick-start/) or [Developer Guide](../developer/)
