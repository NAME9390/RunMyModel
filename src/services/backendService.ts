import { invoke } from '@tauri-apps/api/core'

export interface ChatMessage {
  id?: string
  role: 'user' | 'assistant' | 'system'
  content: string
  timestamp: number
}

export interface OllamaModelInfo {
  name: string
  size?: number
  modified_at?: string
  family?: string
  format?: string
  families?: string[]
  parameter_size?: string
  quantization_level?: string
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
export interface BackendModel {
  id: string
  name: string
  displayName: string
  description: string | null
  size: number // Model size in GB
  parameters: string // e.g., "7B", "13B", "70B"
  architecture: string // e.g., "Llama 2", "Mistral", "CodeLlama"
  quantized: boolean
  quantization: string | null // e.g., "Q4_K_M", "Q8_0"
  ollamaName: string | null // Name used in ollama pull command
  createdAt: string
  updatedAt: string
}

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
  // Backend specific fields
  displayName?: string
  parameters?: string
  architecture?: string
  quantized?: boolean
  quantization?: string
  ollamaName?: string
}

export class BackendService {
  private static instance: BackendService
  private availableModels: ModelInfo[] = []
  private cachedModels: ModelInfo[] = []
  private lastFetchTime: number = 0
  private readonly CACHE_DURATION = 30 * 60 * 1000 // 30 minutes
  private readonly API_BASE_URL = 'http://localhost:3001/api' // Backend API URL

  private constructor() {}

  public static getInstance(): BackendService {
    if (!BackendService.instance) {
      BackendService.instance = new BackendService()
    }
    return BackendService.instance
  }

  public async initialize(): Promise<void> {
    try {
      // Load cached models first
      await this.loadCachedModels()
      
      // Try to fetch fresh models from backend API
      await this.fetchModelsFromBackend()
      
      console.log('Backend service initialized successfully')
    } catch (error) {
      console.error('Failed to initialize backend service:', error)
      // Fallback to cached models or static models
      await this.loadFallbackModels()
    }
  }

  private async loadCachedModels(): Promise<void> {
    try {
      const cached = localStorage.getItem('backend_models_cache')
      const cacheTime = localStorage.getItem('backend_models_cache_time')
      
      if (cached && cacheTime) {
        const timeDiff = Date.now() - parseInt(cacheTime)
        if (timeDiff < this.CACHE_DURATION) {
          this.cachedModels = JSON.parse(cached)
          this.availableModels = [...this.cachedModels]
          console.log('Loaded cached backend models:', this.cachedModels.length)
        }
      }
    } catch (error) {
      console.error('Error loading cached backend models:', error)
    }
  }

