# Changelog

All notable changes to RunMyModel Desktop will be documented in this file.

## [0.2.0-ALPHA] - 2025-10-08

### Added
- **Qt6 Desktop Application**: Complete rewrite from web-only to native Qt6/C++ desktop application
- **360+ Models**: Added `llms.txt` with 361 pre-configured models from Hugging Face
  - Text Generation models (Qwen, DeepSeek, Llama, Granite, etc.)
  - Text-to-Speech models (NeuTTS, Kokoro, VibeVoice, etc.)
  - Text-to-Image models (FLUX, Stable Diffusion, Qwen-Image, etc.)
  - Text-to-Video models (Wan, HunyuanVideo, CogVideoX, etc.)
- **C++ Backend**: Full C++ backend with Qt6 components
  - `Backend`: Main backend orchestrator
  - `WebServer`: HTTP API server with CORS support
  - `HuggingFaceClient`: Model discovery and management
  - `ModelManager`: Local model cache management
  - `SystemInfo`: System information gathering
- **Build System**: CMake-based build system for cross-platform compilation
- **Build Scripts**:
  - `proper_build.sh`: Quick build for development
  - `build.sh`: Full build with packaging options
- **Documentation**: Comprehensive BUILD.md with build instructions

### Changed
- **Architecture**: Migrated from browser-based to Qt desktop application
- **Frontend Integration**: React frontend now embedded in Qt WebEngineView
- **Model Loading**: Models now loaded from `llms.txt` instead of hardcoded lists
- **API Communication**: Frontend communicates with C++ backend via HTTP API (localhost:8080)
- **Port Management**: Dynamic port allocation with fallback if 8080 is busy

### Fixed
- **Model Parsing**: Fixed parser to handle both 4-field and 5-field CSV formats in `llms.txt`
- **Task Type Normalization**: Proper categorization of models by task type
- **Build Process**: Streamlined build with proper dependency management
- **Resource Management**: Automatic copying of frontend assets and `llms.txt` to build directory

### Removed
- Redundant build scripts (12 scripts consolidated into 2)
- Unused Qt resource files (`.qrc`)
- Temporary build directories and artifacts
- Old browser launcher scripts

### Technical Details
- **C++ Standard**: C++17
- **Qt Version**: Qt6 (Core, Widgets, WebEngineWidgets, WebChannel, Network)
- **CMake Version**: 3.16+
- **Frontend Build**: Vite 4.5.14
- **React Version**: 18.x
- **TypeScript**: 5.x

### Known Limitations (ALPHA)
- Model downloads are simulated (not yet implemented)
- Model inference engine not yet integrated
- No actual chat functionality with models yet
- Windows builds require cross-compilation setup
- AppImage and installer packaging not yet automated

## [0.1.0] - Previous Version
- Initial web-based version
- Basic UI components
- Placeholder backend integration

