# Quick Start Guide

Welcome to RunMyModel Desktop! This guide will get you up and running in minutes.

## 🚀 Installation

### Option 1: Download Pre-built (Recommended)

1. **Download** the latest AppImage from [releases](executables/)
2. **Make executable**: `chmod +x RunMyModelDesktop-v0.6.0-PRE-RELEASE-x86_64.AppImage`
3. **Run**: `./RunMyModelDesktop-v0.6.0-PRE-RELEASE-x86_64.AppImage`

### Option 2: Build from Source

```bash
# Clone repository
git clone https://github.com/NAME9390/RunMyModel.git
cd RunMyModel

# One-command setup
./run.sh
```

## 📥 Download Models

### Automatic Download (Recommended)

RunMyModel will automatically download TinyLlama-1.1B if no model is found.

### Manual Download

```bash
# Create models directory
mkdir -p models

# Download TinyLlama (638MB)
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf
```

### Other Models

You can use any GGUF format model:

- **Llama 2 7B**: [TheBloke/Llama-2-7B-Chat-GGUF](https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF)
- **CodeLlama 7B**: [TheBloke/CodeLlama-7B-Instruct-GGUF](https://huggingface.co/TheBloke/CodeLlama-7B-Instruct-GGUF)
- **Mistral 7B**: [TheBloke/Mistral-7B-Instruct-v0.1-GGUF](https://huggingface.co/TheBloke/Mistral-7B-Instruct-v0.1-GGUF)

## 💬 First Chat

1. **Launch** RunMyModel Desktop
2. **Wait** for model to load (progress bar will show)
3. **Type** your message in the input field
4. **Press Enter** or click "Send"
5. **Watch** the AI respond in real-time!

### Example Prompts

- "Hello! How are you today?"
- "Explain quantum computing in simple terms"
- "Write a Python function to sort a list"
- "What are the benefits of renewable energy?"

## ⚙️ Basic Settings

### Temperature (Creativity)
- **0.1**: Very focused, deterministic
- **0.7**: Balanced (default)
- **1.0**: More creative, varied responses

### Max Tokens
- **512**: Short responses
- **1024**: Medium responses (default)
- **2048**: Long responses

### GPU Acceleration
- **Auto**: Automatically use GPU if available
- **CPU Only**: Force CPU inference
- **GPU Only**: Force GPU inference

## 🎯 Tips for Best Results

1. **Be specific** in your prompts
2. **Use context** - reference previous messages
3. **Adjust temperature** based on your needs
4. **Try different models** for different tasks
5. **Save conversations** for later reference

## 🆘 Need Help?

- **Documentation**: Check other guides in this folder
- **Issues**: [GitHub Issues](https://github.com/NAME9390/RunMyModel/issues)
- **Discussions**: [GitHub Discussions](https://github.com/NAME9390/RunMyModel/discussions)

---

**Next Steps**: Explore [Features](../features/) or [Troubleshooting](../support/) guides!
