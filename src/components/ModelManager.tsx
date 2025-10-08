import React, { useState, useEffect } from 'react'
import { Download, Search, X, CheckCircle, AlertCircle, Clock, ExternalLink } from 'lucide-react'
import { useModelStore } from '../store/modelStore'
import { backendService, HuggingFaceModelInfo } from '../services/backendService'

interface ModelManagerProps {
  onClose: () => void
}

export const ModelManager: React.FC<ModelManagerProps> = ({ onClose }) => {
  const [searchQuery, setSearchQuery] = useState('')
  const [searchResults, setSearchResults] = useState<HuggingFaceModelInfo[]>([])
  const [isSearching, setIsSearching] = useState(false)
  
  const { 
    availableModels, 
    downloadProgress, 
    refreshModels, 
    downloadModel,
    searchModels,
    isLoading 
  } = useModelStore()

  useEffect(() => {
    refreshModels()
  }, [refreshModels])

  const handleSearch = async () => {
    if (!searchQuery.trim()) return
    
    setIsSearching(true)
    try {
      const results = await searchModels(searchQuery)
      setSearchResults(results)
    } catch (error) {
      console.error('Search error:', error)
    } finally {
      setIsSearching(false)
    }
  }

  const handleDownload = async (modelName: string) => {
    try {
      await downloadModel(modelName)
    } catch (error) {
      console.error('Download error:', error)
    }
  }

  const getDownloadStatus = (modelName: string) => {
    return downloadProgress[modelName] || null
  }

  const formatModelSize = (size?: string) => {
    if (!size) return 'Unknown'
    return backendService.formatModelSize(size)
  }

  const getModelRating = (model: HuggingFaceModelInfo) => {
    return backendService.getModelRating(model)
  }

  const getModelTaskType = (model: HuggingFaceModelInfo) => {
    return backendService.getModelTaskType(model)
  }

  const isModelDownloaded = (model: HuggingFaceModelInfo) => {
    return backendService.isModelDownloaded(model)
  }

  const getModelDisplayName = (modelName: string) => {
    // Extract a more readable name from the Hugging Face model name
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

  const renderModelCard = (model: HuggingFaceModelInfo) => {
    const downloadStatus = getDownloadStatus(model.name)
    const displayName = getModelDisplayName(model.name)
    const author = getModelAuthor(model.name)
    const rating = getModelRating(model)
    const taskType = getModelTaskType(model)
    const downloaded = isModelDownloaded(model)

    return (
      <div
        key={model.name}
        className="flex items-center justify-between p-4 border border-gray-200 dark:border-gray-700 rounded-lg hover:bg-gray-50 dark:hover:bg-gray-700"
      >
        <div className="flex-1">
          <div className="flex items-center space-x-3">
            <h4 className="font-medium text-gray-900 dark:text-white">
              {displayName}
            </h4>
            <span className="text-sm text-gray-500 dark:text-gray-400">
              by {author}
            </span>
            {rating > 0 && (
              <span className="px-2 py-1 bg-yellow-100 dark:bg-yellow-900 text-yellow-800 dark:text-yellow-200 text-xs rounded">
                ⭐ {rating}
              </span>
            )}
          </div>
          <p className="text-sm text-gray-600 dark:text-gray-400 mt-1">
            {taskType}
          </p>
          <div className="flex items-center space-x-4 mt-2 text-xs text-gray-500 dark:text-gray-400">
            <span>{formatModelSize(model.size)}</span>
            <span>{taskType}</span>
            {downloaded && (
              <span className="text-green-600 dark:text-green-400">Downloaded</span>
            )}
          </div>
          <div className="flex items-center space-x-2 mt-2">
            <span className="px-2 py-1 bg-blue-100 dark:bg-blue-900 text-blue-800 dark:text-blue-200 text-xs rounded">
              {model.size || 'Unknown Size'}
            </span>
            <span className="px-2 py-1 bg-green-100 dark:bg-green-900 text-green-800 dark:text-green-200 text-xs rounded">
              {taskType}
            </span>
            {model.url && (
              <a
                href={model.url}
                target="_blank"
                rel="noopener noreferrer"
                className="inline-flex items-center space-x-1 text-blue-600 hover:text-blue-800 dark:text-blue-400 dark:hover:text-blue-300"
              >
                <ExternalLink className="w-3 h-3" />
                <span className="text-xs">View on HF</span>
              </a>
            )}
          </div>
        </div>
        <div className="ml-4">
          {downloadStatus ? (
            <div className="flex items-center space-x-2">
              {downloadStatus.status === 'downloading' && (
                <>
                  <Clock className="w-4 h-4 text-blue-600" />
                  <span className="text-sm text-blue-600">
                    {Math.round(downloadStatus.progress)}%
                  </span>
                </>
              )}
              {downloadStatus.status === 'completed' && (
                <>
                  <CheckCircle className="w-4 h-4 text-green-600" />
                  <span className="text-sm text-green-600">Downloaded</span>
                </>
              )}
              {downloadStatus.status === 'error' && (
                <>
                  <AlertCircle className="w-4 h-4 text-red-600" />
                  <span className="text-sm text-red-600">Error</span>
                </>
              )}
            </div>
          ) : downloaded ? (
            <div className="flex items-center space-x-2">
              <CheckCircle className="w-4 h-4 text-green-600" />
              <span className="text-sm text-green-600">Ready</span>
            </div>
          ) : (
            <button
              onClick={() => handleDownload(model.name)}
              className="flex items-center space-x-2 px-3 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 text-sm"
            >
              <Download className="w-4 h-4" />
              <span>Download</span>
            </button>
          )}
        </div>
      </div>
    )
  }

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
      <div className="bg-white dark:bg-gray-800 rounded-lg shadow-xl w-full max-w-4xl max-h-[80vh] overflow-hidden">
        {/* Header */}
        <div className="flex items-center justify-between p-6 border-b border-gray-200 dark:border-gray-700">
          <h2 className="text-xl font-semibold text-gray-900 dark:text-white">
            Hugging Face Models
          </h2>
          <button
            onClick={onClose}
            className="text-gray-400 hover:text-gray-600 dark:hover:text-gray-300"
          >
            <X className="w-6 h-6" />
          </button>
        </div>

        {/* Search */}
        <div className="p-6 border-b border-gray-200 dark:border-gray-700">
          <div className="flex space-x-2">
            <div className="flex-1 relative">
              <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400 w-4 h-4" />
              <input
                type="text"
                placeholder="Search Hugging Face models..."
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                onKeyPress={(e) => e.key === 'Enter' && handleSearch()}
                className="w-full pl-10 pr-4 py-2 border border-gray-300 dark:border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
              />
            </div>
            <button
              onClick={handleSearch}
              disabled={isSearching}
              className="px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 disabled:opacity-50 disabled:cursor-not-allowed"
            >
              {isSearching ? 'Searching...' : 'Search'}
            </button>
          </div>
        </div>

        {/* Content */}
        <div className="overflow-y-auto max-h-96">
          {isLoading ? (
            <div className="flex items-center justify-center py-12">
              <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-blue-600"></div>
            </div>
          ) : (
            <div className="p-6">
              {/* Available Models */}
              <div className="mb-6">
                <h3 className="text-lg font-medium text-gray-900 dark:text-white mb-4">
                  Available Models ({availableModels.length})
                </h3>
                <div className="space-y-3">
                  {availableModels.map(renderModelCard)}
                </div>
              </div>

              {/* Search Results */}
              {searchResults.length > 0 && (
                <div>
                  <h3 className="text-lg font-medium text-gray-900 dark:text-white mb-4">
                    Search Results ({searchResults.length})
                  </h3>
                  <div className="space-y-3">
                    {searchResults.map(renderModelCard)}
                  </div>
                </div>
              )}
            </div>
          )}
        </div>
      </div>
    </div>
  )
}