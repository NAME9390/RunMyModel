import { create } from 'zustand'
import { persist, createJSONStorage } from 'zustand/middleware'

export interface AppState {
  theme: 'light' | 'dark'
  sidebarCollapsed: boolean
  isLoading: boolean
  error: string | null
  setTheme: (theme: 'light' | 'dark') => void
  setSidebarCollapsed: (collapsed: boolean) => void
  setLoading: (loading: boolean) => void
  setError: (error: string | null) => void
}

export const useAppStore = create<AppState>()(
  persist(
    (set) => ({
      theme: 'light',
      sidebarCollapsed: false,
      isLoading: false,
      error: null,
      setTheme: (theme) => set({ theme }),
      setSidebarCollapsed: (sidebarCollapsed) => set({ sidebarCollapsed }),
      setLoading: (isLoading) => set({ isLoading }),
      setError: (error) => set({ error }),
    }),
    {
      name: 'app-storage',
      storage: createJSONStorage(() => localStorage),
      partialize: (state) => ({ theme: state.theme, sidebarCollapsed: state.sidebarCollapsed }),
    }
  )
)
