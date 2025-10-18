# Support Documentation

<div align="center">

![RunMyModel Desktop](https://img.shields.io/badge/RunMyModel-Desktop-blue?style=for-the-badge&logo=qt)

**Support and troubleshooting guide for RunMyModel Desktop**

</div>

---

## 🆘 **Getting Help**

### Support Channels

- **GitHub Issues**: [Report bugs or request features](https://github.com/NAME9390/RunMyModel/issues)
- **GitHub Discussions**: [Community discussions and support](https://github.com/NAME9390/RunMyModel/discussions)
- **Documentation**: [Complete documentation](docs/README.md)
- **FAQ**: [Frequently asked questions](user/faq.md)

### Before Asking for Help

1. **Check Documentation**: Review the relevant documentation
2. **Search Issues**: Look for similar issues on GitHub
3. **Check Troubleshooting**: Review the troubleshooting section below
4. **Gather Information**: Collect relevant system information

## 🐛 **Troubleshooting**

### Installation Issues

#### Build Fails

**Symptoms:**
- Build script fails with errors
- Compilation errors
- Missing dependencies

**Solutions:**
```bash
# Check Qt6 installation
pkg-config --modversion Qt6Core  # Universal
pacman -Qi qt6-base              # Arch
apt list --installed | grep qt6  # Ubuntu

# Check GCC
g++ --version

# Check CMake
cmake --version

# Rebuild llama.cpp
cd lib/llama.cpp
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### Missing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential
```

**Arch Linux:**
```bash
sudo pacman -S qt6-base qt6-tools cmake gcc
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++
```

### Runtime Issues

#### Application Won't Start

**Symptoms:**
- Application crashes on startup
- GUI doesn't appear
- Error messages on launch

**Solutions:**
```bash
# Check if executable exists
ls -la build/RunMyModelDesktop

# Check library path
export LD_LIBRARY_PATH="$(pwd)/lib/llama.cpp/build/bin:$LD_LIBRARY_PATH"

# Run with debug output
./build/RunMyModelDesktop --debug

# Check logs
cat build.log
```

#### Model Loading Issues

**Symptoms:**
- "Model not found" error
- Model loading fails
- Empty model information

**Solutions:**
```bash
# Check model file exists
ls -lh models/tinyllama.gguf

# Download model if missing
mkdir -p models
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf

# Verify model integrity
file models/tinyllama.gguf
```

#### GPU Issues

**Symptoms:**
- GPU not detected
- Slow inference despite GPU
- CUDA errors

**Solutions:**
```bash
# Check NVIDIA GPU
nvidia-smi

# Check CUDA installation
nvcc --version

# On Arch, run optimized script
./run_arch.sh

# On other distros, install CUDA manually
# Ubuntu example:
sudo apt install nvidia-cuda-toolkit

# Rebuild llama.cpp with CUDA
cd lib/llama.cpp
rm -rf build && mkdir build && cd build
cmake .. -DGGML_CUDA=ON
make -j$(nproc)
```

### Performance Issues

#### Slow Inference

**Symptoms:**
- Very slow response generation
- Low tokens per second
- High CPU usage

**Solutions:**
- **Enable GPU**: Use CUDA acceleration if available
- **Reduce Max Tokens**: Lower the maximum token limit
- **Use Quantized Models**: Use Q4_K_M or Q8_0 quantized models
- **Close Other Applications**: Free up system resources
- **Check System Resources**: Monitor CPU and memory usage

#### High Memory Usage

**Symptoms:**
- High RAM usage
- System slowdown
- Out of memory errors

**Solutions:**
- **Use Smaller Models**: Switch to smaller models
- **Reduce Context Size**: Lower the context window
- **Enable Memory Optimization**: Use memory-efficient settings
- **Monitor Memory**: Use system monitoring tools

#### UI Responsiveness

**Symptoms:**
- UI freezes during inference
- Unresponsive interface
- Slow UI updates

**Solutions:**
- **Check Threading**: Ensure proper threading implementation
- **Reduce UI Updates**: Limit UI update frequency
- **Use QtConcurrent**: Proper background processing
- **Monitor Performance**: Check for performance bottlenecks

### Configuration Issues

#### Settings Not Saving

**Symptoms:**
- Settings reset on restart
- Configuration not applied
- Invalid configuration errors

**Solutions:**
```bash
# Check config file permissions
ls -la app/config/default_config.json

# Verify config file format
cat app/config/default_config.json | jq .

# Reset configuration
rm app/config/default_config.json
# Restart application to recreate default config
```

#### Plugin Issues

**Symptoms:**
- Plugins not loading
- Plugin errors
- Plugin conflicts

**Solutions:**
```bash
# Check plugin directory
ls -la plugins/

# Check plugin permissions
ls -la plugins/*.so

# Disable problematic plugins
# Edit config to disable specific plugins
```

## 📋 **Common Questions**

### General Questions

**Q: What models are supported?**
A: RunMyModel Desktop supports GGUF format models, including TinyLlama, Llama-2, CodeLlama, and custom models.

**Q: Can I use my own models?**
A: Yes, you can use any GGUF format model by placing it in the models directory and updating the configuration.

**Q: Is internet required?**
A: No, RunMyModel Desktop runs completely offline. Only the initial model download requires internet.

**Q: What operating systems are supported?**
A: Currently Linux (Arch, Ubuntu, Fedora) with Windows support planned.

### Technical Questions

**Q: How much RAM do I need?**
A: Minimum 4GB RAM, recommended 8GB+ for larger models.

**Q: What GPU is required for acceleration?**
A: NVIDIA GPU with CUDA support. AMD GPU support is planned.

**Q: Can I run multiple models simultaneously?**
A: Not currently, but this feature is planned for future releases.

**Q: How do I update the application?**
A: Pull the latest changes from GitHub and rebuild using `./run.sh`.

### Usage Questions

**Q: How do I save conversations?**
A: Use the "Save Chat" button in the chat interface.

**Q: Can I customize the interface?**
A: Yes, through configuration files and plugins.

**Q: How do I change the model?**
A: Place the new model in the models directory and update the configuration.

**Q: Can I use the application programmatically?**
A: Yes, through the plugin system and API integration.

## 🔧 **Advanced Troubleshooting**

### Debug Mode

Enable debug mode for detailed logging:

```bash
# Run with debug output
./build/RunMyModelDesktop --debug

# Check debug logs
tail -f logs/debug.log
```

### System Information

Collect system information for support:

```bash
# System information
uname -a
cat /etc/os-release

# Qt6 version
pkg-config --modversion Qt6Core

# GCC version
g++ --version

# CUDA version (if applicable)
nvcc --version

# Memory information
free -h

# GPU information (if applicable)
nvidia-smi
```

### Log Analysis

Analyze logs for issues:

```bash
# Application logs
cat logs/app.log

# Build logs
cat build.log

# Error logs
cat logs/error.log

# Debug logs
cat logs/debug.log
```

### Performance Profiling

Profile application performance:

```bash
# CPU profiling
top -p $(pgrep RunMyModelDesktop)

# Memory profiling
ps aux | grep RunMyModelDesktop

# GPU profiling (if applicable)
nvidia-smi -l 1
```

## 🚨 **Emergency Recovery**

### Complete Reset

If the application is completely broken:

```bash
# Remove all build artifacts
rm -rf build/

# Remove llama.cpp
rm -rf lib/llama.cpp/

# Remove configuration
rm -rf app/config/

# Remove sessions
rm -rf sessions/

# Remove logs
rm -rf logs/

# Rebuild everything
./run.sh
```

### Model Recovery

If models are corrupted:

```bash
# Remove corrupted models
rm -rf models/

# Recreate models directory
mkdir -p models

# Download fresh model
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf
```

### Configuration Recovery

If configuration is corrupted:

```bash
# Remove corrupted config
rm -rf app/config/

# Recreate config directory
mkdir -p app/config

# Restart application to recreate default config
./run.sh
```

## 📞 **Contact Support**

### Reporting Issues

When reporting issues, please include:

1. **Clear Title**: Brief description of the issue
2. **Detailed Description**: What happened and what you expected
3. **Steps to Reproduce**: How to reproduce the issue
4. **Environment Details**:
   - OS and version
   - Qt6 version
   - GCC version
   - CUDA version (if applicable)
   - Model being used
5. **Error Messages**: Any error messages or logs
6. **Screenshots**: If applicable

### Feature Requests

When requesting features:

1. **Clear Title**: Brief description of the feature
2. **Detailed Description**: What the feature should do
3. **Use Case**: Why this feature would be useful
4. **Proposed Implementation**: How you think it should work
5. **Additional Context**: Any other relevant information

### Community Support

- **GitHub Discussions**: [Community discussions](https://github.com/NAME9390/RunMyModel/discussions)
- **GitHub Issues**: [Bug reports and feature requests](https://github.com/NAME9390/RunMyModel/issues)
- **Documentation**: [Complete documentation](docs/README.md)

---

<div align="center">

**Built with ❤️ for the local AI community**

[![GitHub](https://img.shields.io/badge/GitHub-NAME9390/RunMyModel-blue?style=flat-square&logo=github)](https://github.com/NAME9390/RunMyModel)
[![License](https://img.shields.io/badge/License-MPL--2.0-orange?style=flat-square)](LICENSE)

*Making AI accessible, one desktop at a time* 🚀

</div>
