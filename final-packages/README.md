# RunMyModel Desktop - Final Release

## 🎉 Version 1.0.0 - Production Ready

RunMyModel Desktop is a powerful desktop application that allows you to run large language models locally using Ollama CLI. Chat with models like LLaMA, Mistral, CodeLlama, and more in a beautiful ChatGPT-style interface.

## 📦 Available Packages

### Linux (x86_64)
- **`runmymodel-desktop-linux`** - Direct executable (14MB)
- **`runmymodel-desktop-linux.tar.gz`** - Compressed archive (4.3MB)
- **`RunMyModel Desktop_1.0.0_amd64.deb`** - Debian package (4.4MB)

## 🚀 Installation

### Linux
1. **Direct executable** (Recommended)
   ```bash
   chmod +x runmymodel-desktop-linux
   ./runmymodel-desktop-linux
   ```

2. **Debian package**
   ```bash
   sudo dpkg -i "RunMyModel Desktop_1.0.0_amd64.deb"
   ```

3. **Archive**
   ```bash
   tar -xzf runmymodel-desktop-linux.tar.gz
   chmod +x runmymodel-desktop-linux
   ./runmymodel-desktop-linux
   ```

## 📋 Requirements

- **Ollama CLI** installed and running
- **Linux x86_64** system
- **GTK3** libraries
- **WebKit2GTK** (usually included with GTK3)

## ✨ Features

### 🤖 Model Support
- **145+ supported models** including:
  - LLaMA 3.2 (1B, 3B, 8B, 70B)
  - Gemma 3 (1B, 4B, 12B, 27B)
  - Qwen 2.5 Coder (0.5B to 32B)
  - DeepSeek R1 (7B, 671B)
  - CodeLlama (7B, 13B)
  - Mistral (7B)
  - Phi 3/4 (3.8B, 14B)
  - And many more!

### 🎨 User Interface
- **Clean, professional design** - No emojis, text-based labels
- **Dark/light theme** support
- **Responsive layout** - Works on different screen sizes
- **ChatGPT-style interface** - Familiar and intuitive

### 🔧 Model Management
- **Dynamic model loading** from `models.txt`
- **Model browser** with search and filtering
- **One-click installation** of models
- **Real-time progress** tracking
- **Model removal** functionality

### 💬 Chat Features
- **Real-time chat** with LLMs
- **Message history** persistence
- **System prompts** support
- **Temperature control**
- **Token usage** tracking

## 🛠️ Model Management

### Adding Models
Edit the `models.txt` file to add new models:
```
# Add model names here, one per line
llama3.2:3b-instruct-q4_K_M
mistral:7b-instruct-q4_K_M
your-custom-model:tag
```

### Model Detection
The system automatically detects:
- **Parameter sizes**: 22M, 135M, 1B, 3B, 7B, 13B, 70B, etc.
- **Quantization levels**: Q4_K_M, Q4_K_S, Q4_0, Q8_0, FP16
- **Model families**: Based on naming patterns
- **Estimated sizes**: For download planning

### Interactive Management
Use the built-in model browser:
1. Click "Browse Models" in the sidebar
2. Search and filter models
3. Install/remove models with one click
4. View detailed model information

## 🔧 Configuration

### Models File
The `models.txt` file supports:
- **Comments** using `#`
- **Empty lines** (ignored)
- **Model names** in format `family:tag` or just `family`
- **Automatic parsing** of parameter sizes and families

### Example models.txt:
```
# Popular models
llama3.2:3b-instruct-q4_K_M
llama3.2:8b-instruct-q4_K_M
mistral:7b-instruct-q4_K_M
codellama:7b-instruct-q4_K_M

# Custom models
my-model:latest
another-model:1.0
```

## 🐛 Troubleshooting

### Common Issues

1. **"Ollama not found"**
   - Install Ollama CLI: `curl -fsSL https://ollama.ai/install.sh | sh`
   - Ensure Ollama is running: `ollama serve`

2. **"Model not installed"**
   - Use the model browser to install models
   - Or run: `ollama pull model-name`

3. **"Permission denied"**
   - Make executable: `chmod +x runmymodel-desktop-linux`

4. **"GTK libraries missing"**
   - Install: `sudo apt install libgtk-3-0 libwebkit2gtk-4.0-37`

### Debug Mode
Run with debug output:
```bash
RUST_LOG=debug ./runmymodel-desktop-linux
```

## 📊 Performance

- **Startup time**: ~2-3 seconds
- **Memory usage**: ~50-100MB base
- **Model loading**: Depends on model size
- **Chat response**: Real-time streaming

## 🔒 Security

- **Local execution** - No data sent to external servers
- **Ollama integration** - Uses your local Ollama installation
- **No telemetry** - Completely private
- **Open source** - Code is transparent and auditable

## 🌐 Support

- **Website**: https://runmymodel.org
- **Documentation**: See README files
- **Issues**: Report on GitHub
- **Community**: Join discussions

## 📄 License

MIT License - See LICENSE file for details.

## 🙏 Acknowledgments

- **Ollama** - For the amazing LLM runtime
- **Tauri** - For the desktop framework
- **React** - For the UI framework
- **All contributors** - For making this possible

---

**RunMyModel Desktop v1.0.0** - Run LLMs locally with style! 🚀
