import React, { useState, useEffect } from 'react'
import { 
  Search, 
  Filter, 
  Download, 
  Star, 
  Clock, 
  CheckCircle, 
  AlertCircle,
  Grid,
  List,
  SortAsc,
  SortDesc,
  X,
  Tag,
  HardDrive
} from 'lucide-react'
import { useModelStore } from '../store/modelStore'
import { backendService, ModelInfo } from '../services/backendService'

interface ModelsPageProps {
  onClose: () => void
}

export const ModelsPage: React.FC<ModelsPageProps> = ({ onClose }) => {
  const [searchQuery, setSearchQuery] = useState('')
  const [selectedCategory, setSelectedCategory] = useState<string>('all')
  const [sortBy, setSortBy] = useState<'name' | 'downloads' | 'likes' | 'size'>('downloads')
  const [sortOrder, setSortOrder] = useState<'asc' | 'desc'>('desc')
  const [viewMode, setViewMode] = useState<'grid' | 'list'>('grid')
  const [showFilters, setShowFilters] = useState(false)
  const [selectedTags, setSelectedTags] = useState<string[]>([])
  
  const { 
    availableModels, 
    downloadProgress, 
    refreshModels, 
    downloadModel,
    isLoading 
  } = useModelStore()

  const [filteredModels, setFilteredModels] = useState<ModelInfo[]>([])
  const [allTags, setAllTags] = useState<string[]>([])

  useEffect(() => {
    refreshModels()
  }, [refreshModels])

  useEffect(() => {
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

    // Filter by tags
    if (selectedTags.length > 0) {
      models = models.filter(model => 
        selectedTags.some(tag => model.tags.includes(tag))
      )
    }

    // Sort models
    models.sort((a, b) => {
      let aValue: any, bValue: any
      
      switch (sortBy) {
        case 'name':
          aValue = a.name.toLowerCase()
          bValue = b.name.toLowerCase()
          break
        case 'downloads':
          aValue = a.downloads
          bValue = b.downloads
          break
        case 'likes':
          aValue = a.likes
          bValue = b.likes
          break
        case 'size':
          aValue = a.size
          bValue = b.size
          break
        default:
          aValue = a.downloads
          bValue = b.downloads
      }

      if (sortOrder === 'asc') {
        return aValue > bValue ? 1 : -1
      } else {
        return aValue < bValue ? 1 : -1
      }
    })

    setFilteredModels(models)
  }, [availableModels, searchQuery, selectedCategory, sortBy, sortOrder, selectedTags])

  useEffect(() => {
    // Extract all unique tags
    const tags = new Set<string>()
    availableModels.forEach(model => {
      model.tags.forEach(tag => tags.add(tag))
    })
    setAllTags(Array.from(tags).sort())
  }, [availableModels])

  const categories = [
    { id: 'all', name: 'All Models', count: availableModels.length },
    { id: 'text-generation', name: 'Text Generation', count: availableModels.filter(m => m.category === 'text-generation').length },
    { id: 'code-generation', name: 'Code Generation', count: availableModels.filter(m => m.category === 'code-generation').length },
    { id: 'question-answering', name: 'Question Answering', count: availableModels.filter(m => m.category === 'question-answering').length },
    { id: 'summarization', name: 'Summarization', count: availableModels.filter(m => m.category === 'summarization').length },
    { id: 'translation', name: 'Translation', count: availableModels.filter(m => m.category === 'translation').length },
    { id: 'sentiment-analysis', name: 'Sentiment Analysis', count: availableModels.filter(m => m.category === 'sentiment-analysis').length },
    { id: 'token-classification', name: 'Named Entity Recognition', count: availableModels.filter(m => m.category === 'token-classification').length },
    { id: 'text-classification', name: 'Text Classification', count: availableModels.filter(m => m.category === 'text-classification').length }
  ]

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

  const toggleTag = (tag: string) => {
    setSelectedTags(prev => 
      prev.includes(tag) 
        ? prev.filter(t => t !== tag)
        : [...prev, tag]
    )
  }

  const clearFilters = () => {
    setSearchQuery('')
    setSelectedCategory('all')
    setSelectedTags([])
    setSortBy('downloads')
    setSortOrder('desc')
  }

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
      <div className="bg-white dark:bg-gray-800 rounded-xl shadow-2xl w-full max-w-7xl max-h-[90vh] overflow-hidden flex flex-col">
        {/* Header */}
        <div className="flex items-center justify-between p-6 border-b border-gray-200 dark:border-gray-700">
          <div>
            <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
              AI Models Library
            </h2>
            <p className="text-gray-600 dark:text-gray-400 mt-1">
              {filteredModels.length} of {availableModels.length} models
            </p>
          </div>
          <button
            onClick={onClose}
            className="text-gray-400 hover:text-gray-600 dark:hover:text-gray-300 p-2 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg"
          >
            <X className="w-6 h-6" />
          </button>
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

            {/* Sort Controls */}
            <div className="flex gap-2">
              <select
                value={sortBy}
                onChange={(e) => setSortBy(e.target.value as any)}
                className="px-3 py-3 border border-gray-300 dark:border-gray-600 rounded-lg bg-white dark:bg-gray-700 text-gray-900 dark:text-white"
              >
                <option value="downloads">Downloads</option>
                <option value="likes">Likes</option>
                <option value="name">Name</option>
                <option value="size">Size</option>
              </select>
              
              <button
                onClick={() => setSortOrder(sortOrder === 'asc' ? 'desc' : 'asc')}
                className="px-3 py-3 border border-gray-300 dark:border-gray-600 rounded-lg bg-white dark:bg-gray-700 text-gray-900 dark:text-white hover:bg-gray-50 dark:hover:bg-gray-600"
              >
                {sortOrder === 'asc' ? <SortAsc className="w-4 h-4" /> : <SortDesc className="w-4 h-4" />}
              </button>

              <button
                onClick={() => setViewMode(viewMode === 'grid' ? 'list' : 'grid')}
                className="px-3 py-3 border border-gray-300 dark:border-gray-600 rounded-lg bg-white dark:bg-gray-700 text-gray-900 dark:text-white hover:bg-gray-50 dark:hover:bg-gray-600"
              >
                {viewMode === 'grid' ? <List className="w-4 h-4" /> : <Grid className="w-4 h-4" />}
              </button>

              <button
                onClick={() => setShowFilters(!showFilters)}
                className="px-3 py-3 border border-gray-300 dark:border-gray-600 rounded-lg bg-white dark:bg-gray-700 text-gray-900 dark:text-white hover:bg-gray-50 dark:hover:bg-gray-600"
              >
                <Filter className="w-4 h-4" />
              </button>
            </div>
          </div>

          {/* Filters */}
          {showFilters && (
            <div className="mt-4 p-4 bg-gray-50 dark:bg-gray-700 rounded-lg">
              <div className="flex flex-wrap gap-4">
                {/* Categories */}
                <div className="flex flex-wrap gap-2">
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

                {/* Tags */}
                <div className="flex flex-wrap gap-2">
                  {allTags.slice(0, 10).map(tag => (
                    <button
                      key={tag}
                      onClick={() => toggleTag(tag)}
                      className={`px-2 py-1 rounded-full text-xs font-medium transition-colors flex items-center gap-1 ${
                        selectedTags.includes(tag)
                          ? 'bg-purple-600 text-white'
                          : 'bg-gray-200 dark:bg-gray-600 text-gray-700 dark:text-gray-300 hover:bg-gray-300 dark:hover:bg-gray-500'
                      }`}
                    >
                      <Tag className="w-3 h-3" />
                      {tag}
                    </button>
                  ))}
                </div>

                <button
                  onClick={clearFilters}
                  className="px-3 py-1 rounded-full text-sm font-medium bg-red-100 dark:bg-red-900 text-red-700 dark:text-red-300 hover:bg-red-200 dark:hover:bg-red-800"
                >
                  Clear Filters
                </button>
              </div>
            </div>
          )}
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
            <div className={`p-6 ${viewMode === 'grid' ? 'grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6' : 'space-y-4'}`}>
              {filteredModels.map((model) => {
                const downloadStatus = getDownloadStatus(model.id)
                return (
                  <div
                    key={model.id}
                    className={`bg-white dark:bg-gray-700 rounded-xl shadow-lg hover:shadow-xl transition-all duration-200 ${
                      viewMode === 'list' ? 'flex items-center p-6' : 'p-6'
                    }`}
                  >
                    {viewMode === 'grid' ? (
                      <>
                        <div className="flex items-start justify-between mb-4">
                          <div className="flex-1">
                            <h3 className="font-semibold text-gray-900 dark:text-white mb-1">
                              {model.name}
                            </h3>
                            <p className="text-sm text-gray-600 dark:text-gray-400 mb-2">
                              by {model.author}
                            </p>
                            <p className="text-sm text-gray-700 dark:text-gray-300 line-clamp-2">
                              {model.description}
                            </p>
                          </div>
                          <div className="flex items-center gap-1 text-yellow-500 ml-4">
                            <Star className="w-4 h-4 fill-current" />
                            <span className="text-sm font-medium">4.8</span>
                          </div>
                        </div>

                        <div className="flex flex-wrap gap-1 mb-4">
                          {model.tags.slice(0, 3).map((tag) => (
                            <span
                              key={tag}
                              className="px-2 py-1 bg-blue-100 dark:bg-blue-900 text-blue-800 dark:text-blue-200 text-xs rounded"
                            >
                              {tag}
                            </span>
                          ))}
                          {model.tags.length > 3 && (
                            <span className="px-2 py-1 bg-gray-100 dark:bg-gray-600 text-gray-600 dark:text-gray-400 text-xs rounded">
                              +{model.tags.length - 3}
                            </span>
                          )}
                        </div>

                        <div className="flex items-center justify-between text-sm text-gray-500 dark:text-gray-400 mb-4">
                          <div className="flex items-center gap-4">
                            <div className="flex items-center gap-1">
                              <HardDrive className="w-4 h-4" />
                              <span>{formatBytes(model.size)}</span>
                            </div>
                            <div className="flex items-center gap-1">
                              <Download className="w-4 h-4" />
                              <span>{formatNumber(model.downloads)}</span>
                            </div>
                            <div className="flex items-center gap-1">
                              <Star className="w-4 h-4" />
                              <span>{formatNumber(model.likes)}</span>
                            </div>
                          </div>
                        </div>

                        <div className="flex items-center justify-between">
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
                              className="flex items-center space-x-2 px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 text-sm font-medium"
                            >
                              <Download className="w-4 h-4" />
                              <span>Download</span>
                            </button>
                          )}
                        </div>
                      </>
                    ) : (
                      <>
                        <div className="flex-1">
                          <div className="flex items-center justify-between mb-2">
                            <h3 className="font-semibold text-gray-900 dark:text-white">
                              {model.name}
                            </h3>
                            <div className="flex items-center gap-1 text-yellow-500">
                              <Star className="w-4 h-4 fill-current" />
                              <span className="text-sm font-medium">4.8</span>
                            </div>
                          </div>
                          <p className="text-sm text-gray-600 dark:text-gray-400 mb-2">
                            by {model.author} • {model.description}
                          </p>
                          <div className="flex items-center gap-4 text-sm text-gray-500 dark:text-gray-400">
                            <div className="flex items-center gap-1">
                              <HardDrive className="w-4 h-4" />
                              <span>{formatBytes(model.size)}</span>
                            </div>
                            <div className="flex items-center gap-1">
                              <Download className="w-4 h-4" />
                              <span>{formatNumber(model.downloads)} downloads</span>
                            </div>
                            <div className="flex items-center gap-1">
                              <Star className="w-4 h-4" />
                              <span>{formatNumber(model.likes)} likes</span>
                            </div>
                          </div>
                        </div>
                        <div className="ml-6">
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
                              className="flex items-center space-x-2 px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 text-sm font-medium"
                            >
                              <Download className="w-4 h-4" />
                              <span>Download</span>
                            </button>
                          )}
                        </div>
                      </>
                    )}
                  </div>
                )
              })}
            </div>
          )}
        </div>
      </div>
    </div>
  )
}
