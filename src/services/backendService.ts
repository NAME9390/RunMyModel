// Backend service for C++ Qt application
// Communicates with C++ backend via HTTP API

export interface ChatMessage {
  id?: string
  role: 'user' | 'assistant' | 'system'
  content: string
  timestamp: number
}

export interface HuggingFaceModelInfo {
  name: string
  size?: string
  task_type?: string
  rating?: number
  url?: string
  downloaded: boolean
  local_path?: string
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

export interface ModelDownloadProgress {
  model_name: string
  progress: number
  status: string
  downloaded_bytes: number
  total_bytes?: number
}

export interface SystemInfo {
  platform: string
  arch: string
  gpu: {
    available: boolean
    name?: string
    memory?: number
    driver?: string
  }
  cpu: {
    cores: number
    name: string
    memory: number
  }
  huggingface: {
    available: boolean
    cache_dir?: string
    models_downloaded: string[]
  }
}

export class BackendService {
  private static instance: BackendService
  private availableModels: HuggingFaceModelInfo[] = []
  private cachedModels: HuggingFaceModelInfo[] = []
  private lastFetchTime: number = 0
  private readonly CACHE_DURATION = 30 * 60 * 1000 // 30 minutes
  private API_BASE_URL = 'http://localhost:8080/api'

  private constructor() {
    // Allow overriding backend port via global (injected from Qt) or window var
    try {
      // @ts-ignore
      const runtimePort = (window as any)?.__BACKEND_PORT__
      if (runtimePort && typeof runtimePort === 'number') {
        this.API_BASE_URL = `http://localhost:${runtimePort}/api`
      }
    } catch {}
  }

  public static getInstance(): BackendService {
    if (!BackendService.instance) {
      BackendService.instance = new BackendService()
    }
    return BackendService.instance
  }