  private async fetchModelsFromBackend(): Promise<void> {
    try {
      console.log('Fetching models from backend API...')
      
      const response = await fetch(`${this.API_BASE_URL}/models`)
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`)
      }
      
      const backendModels: BackendModel[] = await response.json()
      
      // Transform backend models to ModelInfo format
      const transformedModels: ModelInfo[] = backendModels.map(model => this.transformBackendModel(model))
      
      this.availableModels = transformedModels
      this.cachedModels = [...this.availableModels]
      this.lastFetchTime = Date.now()

      // Cache the models
      localStorage.setItem('backend_models_cache', JSON.stringify(this.availableModels))
      localStorage.setItem('backend_models_cache_time', this.lastFetchTime.toString())

      console.log('Successfully fetched', this.availableModels.length, 'models from backend')
    } catch (error) {
      console.error('Error fetching models from backend:', error)
      throw error
    }
  }

  private transformBackendModel(model: BackendModel): ModelInfo {
    return {
      id: model.id,
      name: model.name,
      displayName: model.displayName,
      size: model.size * 1000000000, // Convert GB to bytes
      downloads: this.estimateDownloads(model),
      likes: this.estimateLikes(model),
      tags: this.generateTags(model),
      description: model.description || model.displayName,
      author: this.extractAuthor(model.name),
      lastModified: model.updatedAt,
      category: this.mapArchitectureToCategory(model.architecture),
      pipeline_tag: this.mapArchitectureToPipelineTag(model.architecture),
      library_name: 'transformers',
      private: false,
      gated: false,
      downloads_last_month: Math.floor(this.estimateDownloads(model) * 0.1),
      created_at: model.createdAt,
      updated_at: model.updatedAt,
      // Backend specific fields
      parameters: model.parameters,
      architecture: model.architecture,
      quantized: model.quantized,
      quantization: model.quantization || undefined,
      ollamaName: model.ollamaName || undefined
    }
  }

  private estimateDownloads(model: BackendModel): number {
    // Estimate downloads based on model popularity and size
    const baseDownloads = {
      'Llama 2': 1000000,
      'Llama 3.1': 800000,
      'Vicuna': 600000,
      'CodeLlama': 500000,
      'Falcon': 400000,
      'GPT-J': 300000,
      'OPT': 250000,
      'T5': 200000,
      'Pythia': 150000,
      'Cerebras-GPT': 100000,
      'Baichuan': 80000,
      'InternLM': 70000,
      'TinyLlama': 60000,
      'RedPajama': 50000,
      'Koala': 40000,
      'Alpaca': 30000,
      'Dolly': 20000,
      'StarCoder': 100000,
      'CodeGen': 80000,
      'OpenLLaMA': 90000,
      'MPT': 70000
    }
    
    const architectureDownloads = baseDownloads[model.architecture as keyof typeof baseDownloads] || 10000
    const sizeMultiplier = this.getSizeMultiplier(model.size)
    
    return Math.floor(architectureDownloads * sizeMultiplier)
  }

  private getSizeMultiplier(size: number): number {
    if (size <= 1) return 0.5
    if (size <= 3) return 0.8
    if (size <= 7) return 1.0
    if (size <= 15) return 1.2
    if (size <= 30) return 1.5
    return 2.0
  }

  private estimateLikes(model: BackendModel): number {
    return Math.floor(this.estimateDownloads(model) * 0.05)
  }

  private generateTags(model: BackendModel): string[] {
    const tags: string[] = []
    
    // Architecture tags
    tags.push(model.architecture.toLowerCase())
    
    // Parameter size tags
    tags.push(model.parameters.toLowerCase())
    
    // Quantization tags
    if (model.quantized && model.quantization) {
      tags.push(model.quantization.toLowerCase())
    }
    
    // Specialized tags based on architecture
    if (model.architecture.includes('Code')) {
      tags.push('coding', 'programming')
    }
    if (model.architecture.includes('Llama')) {
      tags.push('chat', 'conversational')
    }
    if (model.architecture.includes('T5')) {
      tags.push('summarization', 'translation')
    }
    if (model.architecture.includes('Tiny')) {
      tags.push('lightweight', 'efficient')
    }
    
    // Size-based tags
    if (model.size <= 1) {
      tags.push('small', 'lightweight')
    } else if (model.size <= 7) {
      tags.push('medium')
    } else {
      tags.push('large', 'powerful')
    }
    
    return [...new Set(tags)] // Remove duplicates
  }

  private extractAuthor(name: string): string {
    // Extract author from model name (e.g., "llama2-7b" -> "meta")
    const authorMap: { [key: string]: string } = {
      'llama': 'meta',
      'vicuna': 'lmsys',
      'codellama': 'meta',
      'falcon': 'technology-innovation-institute',
      'gpt-j': 'eleutherai',
      'opt': 'meta',
      't5': 'google',
      'pythia': 'eleutherai',
      'cerebras-gpt': 'cerebras',
      'baichuan': 'baichuan-inc',
      'internlm': 'internlm',
      'tinyllama': 'tinyllama',
      'redpajama': 'together',
      'koala': 'berkeley',
      'alpaca': 'stanford',
      'dolly': 'databricks',
      'starcoder': 'bigcode',
      'codegen': 'salesforce',
      'openllama': 'openlm-research',
      'mpt': 'mosaicml'
    }
    
    const lowerName = name.toLowerCase()
    for (const [key, author] of Object.entries(authorMap)) {
      if (lowerName.includes(key)) {
        return author
      }
    }
    
    return 'unknown'
  }

  private mapArchitectureToCategory(architecture: string): string {
    const mapping: { [key: string]: string } = {
      'Llama 2': 'text-generation',
      'Llama 3.1': 'text-generation',
      'Vicuna': 'text-generation',
      'CodeLlama': 'text-generation',
      'Falcon': 'text-generation',
      'GPT-J': 'text-generation',
      'OPT': 'text-generation',
      'T5': 'text2text-generation',
      'Pythia': 'text-generation',
      'Cerebras-GPT': 'text-generation',
      'Baichuan': 'text-generation',
      'InternLM': 'text-generation',
      'TinyLlama': 'text-generation',
      'RedPajama': 'text-generation',
      'Koala': 'text-generation',
      'Alpaca': 'text-generation',
      'Dolly': 'text-generation',
      'StarCoder': 'text-generation',
      'CodeGen': 'text-generation',
      'OpenLLaMA': 'text-generation',
      'MPT': 'text-generation'
    }
    return mapping[architecture] || 'text-generation'
  }

  private mapArchitectureToPipelineTag(architecture: string): string {
    return this.mapArchitectureToCategory(architecture)
  }

  private async loadFallbackModels(): Promise<void> {
    // Fallback to a subset of the most popular models if backend is unavailable
    this.availableModels = [
      {
        id: 'model-1',
        name: 'llama2-7b',
        displayName: 'Llama 2 7B',
        size: 3.8 * 1000000000,
        downloads: 1000000,
        likes: 50000,
        tags: ['llama', '7b', 'chat', 'conversational'],
        description: 'Meta research-friendly weights, strong generalist',
        author: 'meta',
        lastModified: new Date().toISOString(),
        category: 'text-generation',
        pipeline_tag: 'text-generation',
        library_name: 'transformers',
        private: false,
        gated: false,
        downloads_last_month: 100000,
        created_at: new Date().toISOString(),
        updated_at: new Date().toISOString(),
        parameters: '7B',
        architecture: 'Llama 2',
        quantized: true,
        quantization: 'Q4_K_M',
        ollamaName: 'llama2:7b'
      },
      {
        id: 'model-4',
        name: 'llama3.1-8b',
        displayName: 'Llama 3.1 8B',
        size: 4.1 * 1000000000,
        downloads: 800000,
        likes: 40000,
        tags: ['llama', '8b', 'chat', 'conversational'],
        description: 'Newer LLaMA family release with open weights',
        author: 'meta',
        lastModified: new Date().toISOString(),
        category: 'text-generation',
        pipeline_tag: 'text-generation',
        library_name: 'transformers',
        private: false,
        gated: false,
        downloads_last_month: 80000,
        created_at: new Date().toISOString(),
        updated_at: new Date().toISOString(),
        parameters: '8B',
        architecture: 'Llama 3.1',
        quantized: true,
        quantization: 'Q4_K_M',
        ollamaName: 'llama3.1:8b'
      },
      {
        id: 'model-45',
        name: 'codellama-7b-general',
        displayName: 'CodeLlama 7B (General)',
        size: 3.8 * 1000000000,
        downloads: 500000,
        likes: 25000,
        tags: ['codellama', '7b', 'coding', 'programming'],
        description: 'CodeLlama general chat variant, can do conversation',
        author: 'meta',
        lastModified: new Date().toISOString(),
        category: 'text-generation',
        pipeline_tag: 'text-generation',
        library_name: 'transformers',
        private: false,
        gated: false,
        downloads_last_month: 50000,
        created_at: new Date().toISOString(),
        updated_at: new Date().toISOString(),
        parameters: '7B',
        architecture: 'CodeLlama',
        quantized: true,
        quantization: 'Q4_K_M',
        ollamaName: 'codellama:7b'
      }
    ]
    console.log('Loaded fallback backend models:', this.availableModels.length)
  }

  public async refreshModels(): Promise<void> {
    try {
      await this.fetchModelsFromBackend()
    } catch (error) {
      console.error('Error refreshing backend models:', error)
      // Keep existing models if refresh fails
    }
  }

  public getAvailableModels(): ModelInfo[] {
    return this.availableModels
  }

  public async searchModels(query: string): Promise<ModelInfo[]> {
    try {
      // Filter models based on search query
      return this.availableModels.filter(model => 
        model.name.toLowerCase().includes(query.toLowerCase()) ||
        model.displayName?.toLowerCase().includes(query.toLowerCase()) ||
        model.description.toLowerCase().includes(query.toLowerCase()) ||
        model.tags.some(tag => tag.toLowerCase().includes(query.toLowerCase())) ||
        model.architecture?.toLowerCase().includes(query.toLowerCase())
      )
    } catch (error) {
      console.error('Error searching backend models:', error)
      return []
    }
  }

  public getModelInfo(modelId: string): ModelInfo | undefined {
    return this.availableModels.find(model => model.id === modelId)
  }

  public async getAllAvailableModels(): Promise<OllamaModelInfo[]> {
    try {
      const models = await invoke<OllamaModelInfo[]>('get_all_ollama_models')
      return models
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

      // Use the real Ollama integration via Tauri
      const response = await invoke<ChatResponse>('chat_with_ollama', {
        request: {
          model,
          messages: messages.map(msg => ({
            role: msg.role,
            content: msg.content
          })),
          temperature: request.temperature,
          max_tokens: request.max_tokens,
          stream: request.stream
        }
      })

      console.log('Received response:', response)
      return response
    } catch (error) {
      console.error('Detailed error in backend chat:', error)
      
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
      const models = await invoke<string[]>('get_ollama_models')
      return models
    } catch (error) {
      console.error('Error getting installed models:', error)
      return []
    }
  }

  public async installModel(modelName: string): Promise<string> {
    try {
      const result = await invoke<string>('pull_ollama_model', { modelName })
      return result
    } catch (error) {
      console.error('Error installing model:', error)
      throw error
    }
  }

  public async removeModel(modelName: string): Promise<string> {
    try {
      const result = await invoke<string>('remove_ollama_model', { modelName })
      return result
    } catch (error) {
      console.error('Error removing model:', error)
      throw error
    }
  }
}

export const backendService = BackendService.getInstance()
