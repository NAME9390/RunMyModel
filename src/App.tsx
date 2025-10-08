/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 RunMyModel.org
 */

import { useState, useEffect } from 'react'
// import { invoke } from '@tauri-apps/api/core'
// import { listen } from '@tauri-apps/api/event'
import { ModernSidebar } from './components/ModernSidebar'
import { ModernChatWindow } from './components/ModernChatWindow'
import { SplashScreen } from './components/SplashScreen'
import { AboutModal } from './components/AboutModal'
import { WelcomePage } from './components/WelcomePage'
import { ModelsPage } from './components/ModelsPage'
import { CustomModelPage } from './components/CustomModelPage'
import { ModelBrowser } from './components/ModelBrowser'
import { useAppStore } from './store/appStore'
import { useChatStore } from './store/chatStore'
import { useModelStore } from './store/modelStore'
import { backendService } from './services/backendService'
// import { SystemInfo } from './types/system'

type Page = 'welcome' | 'chat' | 'models' | 'custom-models'

function App() {
  const [isLoading, setIsLoading] = useState(true)
  const [showAbout, setShowAbout] = useState(false)
  const [showModelBrowser, setShowModelBrowser] = useState(false)
  const [currentPage, setCurrentPage] = useState<Page>('welcome')
  const { theme, setTheme } = useAppStore()
  const { currentModel, setCurrentModel } = useModelStore()
  const { chats, addChat } = useChatStore()

  useEffect(() => {
    const initializeApp = async () => {
      try {
        // Initialize backend service for models
        await backendService.initialize()
        
        // Check system info
        // const systemInfo = await invoke<SystemInfo>('get_system_info')
        // console.log('System info:', systemInfo)
        
        // Load saved theme
        const savedTheme = localStorage.getItem('theme')
        if (savedTheme) {
          setTheme(savedTheme as 'light' | 'dark')
        }
        
        // Load saved chats
        const savedChats = localStorage.getItem('chats')
        if (savedChats) {
          try {
            const parsedChats = JSON.parse(savedChats)
            parsedChats.forEach((chat: any) => addChat(chat))
          } catch (error) {
            console.error('Error loading saved chats:', error)
          }
        }
        
        // Load saved current model
        const savedModel = localStorage.getItem('currentModel')
        if (savedModel) {
          setCurrentModel(savedModel)
        }
        
        setIsLoading(false)
      } catch (error) {
        console.error('Error initializing app:', error)
        setIsLoading(false)
      }
    }

    initializeApp()
  }, [setTheme, addChat, setCurrentModel])

  useEffect(() => {
    // Apply theme to document
    document.documentElement.classList.toggle('dark', theme === 'dark')
  }, [theme])

  useEffect(() => {
    // Save chats to localStorage whenever chats change
    localStorage.setItem('chats', JSON.stringify(chats))
  }, [chats])

  useEffect(() => {
    // Save current model to localStorage
    if (currentModel) {
      localStorage.setItem('currentModel', currentModel)
    }
  }, [currentModel])

  // Listen for Ollama events
  useEffect(() => {
    // const unlisten = listen('ollama-progress', (event: any) => {
    //   console.log('Ollama progress:', event.payload)
    // })

    // return () => {
    //   unlisten.then((fn: any) => fn())
    // }
  }, [])

  // Listen for deep link events
  useEffect(() => {
    const handleDeepLink = async () => {
      try {
        // Listen for protocol events
        // const unlisten = await listen('tauri://url', (event: any) => {
        //   console.log('Deep link received:', event.payload)
        //   const url = event.payload as string
          
        //   // Parse the URL to extract model name
        //   if (url.startsWith('runmymodel://install/')) {
        //     const modelName = decodeURIComponent(url.replace('runmymodel://install/', ''))
        //     console.log('Installing model from deep link:', modelName)
            
        //     // Install the model
        //     invoke('install_model_from_url', { modelName })
        //       .then((result: any) => {
        //         console.log('Model installation result:', result)
        //         // Refresh installed models
        //         useModelStore.getState().refreshInstalledModels()
        //       })
        //       .catch((error: any) => {
        //         console.error('Failed to install model:', error)
        //       })
        //   }
        // })

        // return unlisten
      } catch (error) {
        console.error('Error setting up deep link listener:', error)
        return () => {}
      }
    }

    let cleanup: (() => void) | undefined

    handleDeepLink().then((unlisten) => {
      cleanup = unlisten
    })

    return () => {
      if (cleanup) {
        cleanup()
      }
    }
  }, [])

  if (isLoading) {
    return <SplashScreen />
  }

  const renderCurrentPage = () => {
    switch (currentPage) {
      case 'welcome':
        return <WelcomePage onGetStarted={() => setCurrentPage('chat')} />
      case 'chat':
        return <ModernChatWindow />
      case 'models':
        return <ModelsPage onClose={() => setCurrentPage('chat')} />
      case 'custom-models':
        return <CustomModelPage onClose={() => setCurrentPage('chat')} />
      default:
        return <ModernChatWindow />
    }
  }

  return (
    <div className="h-screen flex bg-gray-50 dark:bg-gray-900">
      {currentPage !== 'welcome' && (
        <ModernSidebar 
          onShowAbout={() => setShowAbout(true)}
          onShowModels={() => setCurrentPage('models')}
          onShowCustomModels={() => setCurrentPage('custom-models')}
          onShowModelBrowser={() => setShowModelBrowser(true)}
        />
      )}
      
      <div className="flex-1 flex flex-col">
        {renderCurrentPage()}
      </div>

      {showAbout && (
        <AboutModal onClose={() => setShowAbout(false)} />
      )}

      {showModelBrowser && (
        <ModelBrowser onClose={() => setShowModelBrowser(false)} />
      )}
    </div>
  )
}

export default App
