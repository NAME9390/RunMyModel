import React, { useState, useEffect } from 'react'
import { Search, Download, Trash2, CheckCircle, AlertCircle, HardDrive, Cpu, Zap, ExternalLink } from 'lucide-react'
import { backendService, HuggingFaceModelInfo } from '../services/backendService'
import { useModelStore } from '../store/modelStore'

interface ModelBrowserProps {
  onClose: () => void
}

export const ModelBrowser: React.FC<ModelBrowserProps> = ({ onClose }) => {
  const [allModels, setAllModels] = useState<HuggingFaceModelInfo[]>([])
  const [filteredModels, setFilteredModels] = useState<HuggingFaceModelInfo[]>([])
  const [searchQuery, setSearchQuery] = useState('')
  const [selectedTaskType, setSelectedTaskType] = useState<string>('all')
  const [isLoading, setIsLoading] = useState(true)
  const [installingModels, setInstallingModels] = useState<Set<string>>(new Set())
  
  const { installedModels, downloadModel, removeModel, refreshInstalledModels } = useModelStore()

  useEffect(() => {
    loadModels()
  }, [])

  useEffect(() => {
    filterModels()
  }, [allModels, searchQuery, selectedTaskType])

  const loadModels = async () => {
    setIsLoading(true)
    try {
      const models = await backendService.getAllAvailableModels()
      setAllModels(models)
      await refreshInstalledModels()
    } catch (error) {
      console.error('Error loading models:', error)
    } finally {
      setIsLoading(false)
    }
  }

  const filterModels = () => {
    let filtered = allModels

    // Filter by search query
    if (searchQuery) {
      filtered = filtered.filter(model =>
        model.name.toLowerCase().includes(searchQuery.toLowerCase()) ||
        model.task_type?.toLowerCase().includes(searchQuery.toLowerCase()) ||
        model.size?.toLowerCase().includes(searchQuery.toLowerCase())
      )
    }

    // Filter by task type
    if (selectedTaskType !== 'all') {
      filtered = filtered.filter(model => model.task_type === selectedTaskType)
    }

    setFilteredModels(filtered)
  }

  const getUniqueTaskTypes = () => {
    const taskTypes = new Set(allModels.map(model => model.task_type).filter(Boolean))
    return Array.from(taskTypes).sort()
  }

  const isModelInstalled = (modelName: string) => {
    return installedModels.includes(modelName)
  }

  const handleInstallModel = async (modelName: string) => {
    setInstallingModels(prev => new Set(prev).add(modelName))
    try {
      await downloadModel(modelName)
      await refreshInstalledModels()
    } catch (error) {
      console.error('Error installing model:', error)
    } finally {
      setInstallingModels(prev => {
        const newSet = new Set(prev)
        newSet.delete(modelName)
        return newSet
      })
    }
  }

  const handleRemoveModel = async (modelName: string) => {
    try {
      await removeModel(modelName)
      await refreshInstalledModels()
    } catch (error) {
      console.error('Error removing model:', error)
    }
  }

  const getModelIcon = (modelName: string) => {
    const nameLower = modelName.toLowerCase()
    
    if (nameLower.includes('llama')) {
      if (nameLower.includes('code')) return 'CODE'
      return 'LLAMA'
    }
    
    if (nameLower.includes('qwen')) return 'QWEN'
    if (nameLower.includes('mistral')) return 'MISTRAL'
    if (nameLower.includes('gemma')) return 'GEMMA'
    if (nameLower.includes('phi')) return 'PHI'
    if (nameLower.includes('deepseek')) return 'DEEPSEEK'
    if (nameLower.includes('falcon')) return 'FALCON'
    if (nameLower.includes('granite')) return 'GRANITE'
    if (nameLower.includes('mixtral')) return 'MIXTRAL'
    if (nameLower.includes('starcoder')) return 'STAR'
    if (nameLower.includes('codellama')) return 'CODE'
    if (nameLower.includes('dialogpt')) return 'DIALOG'
    if (nameLower.includes('gpt-neo')) return 'GPT'
    if (nameLower.includes('llava')) return 'VISION'
    if (nameLower.includes('bert')) return 'BERT'
    if (nameLower.includes('distilbert')) return 'DISTIL'
    
    return 'AI'
  }

  const getModelDisplayName = (modelName: string) => {
    const parts = modelName.split('/')
    if (parts.length > 1) {
      return parts[1].replace(/-/g, ' ').replace(/_/g, ' ')
    }
    return modelName
  }

  const getModelAuthor = (modelName: string) => {
    const parts = modelName.split('/')
    return parts[0] || 'Unknown'
  }

  const getSizeColor = (size?: string) => {
    if (!size) return 'text-gray-500'
    const sizeValue = parseFloat(size)
    if (sizeValue < 2) return 'text-green-600' // < 2B
    if (sizeValue < 8) return 'text-yellow-600' // < 8B
    return 'text-red-600' // >= 8B
  }

  if (isLoading) {
    return (
      <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
        <div className="bg-white dark:bg-gray-800 rounded-lg p-8 max-w-md w-full mx-4">
          <div className="flex items-center justify-center">
            <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-blue-600"></div>
            <span className="ml-3 text-lg">Loading models...</span>
          </div>
        </div>
      </div>
    )
  }

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
      <div className="bg-white dark:bg-gray-800 rounded-lg max-w-6xl w-full max-h-[90vh] flex flex-col">
        {/* Header */}
        <div className="flex items-center justify-between p-6 border-b border-gray-200 dark:border-gray-700">
          <div>
            <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Hugging Face Model Browser</h2>
            <p className="text-gray-600 dark:text-gray-400 mt-1">
              Browse and install {allModels.length} available models
            </p>
          </div>
          <button
            onClick={onClose}
            className="text-gray-400 hover:text-gray-600 dark:hover:text-gray-300"
          >
            <svg className="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M6 18L18 6M6 6l12 12" />
            </svg>
          </button>
        </div>

        {/* Filters */}
        <div className="p-6 border-b border-gray-200 dark:border-gray-700">
          <div className="flex flex-col sm:flex-row gap-4">
            {/* Search */}
            <div className="flex-1">
              <div className="relative">
                <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400 w-4 h-4" />
                <input
                  type="text"
                  placeholder="Search Hugging Face models..."
                  value={searchQuery}
                  onChange={(e) => setSearchQuery(e.target.value)}
                  className="w-full pl-10 pr-4 py-2 border border-gray-300 dark:border-gray-600 rounded-lg bg-white dark:bg-gray-700 text-gray-900 dark:text-white focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                />
              </div>
            </div>

            {/* Task Type Filter */}
            <div className="sm:w-48">
              <select
                value={selectedTaskType}
                onChange={(e) => setSelectedTaskType(e.target.value)}
                className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-lg bg-white dark:bg-gray-700 text-gray-900 dark:text-white focus:ring-2 focus:ring-blue-500 focus:border-transparent"
              >
                <option value="all">All Task Types</option>
                {getUniqueTaskTypes().map(taskType => (
                  <option key={taskType} value={taskType}>
                    {taskType}
                  </option>
                ))}
              </select>
            </div>
          </div>
        </div>

        {/* Models Grid */}
        <div className="flex-1 overflow-y-auto p-6">
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {filteredModels.map((model) => {
              const isInstalled = isModelInstalled(model.name)
              const isInstalling = installingModels.has(model.name)
              const displayName = getModelDisplayName(model.name)
              const author = getModelAuthor(model.name)
              
              return (
                <div
                  key={model.name}
                  className="bg-gray-50 dark:bg-gray-700 rounded-lg p-4 border border-gray-200 dark:border-gray-600 hover:shadow-md transition-shadow"
                >
                  {/* Model Header */}
                  <div className="flex items-start justify-between mb-3">
                    <div className="flex items-center">
                      <div className="w-8 h-8 bg-blue-100 dark:bg-blue-900 rounded-lg flex items-center justify-center mr-3">
                        <span className="text-xs font-bold text-blue-700 dark:text-blue-300">
                          {getModelIcon(model.name)}
                        </span>
                      </div>
                      <div>
                        <h3 className="font-semibold text-gray-900 dark:text-white text-sm">
                          {displayName}
                        </h3>
                        <p className="text-xs text-gray-600 dark:text-gray-400">
                          by {author}
                        </p>
                      </div>
                    </div>
                    {isInstalled && (
                      <CheckCircle className="w-5 h-5 text-green-500" />
                    )}
                  </div>

                  {/* Model Details */}
                  <div className="space-y-2 mb-4">
                    <div className="flex items-center text-xs text-gray-600 dark:text-gray-400">
                      <Cpu className="w-3 h-3 mr-1" />
                      {model.size || 'Unknown'} parameters
                    </div>
                    <div className="flex items-center text-xs text-gray-600 dark:text-gray-400">
                      <HardDrive className="w-3 h-3 mr-1" />
                      <span className={getSizeColor(model.size)}>
                        {model.size ? backendService.formatModelSize(model.size) : 'Unknown size'}
                      </span>
                    </div>
                    <div className="flex items-center text-xs text-gray-600 dark:text-gray-400">
                      <Zap className="w-3 h-3 mr-1" />
                      {model.task_type || 'Unknown task'}
                    </div>
                    {model.rating && (
                      <div className="flex items-center text-xs text-gray-600 dark:text-gray-400">
                        <span className="mr-1">⭐</span>
                        Rating: {model.rating}/5
                      </div>
                    )}
                  </div>

                  {/* Action Button */}
                  <div className="flex gap-2">
                    {isInstalled ? (
                      <button
                        onClick={() => handleRemoveModel(model.name)}
                        className="flex-1 flex items-center justify-center px-3 py-2 bg-red-100 dark:bg-red-900 text-red-700 dark:text-red-300 rounded-lg hover:bg-red-200 dark:hover:bg-red-800 transition-colors text-sm"
                      >
                        <Trash2 className="w-4 h-4 mr-1" />
                        Remove
                      </button>
                    ) : (
                      <button
                        onClick={() => handleInstallModel(model.name)}
                        disabled={isInstalling}
                        className="flex-1 flex items-center justify-center px-3 py-2 bg-blue-100 dark:bg-blue-900 text-blue-700 dark:text-blue-300 rounded-lg hover:bg-blue-200 dark:hover:bg-blue-800 transition-colors text-sm disabled:opacity-50 disabled:cursor-not-allowed"
                      >
                        {isInstalling ? (
                          <>
                            <div className="animate-spin rounded-full h-4 w-4 border-b-2 border-blue-600 mr-1"></div>
                            Installing...
                          </>
                        ) : (
                          <>
                            <Download className="w-4 h-4 mr-1" />
                            Install
                          </>
                        )}
                      </button>
                    )}
                    {model.url && (
                      <button
                        onClick={() => window.open(model.url, '_blank')}
                        className="px-3 py-2 bg-purple-100 dark:bg-purple-900 text-purple-700 dark:text-purple-300 rounded-lg hover:bg-purple-200 dark:hover:bg-purple-800 transition-colors text-sm"
                      >
                        <ExternalLink className="w-4 h-4" />
                      </button>
                    )}
                  </div>
                </div>
              )
            })}
          </div>

          {filteredModels.length === 0 && (
            <div className="text-center py-12">
              <AlertCircle className="w-12 h-12 text-gray-400 mx-auto mb-4" />
              <h3 className="text-lg font-medium text-gray-900 dark:text-white mb-2">
                No models found
              </h3>
              <p className="text-gray-600 dark:text-gray-400">
                Try adjusting your search or filter criteria
              </p>
            </div>
          )}
        </div>

        {/* Footer */}
        <div className="p-6 border-t border-gray-200 dark:border-gray-700 bg-gray-50 dark:bg-gray-700">
          <div className="flex items-center justify-between">
            <div className="text-sm text-gray-600 dark:text-gray-400">
              Showing {filteredModels.length} of {allModels.length} models
            </div>
            <button
              onClick={onClose}
              className="px-4 py-2 bg-gray-200 dark:bg-gray-600 text-gray-800 dark:text-gray-200 rounded-lg hover:bg-gray-300 dark:hover:bg-gray-500 transition-colors"
            >
              Close
            </button>
          </div>
        </div>
      </div>
    </div>
  )
}