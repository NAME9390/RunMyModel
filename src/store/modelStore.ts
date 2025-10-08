import { create } from 'zustand'
import { persist, createJSONStorage } from 'zustand/middleware'
import { backendService, HuggingFaceModelInfo } from '../services/backendService'

export interface ModelDownloadProgress {
  model: string
  status: 'downloading' | 'completed' | 'error'
  progress: number
  total: number
  downloaded: number
  speed?: number
  eta?: number
}

export interface ModelState {
  availableModels: HuggingFaceModelInfo[]
  installedModels: string[]
  currentModel: string | null
  downloadProgress: Record<string, ModelDownloadProgress>
  isLoading: boolean
  error: string | null
  setAvailableModels: (models: HuggingFaceModelInfo[]) => void
  setInstalledModels: (models: string[]) => void
  setCurrentModel: (model: string | null) => void
  setDownloadProgress: (model: string, progress: ModelDownloadProgress) => void
  removeDownloadProgress: (model: string) => void
  setLoading: (loading: boolean) => void
  setError: (error: string | null) => void
  refreshModels: () => Promise<void>
  refreshInstalledModels: () => Promise<void>
  downloadModel: (modelName: string) => Promise<void>
  removeModel: (modelName: string) => Promise<void>
  searchModels: (query: string) => Promise<HuggingFaceModelInfo[]>
}

export const useModelStore = create<ModelState>()(
  persist(
    (set, get) => ({
      availableModels: [],
      installedModels: [],
      currentModel: null,
      downloadProgress: {},
      isLoading: false,
      error: null,
      
      setAvailableModels: (models) => set({ availableModels: models }),
      setInstalledModels: (models) => set({ installedModels: models }),
      setCurrentModel: (model) => set({ currentModel: model }),
      setDownloadProgress: (model, progress) => 
        set(state => ({ 
          downloadProgress: { ...state.downloadProgress, [model]: progress } 
        })),
      removeDownloadProgress: (model) => 
        set(state => {
          const { [model]: _, ...rest } = state.downloadProgress
          return { downloadProgress: rest }
        }),
      setLoading: (loading) => set({ isLoading: loading }),
      setError: (error) => set({ error }),
      
      refreshModels: async () => {
        set({ isLoading: true, error: null })
        try {
          await backendService.refreshModels()
          const models = backendService.getAvailableModels()
          set({ availableModels: models, isLoading: false })
        } catch (error) {
          set({ 
            error: error instanceof Error ? error.message : 'Failed to refresh models',
            isLoading: false 
          })
        }
      },

      refreshInstalledModels: async () => {
        try {
          const installed = await backendService.getInstalledModels()
          set({ installedModels: installed })
        } catch (error) {
          console.error('Error refreshing installed models:', error)
          set({ error: 'Failed to refresh installed models' })
        }
      },

      downloadModel: async (modelName) => {
        const progress: ModelDownloadProgress = {
          model: modelName,
          status: 'downloading',
          progress: 0,
          total: 100,
          downloaded: 0,
          speed: 0,
          eta: 0
        }
        
        set(state => ({ 
          downloadProgress: { ...state.downloadProgress, [modelName]: progress } 
        }))

        try {
          // Real Hugging Face model download
          await backendService.installModel(modelName)
          
          // Complete the download
          set(state => ({
            downloadProgress: {
              ...state.downloadProgress,
              [modelName]: {
                ...state.downloadProgress[modelName],
                status: 'completed',
                progress: 100,
                downloaded: 100
              }
            }
          }))

          // Refresh installed models and available models
          await get().refreshInstalledModels()
          await get().refreshModels()
        } catch (error) {
          set(state => ({
            downloadProgress: {
              ...state.downloadProgress,
              [modelName]: {
                ...state.downloadProgress[modelName],
                status: 'error'
              }
            }
          }))
          throw error
        }
      },

      removeModel: async (modelName) => {
        try {
          await backendService.removeModel(modelName)
          await get().refreshInstalledModels()
          await get().refreshModels()
        } catch (error) {
          console.error('Error removing model:', error)
          throw error
        }
      },

      searchModels: async (query) => {
        try {
          return await backendService.searchModels(query)
        } catch (error) {
          console.error('Error searching models:', error)
          return []
        }
      }
    }),
    {
      name: 'model-store',
      storage: createJSONStorage(() => localStorage),
      partialize: (state) => ({ 
        currentModel: state.currentModel,
        availableModels: state.availableModels,
        installedModels: state.installedModels
      })
    }
  )
)