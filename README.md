# RunMyModel 0.3.0 - Self-Learning LLM Platform

**A local, offline-first AI platform with RAG (Retrieval Augmented Generation) capabilities.**

## ✨ Features

- 🤖 **Local LLM Inference** - Run TinyLlama-1.1B locally with llama.cpp
- 🧠 **Knowledge Management** - Ingest `.txt` and `.md` files into a vector database
- 🔍 **RAG System** - Semantic search with FAISS + retrieval-augmented generation
- 💾 **Persistent Memory** - SQLite metadata + FAISS vector storage
- 🎨 **Modern Qt6 GUI** - Tabbed interface (Chat / Knowledge / Models)
- ⚡ **Real-time Streaming** - Token-by-token response generation
- 🔒 **100% Offline** - No cloud dependencies, all processing local

## 🚀 Quick Start

### Requirements

- **Linux** (tested on Arch/CachyOS, Ubuntu, Fedora)
- **Python 3.9+** with pip
- **Qt6** (Widgets, Core, Gui, Network, Concurrent)
- **GCC/G++** with C++17 support
- **curl** for health checks

### Installation & Run

#### Universal (Any Linux)
```bash
# Clone repository
git clone <your-repo-url>
cd runmymodel-desktop

# Run with ONE command
./run.sh
```

#### Arch Linux / CachyOS (Optimized)
```bash
# Clone repository
git clone <your-repo-url>
cd runmymodel-desktop

# Run with Arch-optimized script
./run_arch.sh
```

**What's the difference?**
- `run.sh` - Universal script, works on any Linux distro
- `run_arch.sh` - Arch-optimized with:
  - Native compiler optimizations (`-march=native -O3`)
  - Better linker flags for performance
  - `systemd-run` for process isolation
  - `fuser` for faster port cleanup
  - Arch-specific package checks

**Both scripts automatically:**
1. ✅ Check all requirements
2. ✅ Set up Python virtual environment
3. ✅ Install dependencies (FAISS, sentence-transformers, FastAPI)
4. ✅ Download embedding model (~90MB, first run only)
5. ✅ Build C++ application
6. ✅ Start knowledge service (port 8001)
7. ✅ Launch GUI application
8. ✅ Clean up on exit

## 📖 Usage

### 1. Chat Tab 💬

- Type your message and press Enter or click "Send"
- Toggle "Use RAG" to enable knowledge-enhanced responses
- Real-time streaming of AI responses

### 2. Knowledge Tab 🧠

**Upload Knowledge:**
- Click "📁 Upload File"
- Select `.txt` or `.md` files
- Watch as the system chunks, embeds, and indexes your knowledge

**Manage Knowledge:**
- View all ingested sources
- See statistics (chunks, vectors, sources)
- Delete sources you no longer need

**RAG Workflow:**
1. Upload a document (e.g., `python_tutorial.md`)
2. Enable "Use RAG" in Chat tab
3. Ask a question related to the document
4. The AI retrieves relevant chunks and generates an enhanced response!

### 3. Models Tab ⚙️

- View current model information
- (Future: Switch models, add API keys)

## 🧪 Testing RAG

A test file is included to try the RAG system:

```bash
# 1. Start the application
./run.sh  # or ./run_arch.sh on Arch

# 2. In the GUI:
#    - Go to "Knowledge" tab
#    - Click "Upload File"
#    - Select "test_knowledge.md"
#    - Wait 5 seconds for ingestion

# 3. Test it:
#    - Go to "Chat" tab
#    - Enable "Use RAG"
#    - Ask: "How do I print text in Python?"
#    - Watch the AI use your knowledge! 🎯
```

## 🏗️ Architecture

```
┌─────────────────────────────────────────┐
│         C++ Qt6 GUI Application         │
│  ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │   Chat   │ │Knowledge │ │ Models  │ │
│  └──────────┘ └──────────┘ └─────────┘ │
└────────┬──────────────┬─────────────────┘
         │              │ HTTP (port 8001)
    ┌────▼────┐    ┌────▼────┐
    │ Llama   │    │Knowledge│
    │ Engine  │    │ Client  │
    └────┬────┘    └────┬────┘
         │              │
    ┌────▼────┐    ┌────▼────┐
    │llama.cpp│    │ FastAPI │
    │(TinyLlama)    │ Service │
    └─────────┘    └────┬────┘
                        │
                   ┌────▼────┐
                   │Knowledge│
                   │ Manager │
                   └────┬────┘
                        │
              ┌─────────┴─────────┐
              │                   │
         ┌────▼────┐         ┌────▼────┐
         │  FAISS  │         │ SQLite  │
         │(vectors)│         │(metadata)│
         └─────────┘         └─────────┘
```