  private async makeRequest<T>(endpoint: string, method: 'GET' | 'POST' = 'GET', data?: any): Promise<T> {
    const url = `${this.API_BASE_URL}${endpoint}`
    const options: RequestInit = {
      method,
      headers: {
        'Content-Type': 'application/json',
      },
    }

    if (data && method === 'POST') {
      options.body = JSON.stringify(data)
    }

    try {
      const response = await fetch(url, options)
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`)
      }

      const result = await response.json()
      return result as T
    } catch (error) {
      console.error(`Error making request to ${endpoint}:`, error)
      throw error
    }
  }

  public async initialize(): Promise<void> {
    try {
      // Load cached models first
      await this.loadCachedModels()
      
      // Try to fetch fresh models from Hugging Face
      await this.fetchModelsFromHuggingFace()
      
      console.log('Backend service initialized successfully')
    } catch (error) {
      console.error('Failed to initialize backend service:', error)
      // Fallback to cached models or static models
      await this.loadFallbackModels()
    }
  }

  private async loadCachedModels(): Promise<void> {
    try {
      const cached = localStorage.getItem('huggingface_models_cache')
      const cacheTime = localStorage.getItem('huggingface_models_cache_time')
      
      if (cached && cacheTime) {
        const timeDiff = Date.now() - parseInt(cacheTime)
        if (timeDiff < this.CACHE_DURATION) {
          this.cachedModels = JSON.parse(cached)
          this.availableModels = [...this.cachedModels]
          console.log('Loaded cached Hugging Face models:', this.cachedModels.length)
        }
      }
    } catch (error) {
      console.error('Error loading cached Hugging Face models:', error)
    }
  }

  private async fetchModelsFromHuggingFace(): Promise<void> {
    try {
      console.log('Fetching models from Hugging Face...')
      
      const response = await this.makeRequest<{models: HuggingFaceModelInfo[]}>('/models')
      const models = response.models || []
      
      this.availableModels = models
      this.cachedModels = [...this.availableModels]
      this.lastFetchTime = Date.now()

      // Cache the models
      localStorage.setItem('huggingface_models_cache', JSON.stringify(this.availableModels))
      localStorage.setItem('huggingface_models_cache_time', this.lastFetchTime.toString())

      console.log('Successfully fetched', this.availableModels.length, 'models from Hugging Face')
    } catch (error) {
      console.error('Error fetching models from Hugging Face:', error)
      throw error
    }
  }

  private async loadFallbackModels(): Promise<void> {
    // Fallback to a subset of the most popular models if Hugging Face is unavailable
    this.availableModels = [
      {
        name: 'Qwen/Qwen2.5-7B-Instruct',
        size: '7B',
        task_type: 'Text Generation',
        rating: 12,
        url: 'https://huggingface.co/Qwen/Qwen2.5-7B-Instruct',
        downloaded: false,
        local_path: undefined
      },
      {
        name: 'meta-llama/Llama-3.1-8B-Instruct',
        size: '8B',
        task_type: 'Text Generation',
        rating: 12,
        url: 'https://huggingface.co/meta-llama/Llama-3.1-8B-Instruct',
        downloaded: false,
        local_path: undefined
      },
      {
        name: 'microsoft/CodeLlama-7b-Instruct-hf',
        size: '7B',
        task_type: 'Code Generation',
        rating: 12,
        url: 'https://huggingface.co/microsoft/CodeLlama-7b-Instruct-hf',
        downloaded: false,
        local_path: undefined
      }
    ]
    console.log('Loaded fallback Hugging Face models:', this.availableModels.length)
  }

  public async refreshModels(): Promise<void> {
    try {
      await this.fetchModelsFromHuggingFace()
    } catch (error) {
      console.error('Error refreshing Hugging Face models:', error)
      // Keep existing models if refresh fails
    }
  }

  public getAvailableModels(): HuggingFaceModelInfo[] {
    return this.availableModels
  }

  public async searchModels(query: string): Promise<HuggingFaceModelInfo[]> {
    try {
      // Filter models based on search query
      return this.availableModels.filter(model => 
        model.name.toLowerCase().includes(query.toLowerCase()) ||
        model.task_type?.toLowerCase().includes(query.toLowerCase()) ||
        model.size?.toLowerCase().includes(query.toLowerCase())
      )
    } catch (error) {
      console.error('Error searching Hugging Face models:', error)
      return []
    }
  }

  public getModelInfo(modelName: string): HuggingFaceModelInfo | undefined {
    return this.availableModels.find(model => model.name === modelName)
  }

  public async getAllAvailableModels(): Promise<HuggingFaceModelInfo[]> {
    try {
      const response = await this.makeRequest<{models: HuggingFaceModelInfo[]}>('/models')
      return response.models || []
    } catch (error) {
      console.error('Error getting all available models:', error)
      return []
    }
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

  public async chat(request: ChatRequest): Promise<ChatResponse> {
    try {
      const { model, messages } = request

      console.log('Sending chat request:', { model, messageCount: messages.length })

      // Use the Hugging Face integration via HTTP API
      const response = await this.makeRequest<ChatResponse>('/chat', 'POST', {
        model,
        messages: messages.map(msg => ({
          role: msg.role,
          content: msg.content
        })),
        temperature: request.temperature,
        max_tokens: request.max_tokens,
        stream: request.stream
      })

      console.log('Received response:', response)
      return response
    } catch (error) {
      console.error('Detailed error in Hugging Face chat:', error)
      
      // Extract more detailed error information
      let errorMessage = 'Unknown error'
      if (error instanceof Error) {
        errorMessage = error.message
      } else if (typeof error === 'string') {
        errorMessage = error
      } else if (error && typeof error === 'object') {
        errorMessage = JSON.stringify(error)
      }
      
      throw new Error(`Failed to generate response: ${errorMessage}`)
    }
  }

  public async getInstalledModels(): Promise<string[]> {
    try {
      const models = await this.getAllAvailableModels()
      return models.filter(model => model.downloaded).map(model => model.name)
    } catch (error) {
      console.error('Error getting installed models:', error)
      return []
    }
  }

  public async installModel(modelName: string): Promise<string> {
    try {
      const response = await this.makeRequest<{message: string, success: boolean}>('/download-model', 'POST', { model_name: modelName })
      return response.message
    } catch (error) {
      console.error('Error installing model:', error)
      throw error
    }
  }

  public async removeModel(modelName: string): Promise<string> {
    try {
      const response = await this.makeRequest<{message: string, success: boolean}>('/remove-model', 'POST', { model_name: modelName })
      return response.message
    } catch (error) {
      console.error('Error removing model:', error)
      throw error
    }
  }

  public async getSystemInfo(): Promise<SystemInfo> {
    try {
      const info = await this.makeRequest<SystemInfo>('/system-info')
      return info
    } catch (error) {
      console.error('Error getting system info:', error)
      throw error
    }
  }

  public async getModelDownloadProgress(modelName: string): Promise<ModelDownloadProgress> {
    try {
      const progress = await this.makeRequest<ModelDownloadProgress>('/download-progress', 'POST', { model_name: modelName })
      return progress
    } catch (error) {
      console.error('Error getting download progress:', error)
      throw error
    }
  }

  public formatModelSize(size: string): string {
    // Convert size string to bytes for display
    const sizeMap: { [key: string]: number } = {
      '125M': 125 * 1024 * 1024,
      '345M': 345 * 1024 * 1024,
      '774M': 774 * 1024 * 1024,
      '1.3B': 1.3 * 1024 * 1024 * 1024,
      '2B': 2 * 1024 * 1024 * 1024,
      '2.7B': 2.7 * 1024 * 1024 * 1024,
      '3.8B': 3.8 * 1024 * 1024 * 1024,
      '6.7B': 6.7 * 1024 * 1024 * 1024,
      '7B': 7 * 1024 * 1024 * 1024,
      '8B': 8 * 1024 * 1024 * 1024,
      '9B': 9 * 1024 * 1024 * 1024,
      '11B': 11 * 1024 * 1024 * 1024,
      '13B': 13 * 1024 * 1024 * 1024,
      '14B': 14 * 1024 * 1024 * 1024,
      '27B': 27 * 1024 * 1024 * 1024,
      '32B': 32 * 1024 * 1024 * 1024,
      '33B': 33 * 1024 * 1024 * 1024,
      '34B': 34 * 1024 * 1024 * 1024,
      '70B': 70 * 1024 * 1024 * 1024,
      '72B': 72 * 1024 * 1024 * 1024,
      '480B': 480 * 1024 * 1024 * 1024,
    }
    
    const bytes = sizeMap[size] || 0
    return this.formatBytes(bytes)
  }

  public getModelRating(model: HuggingFaceModelInfo): number {
    return model.rating || 0
  }

  public getModelTaskType(model: HuggingFaceModelInfo): string {
    return model.task_type || 'Text Generation'
  }

  public isModelDownloaded(model: HuggingFaceModelInfo): boolean {
    return model.downloaded
  }
}

export const backendService = BackendService.getInstance()