import { HfInference } from '@huggingface/inference'

export interface ModelInfo {
  id: string
  name: string
  size: number
  downloads: number
  likes: number
  tags: string[]
  description: string
  author: string
  lastModified: string
  category: string
  pipeline_tag?: string
  library_name?: string
  private?: boolean
  gated?: boolean
  downloads_last_month?: number
  created_at?: string
  updated_at?: string
}

export interface ChatMessage {
  id?: string
  role: 'user' | 'assistant' | 'system'
  content: string
  timestamp: number
}

export interface ChatRequest {
  model: string
  messages: ChatMessage[]
  temperature?: number
  max_tokens?: number
  stream?: boolean
}

export interface ChatResponse {
  content: string
  usage?: {
    prompt_tokens: number
    completion_tokens: number
    total_tokens: number
  }
}

export class HuggingFaceService {
  private static instance: HuggingFaceService
  private hf: HfInference
  private isInitialized = false
  private availableModels: ModelInfo[] = []
  private cachedModels: ModelInfo[] = []
  private lastFetchTime: number = 0
  private readonly CACHE_DURATION = 30 * 60 * 1000 // 30 minutes
  private readonly API_BASE_URL = 'https://huggingface.co/api/models'

  private constructor() {
    this.hf = new HfInference()
  }

  public static getInstance(): HuggingFaceService {
    if (!HuggingFaceService.instance) {
      HuggingFaceService.instance = new HuggingFaceService()
    }
    return HuggingFaceService.instance
  }

  public async initialize(): Promise<void> {
    if (this.isInitialized) return

    try {
      // Load cached models first
      await this.loadCachedModels()
      
      // Try to fetch fresh models from API
      await this.fetchModelsFromAPI()
      
      this.isInitialized = true
      console.log('HuggingFace service initialized successfully')
    } catch (error) {
      console.error('Failed to initialize HuggingFace service:', error)
      // Fallback to cached models or static models
      await this.loadFallbackModels()
      this.isInitialized = true
    }
  }

  private async loadCachedModels(): Promise<void> {
    try {
      const cached = localStorage.getItem('hf_models_cache')
      const cacheTime = localStorage.getItem('hf_models_cache_time')
      
      if (cached && cacheTime) {
        const timeDiff = Date.now() - parseInt(cacheTime)
        if (timeDiff < this.CACHE_DURATION) {
          this.cachedModels = JSON.parse(cached)
          this.availableModels = [...this.cachedModels]
          console.log('Loaded cached models:', this.cachedModels.length)
        }
      }
    } catch (error) {
      console.error('Error loading cached models:', error)
    }
  }

  private async fetchModelsFromAPI(): Promise<void> {
    try {
      console.log('Fetching models from Hugging Face API...')
      
      // Fetch popular models by category
      const categories = [
        'text-generation',
        'text2text-generation', 
        'question-answering',
        'summarization',
        'translation',
        'sentiment-analysis',
        'token-classification',
        'text-classification',
        'fill-mask',
        'zero-shot-classification'
      ]

      const allModels: ModelInfo[] = []
      
      for (const category of categories) {
        try {
          const response = await fetch(`${this.API_BASE_URL}?pipeline_tag=${category}&limit=20&sort=downloads`)
          if (response.ok) {
            const data = await response.json()
            const models = data.map((model: any) => this.transformModelData(model))
            allModels.push(...models)
          }
        } catch (error) {
          console.error(`Error fetching ${category} models:`, error)
        }
      }

      // Remove duplicates and sort by downloads
      const uniqueModels = this.removeDuplicates(allModels)
      uniqueModels.sort((a, b) => b.downloads - a.downloads)

      this.availableModels = uniqueModels.slice(0, 200) // Limit to top 200 models
      this.cachedModels = [...this.availableModels]
      this.lastFetchTime = Date.now()

      // Cache the models
      localStorage.setItem('hf_models_cache', JSON.stringify(this.availableModels))
      localStorage.setItem('hf_models_cache_time', this.lastFetchTime.toString())

      console.log('Successfully fetched', this.availableModels.length, 'models from API')
    } catch (error) {
      console.error('Error fetching models from API:', error)
      throw error
    }
  }

  private transformModelData(model: any): ModelInfo {
    return {
      id: model.id,
      name: model.id.split('/').pop() || model.id,
      size: this.estimateModelSize(model),
      downloads: model.downloads || 0,
      likes: model.likes || 0,
      tags: model.tags || [],
      description: model.cardData?.inference?.parameters?.description || model.id,
      author: model.id.split('/')[0] || 'unknown',
      lastModified: model.lastModified || new Date().toISOString(),
      category: this.mapPipelineTagToCategory(model.pipeline_tag),
      pipeline_tag: model.pipeline_tag,
      library_name: model.library_name,
      private: model.private,
      gated: model.gated,
      downloads_last_month: model.downloads_last_month,
      created_at: model.created_at,
      updated_at: model.updated_at
    }
  }

  private estimateModelSize(model: any): number {
    // Estimate model size based on tags and downloads
    const tags = model.tags || []
    if (tags.includes('7b')) return 7 * 1000000000
    if (tags.includes('13b')) return 13 * 1000000000
    if (tags.includes('30b')) return 30 * 1000000000
    if (tags.includes('70b')) return 70 * 1000000000
    if (tags.includes('large')) return 1000000000
    if (tags.includes('base')) return 250000000
    if (tags.includes('small')) return 100000000
    return 500000000 // Default estimate
  }

