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
  ollama: {
    installed: boolean
    version?: string
    path?: string
  }
}

export interface OllamaResponse {
  model: string
  created_at: string
  response: string
  done: boolean
  context?: number[]
  total_duration?: number
  load_duration?: number
  prompt_eval_count?: number
  prompt_eval_duration?: number
  eval_count?: number
  eval_duration?: number
}

export interface OllamaRequest {
  model: string
  prompt: string
  stream?: boolean
  context?: number[]
  options?: {
    temperature?: number
    top_p?: number
    top_k?: number
    repeat_penalty?: number
    seed?: number
    stop?: string[]
    num_predict?: number
    num_ctx?: number
  }
}

export interface SystemPrompt {
  id: string
  name: string
  description: string
  content: string
  icon: string
  category: 'general' | 'coding' | 'creative' | 'analysis' | 'research'
}

export interface AppConfig {
  defaultModel: string
  cachePath: string
  gpuMode: boolean
  theme: 'light' | 'dark'
  autoSave: boolean
  maxTokens: number
  temperature: number
  systemPrompts: SystemPrompt[]
}
