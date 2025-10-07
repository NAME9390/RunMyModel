import React, { useState, useRef } from 'react'
import { 
  Upload, 
  File, 
  Trash2, 
  Play, 
  Pause, 
  Settings, 
  CheckCircle, 
  AlertCircle,
  X,
  FolderOpen,
  HardDrive,
  Cpu,
  MemoryStick,
  Zap
} from 'lucide-react'

interface CustomModel {
  id: string
  name: string
  path: string
  size: number
  type: 'gguf' | 'safetensors' | 'pytorch' | 'onnx' | 'other'
  status: 'uploaded' | 'processing' | 'ready' | 'error'
  config?: {
    contextLength?: number
    parameters?: number
    quantization?: string
    architecture?: string
  }
}

interface CustomModelPageProps {
  onClose: () => void
}

export const CustomModelPage: React.FC<CustomModelPageProps> = ({ onClose }) => {
  const [customModels, setCustomModels] = useState<CustomModel[]>([
    {
      id: '1',
      name: 'Custom LLaMA 7B',
      path: '/home/user/models/llama-7b.gguf',
      size: 4200000000,
      type: 'gguf',
      status: 'ready',
      config: {
        contextLength: 2048,
        parameters: 7000000000,
        quantization: 'Q4_K_M',
        architecture: 'LLaMA'
      }
    },
    {
      id: '2',
      name: 'Fine-tuned Code Model',
      path: '/home/user/models/code-model.safetensors',
      size: 1500000000,
      type: 'safetensors',
      status: 'processing',
      config: {
        contextLength: 4096,
        parameters: 1500000000,
        quantization: 'FP16',
        architecture: 'GPT-2'
      }
    }
  ])
  
  const [isUploading, setIsUploading] = useState(false)
  const [uploadProgress, setUploadProgress] = useState(0)
  const [selectedModel, setSelectedModel] = useState<CustomModel | null>(null)
  const fileInputRef = useRef<HTMLInputElement>(null)

  const handleFileUpload = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const files = event.target.files
    if (!files || files.length === 0) return

    setIsUploading(true)
    setUploadProgress(0)

    for (const file of Array.from(files)) {
      // Simulate upload progress
      for (let i = 0; i <= 100; i += 10) {
        setUploadProgress(i)
        await new Promise(resolve => setTimeout(resolve, 100))
      }

      const newModel: CustomModel = {
        id: crypto.randomUUID(),
        name: file.name.replace(/\.[^/.]+$/, ''),
        path: file.name,
        size: file.size,
        type: getFileType(file.name),
        status: 'uploaded'
      }

      setCustomModels(prev => [...prev, newModel])
    }

    setIsUploading(false)
    setUploadProgress(0)
  }

  const getFileType = (filename: string): CustomModel['type'] => {
    const ext = filename.toLowerCase().split('.').pop()
    switch (ext) {
      case 'gguf':
        return 'gguf'
      case 'safetensors':
        return 'safetensors'
      case 'pt':
      case 'pth':
        return 'pytorch'
      case 'onnx':
        return 'onnx'
      default:
        return 'other'
    }
  }

  const deleteModel = (id: string) => {
    setCustomModels(prev => prev.filter(model => model.id !== id))
  }

  const toggleModelStatus = (id: string) => {
    setCustomModels(prev => prev.map(model => 
      model.id === id 
        ? { ...model, status: model.status === 'ready' ? 'uploaded' : 'ready' }
        : model
    ))
  }

  const formatBytes = (bytes: number): string => {
    if (bytes === 0) return '0 Bytes'
    const k = 1024
    const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB']
    const i = Math.floor(Math.log(bytes) / Math.log(k))
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i]
  }

  const getStatusIcon = (status: CustomModel['status']) => {
    switch (status) {
      case 'ready':
        return <CheckCircle className="w-4 h-4 text-green-600" />
      case 'processing':
        return <div className="w-4 h-4 border-2 border-blue-600 border-t-transparent rounded-full animate-spin" />
      case 'error':
        return <AlertCircle className="w-4 h-4 text-red-600" />
      default:
        return <File className="w-4 h-4 text-gray-400" />
    }
  }

  const getStatusText = (status: CustomModel['status']) => {
    switch (status) {
      case 'ready':
        return 'Ready to use'
      case 'processing':
        return 'Processing...'
      case 'error':
        return 'Error'
      default:
        return 'Uploaded'
    }
  }

  const getTypeColor = (type: CustomModel['type']) => {
    switch (type) {
      case 'gguf':
        return 'bg-green-100 text-green-800 dark:bg-green-900 dark:text-green-200'
      case 'safetensors':
        return 'bg-blue-100 text-blue-800 dark:bg-blue-900 dark:text-blue-200'
      case 'pytorch':
        return 'bg-orange-100 text-orange-800 dark:bg-orange-900 dark:text-orange-200'
      case 'onnx':
        return 'bg-purple-100 text-purple-800 dark:bg-purple-900 dark:text-purple-200'
      default:
        return 'bg-gray-100 text-gray-800 dark:bg-gray-900 dark:text-gray-200'
    }
  }

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
      <div className="bg-white dark:bg-gray-800 rounded-xl shadow-2xl w-full max-w-6xl max-h-[90vh] overflow-hidden flex flex-col">
        {/* Header */}
        <div className="flex items-center justify-between p-6 border-b border-gray-200 dark:border-gray-700">
          <div>
            <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
              Custom Models
            </h2>
            <p className="text-gray-600 dark:text-gray-400 mt-1">
              Upload and manage your own AI models
            </p>
          </div>
          <button
            onClick={onClose}
            className="text-gray-400 hover:text-gray-600 dark:hover:text-gray-300 p-2 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg"
          >
            <X className="w-6 h-6" />
          </button>
        </div>

        <div className="flex-1 overflow-hidden flex">
          {/* Sidebar */}
          <div className="w-80 border-r border-gray-200 dark:border-gray-700 flex flex-col">
            {/* Upload Section */}
            <div className="p-6 border-b border-gray-200 dark:border-gray-700">
              <h3 className="font-semibold text-gray-900 dark:text-white mb-4">
                Upload Models
              </h3>
              
              <div
                onClick={() => fileInputRef.current?.click()}
                className="border-2 border-dashed border-gray-300 dark:border-gray-600 rounded-lg p-6 text-center hover:border-blue-500 hover:bg-blue-50 dark:hover:bg-blue-900/20 cursor-pointer transition-colors"
              >
                <Upload className="w-8 h-8 text-gray-400 mx-auto mb-2" />
                <p className="text-gray-600 dark:text-gray-400 mb-1">
                  Click to upload or drag files here
                </p>
                <p className="text-sm text-gray-500 dark:text-gray-500">
                  Supports GGUF, SafeTensors, PyTorch, ONNX
                </p>
              </div>

              <input
                ref={fileInputRef}
                type="file"
                multiple
                accept=".gguf,.safetensors,.pt,.pth,.onnx"
                onChange={handleFileUpload}
                className="hidden"
              />

              {isUploading && (
                <div className="mt-4">
                  <div className="flex items-center justify-between text-sm text-gray-600 dark:text-gray-400 mb-2">
                    <span>Uploading...</span>
                    <span>{uploadProgress}%</span>
                  </div>
                  <div className="w-full bg-gray-200 dark:bg-gray-700 rounded-full h-2">
                    <div
                      className="bg-blue-600 h-2 rounded-full transition-all duration-300"
                      style={{ width: `${uploadProgress}%` }}
                    />
                  </div>
                </div>
              )}
            </div>

            {/* Model List */}
            <div className="flex-1 overflow-y-auto p-6">
              <h3 className="font-semibold text-gray-900 dark:text-white mb-4">
                Your Models ({customModels.length})
              </h3>
              
              <div className="space-y-3">
                {customModels.map((model) => (
                  <div
                    key={model.id}
                    onClick={() => setSelectedModel(model)}
                    className={`p-3 rounded-lg border cursor-pointer transition-colors ${
                      selectedModel?.id === model.id
                        ? 'border-blue-500 bg-blue-50 dark:bg-blue-900/20'
                        : 'border-gray-200 dark:border-gray-700 hover:bg-gray-50 dark:hover:bg-gray-700'
                    }`}
                  >
                    <div className="flex items-center justify-between mb-2">
                      <h4 className="font-medium text-gray-900 dark:text-white text-sm">
                        {model.name}
                      </h4>
                      {getStatusIcon(model.status)}
                    </div>
                    
                    <div className="flex items-center justify-between text-xs text-gray-500 dark:text-gray-400">
                      <span className={`px-2 py-1 rounded ${getTypeColor(model.type)}`}>
                        {model.type.toUpperCase()}
                      </span>
                      <span>{formatBytes(model.size)}</span>
                    </div>
                    
                    <p className="text-xs text-gray-500 dark:text-gray-400 mt-1">
                      {getStatusText(model.status)}
                    </p>
                  </div>
                ))}
              </div>
            </div>
          </div>

          {/* Main Content */}
          <div className="flex-1 flex flex-col">
            {selectedModel ? (
              <>
                {/* Model Header */}
                <div className="p-6 border-b border-gray-200 dark:border-gray-700">
                  <div className="flex items-center justify-between">
                    <div>
                      <h3 className="text-xl font-semibold text-gray-900 dark:text-white">
                        {selectedModel.name}
                      </h3>
                      <p className="text-gray-600 dark:text-gray-400 mt-1">
                        {selectedModel.path} • {formatBytes(selectedModel.size)}
                      </p>
                    </div>
                    
                    <div className="flex items-center gap-2">
                      <button
                        onClick={() => toggleModelStatus(selectedModel.id)}
                        disabled={selectedModel.status === 'processing'}
                        className={`px-4 py-2 rounded-lg font-medium transition-colors ${
                          selectedModel.status === 'ready'
                            ? 'bg-green-600 text-white hover:bg-green-700'
                            : 'bg-blue-600 text-white hover:bg-blue-700'
                        } disabled:opacity-50 disabled:cursor-not-allowed`}
                      >
                        {selectedModel.status === 'ready' ? (
                          <>
                            <Pause className="w-4 h-4 inline mr-2" />
                            Stop
                          </>
                        ) : (
                          <>
                            <Play className="w-4 h-4 inline mr-2" />
                            Start
                          </>
                        )}
                      </button>
                      
                      <button
                        onClick={() => {}}
                        className="px-4 py-2 border border-gray-300 dark:border-gray-600 rounded-lg text-gray-700 dark:text-gray-300 hover:bg-gray-50 dark:hover:bg-gray-700"
                      >
                        <Settings className="w-4 h-4 inline mr-2" />
                        Configure
                      </button>
                      
                      <button
                        onClick={() => deleteModel(selectedModel.id)}
                        className="px-4 py-2 border border-red-300 dark:border-red-600 rounded-lg text-red-700 dark:text-red-300 hover:bg-red-50 dark:hover:bg-red-900/20"
                      >
                        <Trash2 className="w-4 h-4 inline mr-2" />
                        Delete
                      </button>
                    </div>
                  </div>
                </div>

                {/* Model Details */}
                <div className="flex-1 overflow-y-auto p-6">
                  <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
                    {/* Model Info */}
                    <div className="space-y-6">
                      <div>
                        <h4 className="font-semibold text-gray-900 dark:text-white mb-3">
                          Model Information
                        </h4>
                        <div className="bg-gray-50 dark:bg-gray-700 rounded-lg p-4 space-y-3">
                          <div className="flex items-center justify-between">
                            <span className="text-gray-600 dark:text-gray-400">Type:</span>
                            <span className={`px-2 py-1 rounded text-sm ${getTypeColor(selectedModel.type)}`}>
                              {selectedModel.type.toUpperCase()}
                            </span>
                          </div>
                          <div className="flex items-center justify-between">
                            <span className="text-gray-600 dark:text-gray-400">Size:</span>
                            <span className="text-gray-900 dark:text-white">{formatBytes(selectedModel.size)}</span>
                          </div>
                          <div className="flex items-center justify-between">
                            <span className="text-gray-600 dark:text-gray-400">Status:</span>
                            <div className="flex items-center gap-2">
                              {getStatusIcon(selectedModel.status)}
                              <span className="text-gray-900 dark:text-white">{getStatusText(selectedModel.status)}</span>
                            </div>
                          </div>
                          <div className="flex items-center justify-between">
                            <span className="text-gray-600 dark:text-gray-400">Path:</span>
                            <span className="text-gray-900 dark:text-white font-mono text-sm">{selectedModel.path}</span>
                          </div>
                        </div>
                      </div>

                      {/* Configuration */}
                      {selectedModel.config && (
                        <div>
                          <h4 className="font-semibold text-gray-900 dark:text-white mb-3">
                            Configuration
                          </h4>
                          <div className="bg-gray-50 dark:bg-gray-700 rounded-lg p-4 space-y-3">
                            {selectedModel.config.parameters && (
                              <div className="flex items-center justify-between">
                                <span className="text-gray-600 dark:text-gray-400">Parameters:</span>
                                <span className="text-gray-900 dark:text-white">
                                  {(selectedModel.config.parameters / 1000000000).toFixed(1)}B
                                </span>
                              </div>
                            )}
                            {selectedModel.config.contextLength && (
                              <div className="flex items-center justify-between">
                                <span className="text-gray-600 dark:text-gray-400">Context Length:</span>
                                <span className="text-gray-900 dark:text-white">{selectedModel.config.contextLength.toLocaleString()}</span>
                              </div>
                            )}
                            {selectedModel.config.quantization && (
                              <div className="flex items-center justify-between">
                                <span className="text-gray-600 dark:text-gray-400">Quantization:</span>
                                <span className="text-gray-900 dark:text-white">{selectedModel.config.quantization}</span>
                              </div>
                            )}
                            {selectedModel.config.architecture && (
                              <div className="flex items-center justify-between">
                                <span className="text-gray-600 dark:text-gray-400">Architecture:</span>
                                <span className="text-gray-900 dark:text-white">{selectedModel.config.architecture}</span>
                              </div>
                            )}
                          </div>
                        </div>
                      )}
                    </div>

                    {/* Usage Stats */}
                    <div className="space-y-6">
                      <div>
                        <h4 className="font-semibold text-gray-900 dark:text-white mb-3">
                          Usage Statistics
                        </h4>
                        <div className="bg-gray-50 dark:bg-gray-700 rounded-lg p-4 space-y-3">
                          <div className="flex items-center justify-between">
                            <span className="text-gray-600 dark:text-gray-400">Total Runs:</span>
                            <span className="text-gray-900 dark:text-white">1,247</span>
                          </div>
                          <div className="flex items-center justify-between">
                            <span className="text-gray-600 dark:text-gray-400">Average Response Time:</span>
                            <span className="text-gray-900 dark:text-white">2.3s</span>
                          </div>
                          <div className="flex items-center justify-between">
                            <span className="text-gray-600 dark:text-gray-400">Last Used:</span>
                            <span className="text-gray-900 dark:text-white">2 hours ago</span>
                          </div>
                          <div className="flex items-center justify-between">
                            <span className="text-gray-600 dark:text-gray-400">Success Rate:</span>
                            <span className="text-green-600 dark:text-green-400">98.5%</span>
                          </div>
                        </div>
                      </div>

                      {/* System Requirements */}
                      <div>
                        <h4 className="font-semibold text-gray-900 dark:text-white mb-3">
                          System Requirements
                        </h4>
                        <div className="bg-gray-50 dark:bg-gray-700 rounded-lg p-4 space-y-3">
                          <div className="flex items-center justify-between">
                            <div className="flex items-center gap-2">
                              <MemoryStick className="w-4 h-4 text-gray-500" />
                              <span className="text-gray-600 dark:text-gray-400">RAM:</span>
                            </div>
                            <span className="text-gray-900 dark:text-white">8GB+</span>
                          </div>
                          <div className="flex items-center justify-between">
                            <div className="flex items-center gap-2">
                              <HardDrive className="w-4 h-4 text-gray-500" />
                              <span className="text-gray-600 dark:text-gray-400">Storage:</span>
                            </div>
                            <span className="text-gray-900 dark:text-white">{formatBytes(selectedModel.size * 1.5)}</span>
                          </div>
                          <div className="flex items-center justify-between">
                            <div className="flex items-center gap-2">
                              <Cpu className="w-4 h-4 text-gray-500" />
                              <span className="text-gray-600 dark:text-gray-400">CPU:</span>
                            </div>
                            <span className="text-gray-900 dark:text-white">4+ cores</span>
                          </div>
                          <div className="flex items-center justify-between">
                            <div className="flex items-center gap-2">
                              <Zap className="w-4 h-4 text-gray-500" />
                              <span className="text-gray-600 dark:text-gray-400">GPU:</span>
                            </div>
                            <span className="text-gray-900 dark:text-white">Optional</span>
                          </div>
                        </div>
                      </div>
                    </div>
                  </div>
                </div>
              </>
            ) : (
              <div className="flex-1 flex items-center justify-center text-gray-500 dark:text-gray-400">
                <div className="text-center">
                  <FolderOpen className="w-16 h-16 mx-auto mb-4" />
                  <h3 className="text-lg font-medium mb-2">No model selected</h3>
                  <p>Choose a model from the sidebar to view details</p>
                </div>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  )
}
