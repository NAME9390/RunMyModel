import React, { useState, useEffect } from 'react'
import { Search, Download, Trash2, CheckCircle, AlertCircle, HardDrive, Cpu, Zap } from 'lucide-react'
import { backendService, OllamaModelInfo } from '../services/backendService'
import { useModelStore } from '../store/modelStore'

interface ModelBrowserProps {
  onClose: () => void
}

export const ModelBrowser: React.FC<ModelBrowserProps> = ({ onClose }) => {
  const [allModels, setAllModels] = useState<OllamaModelInfo[]>([])
  const [filteredModels, setFilteredModels] = useState<OllamaModelInfo[]>([])
  const [searchQuery, setSearchQuery] = useState('')
  const [selectedFamily, setSelectedFamily] = useState<string>('all')
  const [isLoading, setIsLoading] = useState(true)
  const [installingModels, setInstallingModels] = useState<Set<string>>(new Set())
  
  const { installedModels, downloadModel, removeModel, refreshInstalledModels } = useModelStore()

  useEffect(() => {
    loadModels()
  }, [])

  useEffect(() => {
    filterModels()
  }, [allModels, searchQuery, selectedFamily])

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
        model.family?.toLowerCase().includes(searchQuery.toLowerCase()) ||
        model.parameter_size?.toLowerCase().includes(searchQuery.toLowerCase())
      )
    }

    // Filter by family
    if (selectedFamily !== 'all') {
      filtered = filtered.filter(model => model.family === selectedFamily)
    }

    setFilteredModels(filtered)
  }

  const getUniqueFamilies = () => {
    const families = new Set(allModels.map(model => model.family).filter(Boolean))
    return Array.from(families).sort()
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

  const getModelIcon = (family: string | undefined) => {
    if (!family) return 'AI'
    
    const familyLower = family.toLowerCase()
    
    // Llama family variations
    if (familyLower.includes('llama')) {
      if (familyLower.includes('code')) return 'CODE'
      if (familyLower.includes('tiny')) return 'TINY'
      if (familyLower.includes('dolphin')) return 'DOLPHIN'
      if (familyLower.includes('vision')) return 'VISION'
      return 'LLAMA'
    }
    
    // Other model families
    switch (familyLower) {
      case 'mistral':
        return 'MISTRAL'
      case 'gemma':
      case 'gemma3':
      case 'gemma3n':
        return 'GEMMA'
      case 'phi3':
      case 'phi4':
        return 'PHI'
      case 'qwen':
      case 'qwen2.5':
      case 'qwen2.5-coder':
      case 'qwen3':
      case 'qwen3-coder':
      case 'qwq':
        return 'QWEN'
      case 'orca-mini':
        return 'ORCA'
      case 'deepseek':
      case 'deepseek-r1':
      case 'deepseek-coder':
      case 'deepseek-coder-v2':
      case 'deepseek-llm':
      case 'deepseek-v2':
        return 'DEEPSEEK'
      case 'falcon':
      case 'falcon3':
        return 'FALCON'
      case 'granite':
      case 'granite3.1-moe':
      case 'granite3.2':
      case 'granite3.2-vision':
      case 'granite3.3':
      case 'granite-code':
        return 'GRANITE'
      case 'mixtral':
      case 'dolphin-mixtral':
        return 'MIXTRAL'
      case 'starcoder':
      case 'starcoder2':
        return 'STAR'
      case 'codegemma':
        return 'GEMMA'
      case 'snowflake-arctic-embed':
        return 'SNOWFLAKE'
      case 'smollm':
      case 'smollm2':
        return 'SMOLLM'
      case 'openthinker':
        return 'THINKER'
      case 'cogito':
        return 'COGITO'
      case 'codestral':
        return 'CODESTRAL'
      case 'wizardlm2':
      case 'wizardcoder':
      case 'wizard-vicuna-uncensored':
        return 'WIZARD'
      case 'magistral':
        return 'MAGISTRAL'
      case 'deepscaler':
        return 'SCALER'
      case 'devstral':
        return 'DEVSTRAL'
      case 'vicuna':
        return 'VICUNA'
      case 'openchat':
        return 'CHAT'
      case 'nous-hermes':
      case 'nous-hermes2':
        return 'HERMES'
      case 'exaone-deep':
        return 'EXAONE'
      case 'mistral-openorca':
        return 'ORCA'
      case 'codegeex4':
        return 'CODEGEEX'
      case 'openhermes':
        return 'HERMES'
      case 'codeqwen':
        return 'QWEN'
      case 'opencoder':
      case 'opencoder-open':
        return 'CODER'
      case 'aya':
        return 'AYA'
      case 'tinydolphin':
        return 'DOLPHIN'
      case 'glm4':
        return 'GLM'
      case 'stable-code':
        return 'STABLE'
      case 'neural-chat':
        return 'NEURAL'
      case 'command-r-plus':
        return 'COMMAND'
      case 'bakllava':
        return 'BAKLLAVA'
      case 'bge-m3':
        return 'BGE'
      case 'all-minilm':
        return 'MINILM'
      case 'xmodel':
        return 'XMODEL'
      case 'bmodel':
        return 'BMODEL'
      default:
        return 'AI'
    }
  }

  const getSizeColor = (size: number | undefined) => {
    if (!size) return 'text-gray-500'
    if (size < 2_000_000_000) return 'text-green-600' // < 2GB
    if (size < 8_000_000_000) return 'text-yellow-600' // < 8GB
    return 'text-red-600' // >= 8GB
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
            <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Model Browser</h2>
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
                  placeholder="Search models..."
                  value={searchQuery}
                  onChange={(e) => setSearchQuery(e.target.value)}
                  className="w-full pl-10 pr-4 py-2 border border-gray-300 dark:border-gray-600 rounded-lg bg-white dark:bg-gray-700 text-gray-900 dark:text-white focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                />
              </div>
            </div>

            {/* Family Filter */}
            <div className="sm:w-48">
              <select
                value={selectedFamily}
                onChange={(e) => setSelectedFamily(e.target.value)}
                className="w-full px-3 py-2 border border-gray-300 dark:border-gray-600 rounded-lg bg-white dark:bg-gray-700 text-gray-900 dark:text-white focus:ring-2 focus:ring-blue-500 focus:border-transparent"
              >
                <option value="all">All Families</option>
                {getUniqueFamilies().map(family => (
                  <option key={family} value={family}>
                    {family}
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
                          {getModelIcon(model.family)}
                        </span>
                      </div>
                      <div>
                        <h3 className="font-semibold text-gray-900 dark:text-white text-sm">
                          {model.name.split(':')[0]}
                        </h3>
                        <p className="text-xs text-gray-600 dark:text-gray-400">
                          {model.name.split(':')[1]}
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
                      {model.parameter_size || 'Unknown'} parameters
                    </div>
                    <div className="flex items-center text-xs text-gray-600 dark:text-gray-400">
                      <HardDrive className="w-3 h-3 mr-1" />
                      <span className={getSizeColor(model.size)}>
                        {model.size ? backendService.formatBytes(model.size) : 'Unknown size'}
                      </span>
                    </div>
                    {model.quantization_level && (
                      <div className="flex items-center text-xs text-gray-600 dark:text-gray-400">
                        <Zap className="w-3 h-3 mr-1" />
                        {model.quantization_level}
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
