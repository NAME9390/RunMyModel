# Features Documentation

Comprehensive guide to RunMyModel Desktop's powerful features.

## 🤖 Local LLM Inference

### Core Inference Engine

RunMyModel Desktop uses **llama.cpp** for high-performance local inference:

- **CPU Inference**: Optimized for all x86_64 processors
- **GPU Acceleration**: Automatic CUDA support
- **Memory Efficient**: Uses memory mapping and quantization
- **Real-time Streaming**: Token-by-token generation

### Supported Models

| Model | Size | Context | Use Case |
|-------|------|---------|----------|
| **TinyLlama-1.1B** | 638MB | 2048 | General chat, coding |
| **Llama-2-7B** | 3.8GB | 4096 | Advanced reasoning |
| **CodeLlama-7B** | 3.8GB | 4096 | Code generation |
| **Mistral-7B** | 4.1GB | 8192 | Long conversations |

### Model Formats

- **GGUF**: Quantized format for efficiency
- **Quantization**: Q4_K_M (balanced), Q5_K_M (quality), Q8_0 (highest)
- **Memory Mapping**: Load models without full RAM usage

## ⚡ GPU Acceleration

### CUDA Support

**Automatic Detection:**
- Detects NVIDIA GPUs automatically
- Falls back to CPU if GPU unavailable
- Shows GPU status in settings

**Performance Boost:**
- **CPU Only**: ~15-25 tokens/sec
- **GPU (CUDA)**: ~50-80 tokens/sec
- **High-end GPU**: ~100+ tokens/sec

### GPU Configuration

```bash
# Check CUDA availability
nvidia-smi

# Force GPU mode
./RunMyModelDesktop --gpu-only

# Force CPU mode
./RunMyModelDesktop --cpu-only
```

### Memory Management

- **GPU Memory**: Automatically managed
- **CPU Memory**: Configurable limits
- **Swap Usage**: Prevents out-of-memory crashes

## 📊 Model Management

### Loading Models

**Automatic Loading:**
1. Place `.gguf` files in `models/` directory
2. Application detects available models
3. Select model from dropdown menu

**Manual Loading:**
```bash
# Download model
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf

# Load in application
# Go to Models tab → Load Model → Select file
```

### Model Information

**Displayed Info:**
- Model name and size
- Quantization type
- Context length
- Memory usage
- Loading time

**Model Validation:**
- Checks GGUF format validity
- Verifies file integrity
- Tests inference capability

### Switching Models

**Hot-swapping:**
- Switch models without restart
- Preserves conversation history
- Automatic memory cleanup

**Performance Impact:**
- Loading time: 5-30 seconds
- Memory usage varies by model
- GPU memory reallocation

## 🌊 Real-time Streaming

### Token Streaming

**How it works:**
1. User sends message
2. Model generates tokens one by one
3. UI updates in real-time
4. User sees response building

**Benefits:**
- Immediate feedback
- Better user experience
- Can interrupt long responses
- Shows generation progress

### Streaming Configuration

**Speed Control:**
- **Fast**: Show tokens immediately
- **Smooth**: Buffer tokens for readability
- **Custom**: Adjustable delay

**Visual Indicators:**
- Progress bar during generation
- Token count display
- Speed indicator (tokens/sec)
- Estimated time remaining

### Interruption

**Stop Generation:**
- Click "Stop" button
- Press Escape key
- Close application safely

**Partial Responses:**
- Save incomplete responses
- Resume generation later
- Copy partial text

## 🎛️ Advanced Settings

### Generation Parameters

**Temperature (0.1 - 1.0):**
- **0.1**: Very focused, deterministic
- **0.7**: Balanced creativity (default)
- **1.0**: Highly creative, varied

**Max Tokens:**
- **512**: Short responses
- **1024**: Medium responses (default)
- **2048**: Long responses
- **4096**: Very long responses

**Top-p (0.1 - 1.0):**
- Controls response diversity
- Lower = more focused
- Higher = more varied

### Context Management

**Context Length:**
- Model-dependent limits
- Automatic truncation
- Conversation history

**Memory Usage:**
- Configurable limits
- Automatic cleanup
- Swap file usage

### Performance Tuning

**CPU Optimization:**
- Thread count adjustment
- SIMD instruction usage
- Memory allocation strategy

**GPU Optimization:**
- CUDA stream management
- Memory pool optimization
- Tensor core utilization

## 🔧 Configuration Files

### Settings Storage

**Location:**
- Linux: `~/.config/RunMyModel/`
- Windows: `%APPDATA%/RunMyModel/`

**Files:**
- `settings.json`: User preferences
- `models.json`: Model configurations
- `sessions/`: Conversation history

### Custom Configuration

**JSON Format:**
```json
{
  "temperature": 0.7,
  "max_tokens": 1024,
  "gpu_enabled": true,
  "model_path": "models/tinyllama.gguf",
  "auto_save": true
}
```

**Command Line:**
```bash
./RunMyModelDesktop --config custom.json
```

---

**Next**: Learn about [Troubleshooting](../support/) or [Contributing](../developer/)!
