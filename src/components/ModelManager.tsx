import React, { useState, useEffect } from 'react'
import { Download, Search, X, CheckCircle, AlertCircle, Clock } from 'lucide-react'
import { useModelStore } from '../store/modelStore'
import { backendService } from '../services/backendService'

interface ModelManagerProps {
  onClose: () => void
}

export const ModelManager: React.FC<ModelManagerProps> = ({ onClose }) => {
  const [searchQuery, setSearchQuery] = useState('')
  const [searchResults, setSearchResults] = useState<any[]>([])
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

  const handleDownload = async (modelId: string) => {
    try {
      await downloadModel(modelId)
    } catch (error) {
      console.error('Download error:', error)
    }
  }

  const getDownloadStatus = (modelId: string) => {
    return downloadProgress[modelId] || null
  }

  const formatBytes = (bytes: number) => {
    return backendService.formatBytes(bytes)
  }

  const formatNumber = (num: number) => {
    if (num >= 1000000) {
      return (num / 1000000).toFixed(1) + 'M'
    } else if (num >= 1000) {
      return (num / 1000).toFixed(1) + 'K'
    }
    return num.toString()
  }

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
      <div className="bg-white dark:bg-gray-800 rounded-lg shadow-xl w-full max-w-4xl max-h-[80vh] overflow-hidden">
        {/* Header */}
        <div className="flex items-center justify-between p-6 border-b border-gray-200 dark:border-gray-700">
          <h2 className="text-xl font-semibold text-gray-900 dark:text-white">
            AI Models
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
                placeholder="Search models..."
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
                  Available Models
                </h3>
                <div className="space-y-3">
                  {availableModels.map((model) => {
                    const downloadStatus = getDownloadStatus(model.id)
                    return (
                      <div
                        key={model.id}
                        className="flex items-center justify-between p-4 border border-gray-200 dark:border-gray-700 rounded-lg hover:bg-gray-50 dark:hover:bg-gray-700"
                      >
                        <div className="flex-1">
                          <div className="flex items-center space-x-3">
                            <h4 className="font-medium text-gray-900 dark:text-white">
                              {model.name}
                            </h4>
                            <span className="text-sm text-gray-500 dark:text-gray-400">
                              by {model.author}
                            </span>
                          </div>
                          <p className="text-sm text-gray-600 dark:text-gray-400 mt-1">
                            {model.description}
                          </p>
                          <div className="flex items-center space-x-4 mt-2 text-xs text-gray-500 dark:text-gray-400">
                            <span>{formatBytes(model.size)}</span>
                            <span>{formatNumber(model.downloads)} downloads</span>
                            <span>{formatNumber(model.likes)} likes</span>
                          </div>
                          <div className="flex flex-wrap gap-1 mt-2">
                            {model.tags.slice(0, 3).map((tag) => (
                              <span
                                key={tag}
                                className="px-2 py-1 bg-blue-100 dark:bg-blue-900 text-blue-800 dark:text-blue-200 text-xs rounded"
                              >
                                {tag}
                              </span>
                            ))}
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
                          ) : (
                            <button
                              onClick={() => handleDownload(model.id)}
                              className="flex items-center space-x-2 px-3 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 text-sm"
                            >
                              <Download className="w-4 h-4" />
                              <span>Download</span>
                            </button>
                          )}
                        </div>
                      </div>
                    )
                  })}
                </div>
              </div>

              {/* Search Results */}
              {searchResults.length > 0 && (
                <div>
                  <h3 className="text-lg font-medium text-gray-900 dark:text-white mb-4">
                    Search Results
                  </h3>
                  <div className="space-y-3">
                    {searchResults.map((model) => {
                      const downloadStatus = getDownloadStatus(model.id)
                      return (
                        <div
                          key={model.id}
                          className="flex items-center justify-between p-4 border border-gray-200 dark:border-gray-700 rounded-lg hover:bg-gray-50 dark:hover:bg-gray-700"
                        >
                          <div className="flex-1">
                            <div className="flex items-center space-x-3">
                              <h4 className="font-medium text-gray-900 dark:text-white">
                                {model.name}
                              </h4>
                              <span className="text-sm text-gray-500 dark:text-gray-400">
                                by {model.author}
                              </span>
                            </div>
                            <p className="text-sm text-gray-600 dark:text-gray-400 mt-1">
                              {model.description}
                            </p>
                            <div className="flex items-center space-x-4 mt-2 text-xs text-gray-500 dark:text-gray-400">
                              <span>{formatBytes(model.size)}</span>
                              <span>{formatNumber(model.downloads)} downloads</span>
                              <span>{formatNumber(model.likes)} likes</span>
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
                            ) : (
                              <button
                                onClick={() => handleDownload(model.id)}
                                className="flex items-center space-x-2 px-3 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 text-sm"
                              >
                                <Download className="w-4 h-4" />
                                <span>Download</span>
                              </button>
                            )}
                          </div>
                        </div>
                      )
                    })}
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