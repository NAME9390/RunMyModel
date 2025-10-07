import React, { useState, useEffect } from 'react'
import { 
  Search, 
  Star, 
  Download, 
  CheckCircle, 
  RefreshCw,
  Wifi,
  WifiOff,
  Heart,
  Zap,
  X
} from 'lucide-react'
import { useModelStore } from '../store/modelStore'
import { backendService, ModelInfo } from '../services/backendService'

interface ModelSelectorProps {
  onSelectModel: (model: ModelInfo) => void
  onClose: () => void
  currentModel?: string
}

export const ModelSelector: React.FC<ModelSelectorProps> = ({ 
  onSelectModel, 
  onClose, 
  currentModel 
}) => {
  const [searchQuery, setSearchQuery] = useState('')
  const [selectedCategory, setSelectedCategory] = useState<string>('all')
  const [sortBy, setSortBy] = useState<'downloads' | 'likes' | 'name' | 'recent'>('downloads')
  const [showFavorites, setShowFavorites] = useState(false)
  const [isRefreshing, setIsRefreshing] = useState(false)
  const [favorites, setFavorites] = useState<string[]>([])
  
  const { 
    availableModels, 
    refreshModels, 
    isLoading 
  } = useModelStore()

  const [filteredModels, setFilteredModels] = useState<ModelInfo[]>([])
  const [isOnline, setIsOnline] = useState(navigator.onLine)

  useEffect(() => {
    // Load favorites from localStorage
    const savedFavorites = localStorage.getItem('favorite_models')
    if (savedFavorites) {
      setFavorites(JSON.parse(savedFavorites))
    }

    // Listen for online/offline events
    const handleOnline = () => setIsOnline(true)
    const handleOffline = () => setIsOnline(false)
    
    window.addEventListener('online', handleOnline)
    window.addEventListener('offline', handleOffline)

    return () => {
      window.removeEventListener('online', handleOnline)
      window.removeEventListener('offline', handleOffline)
    }
  }, [])

  useEffect(() => {
    filterModels()
  }, [availableModels, searchQuery, selectedCategory, sortBy, showFavorites, favorites])

  const filterModels = () => {
    let models = [...availableModels]

    // Filter by search query
    if (searchQuery.trim()) {
      models = models.filter(model => 
        model.name.toLowerCase().includes(searchQuery.toLowerCase()) ||
        model.description.toLowerCase().includes(searchQuery.toLowerCase()) ||
        model.author.toLowerCase().includes(searchQuery.toLowerCase()) ||
        model.tags.some(tag => tag.toLowerCase().includes(searchQuery.toLowerCase()))
      )
    }

    // Filter by category
    if (selectedCategory !== 'all') {
      models = models.filter(model => model.category === selectedCategory)
    }

    // Filter by favorites
    if (showFavorites) {
      models = models.filter(model => favorites.includes(model.id))
    }

    // Sort models
    models.sort((a, b) => {
      switch (sortBy) {
        case 'downloads':
          return b.downloads - a.downloads
        case 'likes':
          return b.likes - a.likes
        case 'name':
          return a.name.localeCompare(b.name)
        case 'recent':
          return new Date(b.lastModified).getTime() - new Date(a.lastModified).getTime()
        default:
          return b.downloads - a.downloads
      }
    })

    setFilteredModels(models)
  }

  const handleRefresh = async () => {
    setIsRefreshing(true)
    try {
      await refreshModels()
    } catch (error) {
      console.error('Error refreshing models:', error)
    } finally {
      setIsRefreshing(false)
    }
  }

  const toggleFavorite = (modelId: string) => {
    const newFavorites = favorites.includes(modelId)
      ? favorites.filter(id => id !== modelId)
      : [...favorites, modelId]
    
    setFavorites(newFavorites)
    localStorage.setItem('favorite_models', JSON.stringify(newFavorites))
  }

  const categories = [
    { id: 'all', name: 'All Models', count: availableModels.length },
    { id: 'text-generation', name: 'Text Generation', count: availableModels.filter(m => m.category === 'text-generation').length },
    { id: 'question-answering', name: 'Q&A', count: availableModels.filter(m => m.category === 'question-answering').length },
    { id: 'summarization', name: 'Summarization', count: availableModels.filter(m => m.category === 'summarization').length },
    { id: 'translation', name: 'Translation', count: availableModels.filter(m => m.category === 'translation').length },
    { id: 'sentiment-analysis', name: 'Sentiment', count: availableModels.filter(m => m.category === 'sentiment-analysis').length }
  ]

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

  const getModelRating = (model: ModelInfo) => {
    // Calculate rating based on downloads and likes
    const score = (model.downloads / 1000000) + (model.likes / 1000)
    return Math.min(5, Math.max(1, Math.round(score * 2) / 2))
  }

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
      <div className="bg-white dark:bg-gray-800 rounded-xl shadow-2xl w-full max-w-6xl max-h-[90vh] overflow-hidden flex flex-col">
        {/* Header */}
        <div className="flex items-center justify-between p-6 border-b border-gray-200 dark:border-gray-700">
          <div>
            <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
              Select AI Model
            </h2>
            <div className="flex items-center gap-4 mt-2 text-sm text-gray-600 dark:text-gray-400">
              <div className="flex items-center gap-1">
                {isOnline ? (
                  <>
                    <Wifi className="w-4 h-4 text-green-500" />
                    <span>Online</span>
                  </>
                ) : (
                  <>
                    <WifiOff className="w-4 h-4 text-red-500" />
                    <span>Offline</span>
                  </>
                )}
              </div>
              <span>{filteredModels.length} models available</span>
              <span>{favorites.length} favorites</span>
            </div>
          </div>
          <div className="flex items-center gap-2">
            <button
              onClick={handleRefresh}
              disabled={isRefreshing || !isOnline}
              className="p-2 text-gray-400 hover:text-gray-600 dark:hover:text-gray-300 disabled:opacity-50"
            >
              <RefreshCw className={`w-5 h-5 ${isRefreshing ? 'animate-spin' : ''}`} />
            </button>
            <button
              onClick={onClose}
              className="p-2 text-gray-400 hover:text-gray-600 dark:hover:text-gray-300"
            >
              <X className="w-6 h-6" />
            </button>
          </div>
        </div>

        {/* Controls */}
        <div className="p-6 border-b border-gray-200 dark:border-gray-700">
          <div className="flex flex-col lg:flex-row gap-4">
            {/* Search */}
            <div className="flex-1 relative">
              <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400 w-4 h-4" />
              <input
                type="text"
                placeholder="Search models, authors, or tags..."
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                className="w-full pl-10 pr-4 py-3 border border-gray-300 dark:border-gray-600 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
              />
            </div>

            {/* Sort */}
            <select
              value={sortBy}
              onChange={(e) => setSortBy(e.target.value as any)}
              className="px-3 py-3 border border-gray-300 dark:border-gray-600 rounded-lg bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
            >
              <option value="downloads">Most Downloaded</option>
              <option value="likes">Most Liked</option>
              <option value="name">Name A-Z</option>
              <option value="recent">Recently Updated</option>
            </select>

            {/* Favorites Toggle */}
            <button
              onClick={() => setShowFavorites(!showFavorites)}
              className={`px-4 py-3 rounded-lg font-medium transition-colors ${
                showFavorites
                  ? 'bg-red-600 text-white'
                  : 'border border-gray-300 dark:border-gray-600 text-gray-700 dark:text-gray-300 hover:bg-gray-50 dark:hover:bg-gray-700'
              }`}
            >
              <Heart className={`w-4 h-4 inline mr-2 ${showFavorites ? 'fill-current' : ''}`} />
              Favorites
            </button>
          </div>

          {/* Categories */}
          <div className="flex flex-wrap gap-2 mt-4">
            {categories.map(category => (
              <button
                key={category.id}
                onClick={() => setSelectedCategory(category.id)}
                className={`px-3 py-1 rounded-full text-sm font-medium transition-colors ${
                  selectedCategory === category.id
                    ? 'bg-blue-600 text-white'
                    : 'bg-gray-200 dark:bg-gray-600 text-gray-700 dark:text-gray-300 hover:bg-gray-300 dark:hover:bg-gray-500'
                }`}
              >
                {category.name} ({category.count})
              </button>
            ))}
          </div>
        </div>

        {/* Content */}
        <div className="flex-1 overflow-y-auto">
          {isLoading ? (
            <div className="flex items-center justify-center py-12">
              <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-blue-600"></div>
            </div>
          ) : filteredModels.length === 0 ? (
            <div className="flex flex-col items-center justify-center py-12 text-gray-500 dark:text-gray-400">
              <Search className="w-12 h-12 mb-4" />
              <h3 className="text-lg font-medium mb-2">No models found</h3>
              <p>Try adjusting your search or filters</p>
            </div>
          ) : (
            <div className="p-6">
              <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
                {filteredModels.map((model) => {
                  const rating = getModelRating(model)
                  const isFavorite = favorites.includes(model.id)
                  const isCurrentModel = currentModel === model.id
                  
                  return (
                    <div
                      key={model.id}
                      className={`bg-white dark:bg-gray-700 rounded-xl shadow-lg hover:shadow-xl transition-all duration-200 cursor-pointer border-2 ${
                        isCurrentModel 
                          ? 'border-blue-500 bg-blue-50 dark:bg-blue-900/20' 
                          : 'border-transparent hover:border-gray-300 dark:hover:border-gray-600'
                      }`}
                      onClick={() => onSelectModel(model)}
                    >
                      <div className="p-4">
                        {/* Header */}
                        <div className="flex items-start justify-between mb-3">
                          <div className="flex-1 min-w-0">
                            <h3 className="font-semibold text-gray-900 dark:text-white truncate">
                              {model.name}
                            </h3>
                            <p className="text-sm text-gray-600 dark:text-gray-400 truncate">
                              by {model.author}
                            </p>
                          </div>
                          <button
                            onClick={(e) => {
                              e.stopPropagation()
                              toggleFavorite(model.id)
                            }}
                            className="ml-2 p-1 hover:bg-gray-100 dark:hover:bg-gray-600 rounded"
                          >
                            <Heart className={`w-4 h-4 ${isFavorite ? 'text-red-500 fill-current' : 'text-gray-400'}`} />
                          </button>
                        </div>

                        {/* Description */}
                        <p className="text-sm text-gray-700 dark:text-gray-300 mb-3 line-clamp-2">
                          {model.description}
                        </p>

                        {/* Rating */}
                        <div className="flex items-center gap-1 mb-3">
                          {[...Array(5)].map((_, i) => (
                            <Star
                              key={i}
                              className={`w-3 h-3 ${
                                i < rating ? 'text-yellow-500 fill-current' : 'text-gray-300'
                              }`}
                            />
                          ))}
                          <span className="text-xs text-gray-500 dark:text-gray-400 ml-1">
                            {rating.toFixed(1)}
                          </span>
                        </div>

                        {/* Stats */}
                        <div className="flex items-center justify-between text-xs text-gray-500 dark:text-gray-400 mb-3">
                          <div className="flex items-center gap-1">
                            <Download className="w-3 h-3" />
                            <span>{formatNumber(model.downloads)}</span>
                          </div>
                          <div className="flex items-center gap-1">
                            <Heart className="w-3 h-3" />
                            <span>{formatNumber(model.likes)}</span>
                          </div>
                          <div className="flex items-center gap-1">
                            <Zap className="w-3 h-3" />
                            <span>{formatBytes(model.size)}</span>
                          </div>
                        </div>

                        {/* Tags */}
                        <div className="flex flex-wrap gap-1 mb-3">
                          {model.tags.slice(0, 2).map((tag) => (
                            <span
                              key={tag}
                              className="px-2 py-1 bg-blue-100 dark:bg-blue-900 text-blue-800 dark:text-blue-200 text-xs rounded"
                            >
                              {tag}
                            </span>
                          ))}
                          {model.tags.length > 2 && (
                            <span className="px-2 py-1 bg-gray-100 dark:bg-gray-600 text-gray-600 dark:text-gray-400 text-xs rounded">
                              +{model.tags.length - 2}
                            </span>
                          )}
                        </div>

                        {/* Action Button */}
                        <button
                          onClick={(e) => {
                            e.stopPropagation()
                            onSelectModel(model)
                          }}
                          className={`w-full py-2 px-4 rounded-lg font-medium transition-colors ${
                            isCurrentModel
                              ? 'bg-green-600 text-white'
                              : 'bg-blue-600 text-white hover:bg-blue-700'
                          }`}
                        >
                          {isCurrentModel ? (
                            <>
                              <CheckCircle className="w-4 h-4 inline mr-2" />
                              Current Model
                            </>
                          ) : (
                            'Select Model'
                          )}
                        </button>
                      </div>
                    </div>
                  )
                })}
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  )
}