## 📁 Project Structure

```
runmymodel-desktop/
├── run.sh                   # ⭐ Universal launcher
├── run_arch.sh              # ⭐ Arch-optimized launcher
├── build.sh                 # Build script
├── src-cpp/                 # C++ source code
│   ├── include/
│   │   ├── mainwindow.h
│   │   ├── llama_engine.h
│   │   └── knowledge_client.h
│   └── src/
│       ├── main.cpp
│       ├── mainwindow.cpp
│       ├── llama_engine.cpp
│       └── knowledge_client.cpp
├── knowledge_service/       # Python FastAPI service
│   ├── main.py              # FastAPI server
│   ├── knowledge_manager.py # FAISS + SQLite manager
│   └── requirements.txt     # Python dependencies
├── models/                  # LLM models (.gguf)
│   └── tinyllama.gguf
├── memory/                  # Knowledge storage
│   ├── faiss_index.bin      # Vector index
│   └── metadata.db          # SQLite database
├── config/                  # Configuration
│   └── default_config.json
├── test_knowledge.md        # ⭐ Test file for RAG
└── lib/
    └── llama.cpp/           # llama.cpp library
```

## 🛠️ Development

### Manual Build

```bash
# Build only
./build.sh

# Start knowledge service manually
cd knowledge_service
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python3 main.py

# Run GUI (in another terminal)
export LD_LIBRARY_PATH="$(pwd)/build/lib:$(pwd)/lib/llama.cpp/build/bin"
./build/RunMyModelDesktop
```

### Rebuild Application

```bash
# Force rebuild
rm -rf build/
./run.sh  # or ./run_arch.sh
```

### Testing Knowledge Service API

```bash
# Health check
curl http://127.0.0.1:8001/health

# Upload knowledge
curl -X POST http://127.0.0.1:8001/knowledge/ingest \
  -H "Content-Type: application/json" \
  -d '{"text": "Python is a programming language", "source": "test.txt"}'

# Query knowledge
curl -X POST http://127.0.0.1:8001/knowledge/query \
  -H "Content-Type: application/json" \
  -d '{"query": "What is Python?", "top_k": 3}'
```

## 🐛 Troubleshooting

**Port 8001 already in use:**
```bash
# Universal
lsof -i:8001
pkill -9 -f "knowledge_service"

# Arch (faster)
fuser -k 8001/tcp
```

**Knowledge service logs:**
```bash
tail -f knowledge_service.log
```

**Build fails:**
```bash
# Check Qt6 installation
pkg-config --modversion Qt6Core  # Universal
pacman -Qi qt6-base              # Arch

# Check GCC
g++ --version

# Rebuild llama.cpp if needed
cd lib/llama.cpp
rm -rf build
mkdir build && cd build
export CMAKE_ROOT=/usr/share/cmake
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**Model not found:**
```bash
# Download TinyLlama (638MB)
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O models/tinyllama.gguf
```

## 📊 Technical Details

- **LLM Engine:** llama.cpp with GGUF models
- **Embedding Model:** all-MiniLM-L6-v2 (384-dim, ~90MB)
- **Vector DB:** FAISS (Facebook AI Similarity Search)
- **Metadata DB:** SQLite
- **Web Framework:** FastAPI (Python)
- **GUI Framework:** Qt6 (C++)
- **Chunking:** tiktoken (512 tokens, 50 overlap)
- **Retrieval:** Cosine similarity (top-5 by default)

## 🔮 Future Plans

- [ ] Multiple model support
- [ ] API key integration (OpenAI, HuggingFace)
- [ ] Session persistence / chat history
- [ ] Fine-tuning capabilities
- [ ] Knowledge visualization (topic clusters)
- [ ] Export/import knowledge base
- [ ] Windows support
- [ ] AppImage packaging

## 📄 License

MPL 2.0 - See LICENSE file

## 🤝 Contributing

See CONTRIBUTING.md for guidelines.

---

**Built with ❤️ for the local AI community**

**Just run `./run.sh` (or `./run_arch.sh` on Arch) and you're ready to go!** 🚀