  private mapPipelineTagToCategory(pipelineTag: string): string {
    const mapping: { [key: string]: string } = {
      'text-generation': 'text-generation',
      'text2text-generation': 'text-generation',
      'question-answering': 'question-answering',
      'summarization': 'summarization',
      'translation': 'translation',
      'sentiment-analysis': 'sentiment-analysis',
      'token-classification': 'token-classification',
      'text-classification': 'text-classification',
      'fill-mask': 'text-generation',
      'zero-shot-classification': 'text-classification'
    }
    return mapping[pipelineTag] || 'text-generation'
  }

  private removeDuplicates(models: ModelInfo[]): ModelInfo[] {
    const seen = new Set<string>()
    return models.filter(model => {
      if (seen.has(model.id)) {
        return false
      }
      seen.add(model.id)
      return true
    })
  }

  private async loadFallbackModels(): Promise<void> {
    // Fallback to static models if API fails
    this.availableModels = [
      {
        id: 'microsoft/DialoGPT-medium',
        name: 'DialoGPT Medium',
        size: 345000000,
        downloads: 1000000,
        likes: 5000,
        tags: ['conversational', 'gpt', 'medium'],
        description: 'A conversational AI model by Microsoft',
        author: 'microsoft',
        lastModified: '2023-01-01',
        category: 'text-generation'
      },
      {
        id: 'facebook/blenderbot-400M-distill',
        name: 'BlenderBot 400M',
        size: 400000000,
        downloads: 800000,
        likes: 3000,
        tags: ['conversational', 'facebook', 'distill'],
        description: 'Facebook\'s conversational AI model',
        author: 'facebook',
        lastModified: '2023-01-01',
        category: 'text-generation'
      },
      {
        id: 'EleutherAI/gpt-neo-125M',
        name: 'GPT-Neo 125M',
        size: 125000000,
        downloads: 1500000,
        likes: 6000,
        tags: ['gpt', 'neo', 'small'],
        description: 'A smaller GPT model by EleutherAI',
        author: 'EleutherAI',
        lastModified: '2023-01-01',
        category: 'text-generation'
      }
    ]
    console.log('Loaded fallback models:', this.availableModels.length)
  }

  public async refreshModels(): Promise<void> {
    try {
      await this.fetchModelsFromAPI()
    } catch (error) {
      console.error('Error refreshing models:', error)
      // Keep existing models if refresh fails
    }
  }

  public async loadAvailableModels(): Promise<void> {
    // This method is now handled by initialize() which fetches models dynamically
    // Keeping this for backward compatibility
    if (this.availableModels.length === 0) {
      await this.loadFallbackModels()
    }
  }

  public getAvailableModels(): ModelInfo[] {
    return this.availableModels
  }

  public async chat(request: ChatRequest): Promise<ChatResponse> {
    try {
      const { model, messages, temperature = 0.7, max_tokens = 1000 } = request

      // Convert messages to the format expected by Hugging Face
      const prompt = messages
        .map(msg => `${msg.role === 'user' ? 'Human' : 'Assistant'}: ${msg.content}`)
        .join('\n') + '\nAssistant:'

      // Use text generation for conversational models
      const response = await this.hf.textGeneration({
        model,
        inputs: prompt,
        parameters: {
          temperature,
          max_new_tokens: max_tokens,
          return_full_text: false,
          do_sample: true
        }
      })

      return {
        content: (response as any)[0]?.generated_text || 'Sorry, I could not generate a response.',
        usage: {
          prompt_tokens: prompt.length,
          completion_tokens: (response as any)[0]?.generated_text?.length || 0,
          total_tokens: prompt.length + ((response as any)[0]?.generated_text?.length || 0)
        }
      }
    } catch (error) {
      console.error('Error in HuggingFace chat:', error)
      throw new Error(`Failed to generate response: ${error instanceof Error ? error.message : 'Unknown error'}`)
    }
  }

  public async downloadModel(modelId: string): Promise<void> {
    try {
      // For now, we'll just simulate the download
      // In a real implementation, you would download the GGUF file
      console.log(`Downloading model: ${modelId}`)
      
      // Simulate download progress
      await new Promise(resolve => setTimeout(resolve, 2000))
      
      console.log(`Model ${modelId} downloaded successfully`)
    } catch (error) {
      console.error('Error downloading model:', error)
      throw new Error(`Failed to download model: ${error instanceof Error ? error.message : 'Unknown error'}`)
    }
  }

  public async searchModels(query: string): Promise<ModelInfo[]> {
    try {
      // Filter models based on search query
      return this.availableModels.filter(model => 
        model.name.toLowerCase().includes(query.toLowerCase()) ||
        model.description.toLowerCase().includes(query.toLowerCase()) ||
        model.tags.some(tag => tag.toLowerCase().includes(query.toLowerCase()))
      )
    } catch (error) {
      console.error('Error searching models:', error)
      return []
    }
  }

  public getModelInfo(modelId: string): ModelInfo | undefined {
    return this.availableModels.find(model => model.id === modelId)
  }

  public formatBytes(bytes: number): string {
    if (bytes === 0) return '0 Bytes'
    const k = 1024
    const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB']
    const i = Math.floor(Math.log(bytes) / Math.log(k))
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i]
  }

  public isOnline(): boolean {
    return navigator.onLine
  }

  public getCacheStatus(): { cached: boolean; lastUpdate: number; modelCount: number } {
    return {
      cached: this.cachedModels.length > 0,
      lastUpdate: this.lastFetchTime,
      modelCount: this.availableModels.length
    }
  }
}

export const huggingFaceService = HuggingFaceService.getInstance()