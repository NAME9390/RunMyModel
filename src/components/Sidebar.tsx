import React, { useState } from 'react'
import { 
  MessageSquare, 
  Plus, 
  Settings, 
  Download, 
  Trash2, 
  Sun,
  Moon,
  Info,
  ExternalLink,
  ChevronLeft,
  ChevronRight
} from 'lucide-react'
import { useChatStore } from '../store/chatStore'
import { useModelStore } from '../store/modelStore'
import { useAppStore } from '../store/appStore'
import { ModelSelector } from './ModelSelector'
import { SystemPromptSelector } from './SystemPromptSelector'

interface SidebarProps {
  onShowAbout: () => void
  onShowModels: () => void
  onShowCustomModels: () => void
  onShowModelBrowser: () => void
}

export const Sidebar: React.FC<SidebarProps> = ({ onShowAbout, onShowCustomModels, onShowModelBrowser }) => {
  const [showModelSelector, setShowModelSelector] = useState(false)
  const [showSystemPrompts, setShowSystemPrompts] = useState(false)
  const { theme, setTheme, sidebarCollapsed, setSidebarCollapsed } = useAppStore()
  const { chats, currentChatId, addChat, setCurrentChat, deleteChat } = useChatStore()
  const { currentModel, availableModels, setCurrentModel } = useModelStore()

  const handleNewChat = () => {
    addChat({
      title: 'New Chat',
      messages: [],
      model: currentModel || availableModels[0]?.id || 'microsoft/DialoGPT-medium'
    })
  }

  const toggleTheme = () => {
    setTheme(theme === 'light' ? 'dark' : 'light')
  }

  const formatDate = (timestamp: number) => {
    const date = new Date(timestamp)
    const now = new Date()
    const diffTime = Math.abs(now.getTime() - date.getTime())
    const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24))
    
    if (diffDays === 1) return 'Today'
    if (diffDays === 2) return 'Yesterday'
    if (diffDays <= 7) return `${diffDays} days ago`
    return date.toLocaleDateString()
  }

  if (sidebarCollapsed) {
    return (
      <div className="w-16 bg-white dark:bg-gray-800 border-r border-gray-200 dark:border-gray-700 flex flex-col items-center py-4 space-y-4">
        <button
          onClick={() => setSidebarCollapsed(false)}
          className="p-2 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg transition-colors"
        >
          <ChevronRight className="w-5 h-5" />
        </button>
        <button
          onClick={handleNewChat}
          className="p-2 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg transition-colors"
          title="New Chat"
        >
          <Plus className="w-5 h-5" />
        </button>
        <button
          onClick={toggleTheme}
          className="p-2 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg transition-colors"
          title="Toggle Theme"
        >
          {theme === 'light' ? <Moon className="w-5 h-5" /> : <Sun className="w-5 h-5" />}
        </button>
        <button
          onClick={() => setShowModelSelector(true)}
          className="p-2 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg transition-colors"
          title="Model Manager"
        >
          <Download className="w-5 h-5" />
        </button>
        <button
          onClick={onShowAbout}
          className="p-2 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg transition-colors"
          title="About"
        >
          <Info className="w-5 h-5" />
        </button>
      </div>
    )
  }

  return (
    <>
      <div className="w-80 bg-white dark:bg-gray-800 border-r border-gray-200 dark:border-gray-700 flex flex-col h-full">
        {/* Header */}
        <div className="p-4 border-b border-gray-200 dark:border-gray-700">
          <div className="flex items-center justify-between mb-4">
            <div className="flex items-center space-x-2">
              <div className="w-8 h-8 bg-gradient-to-br from-blue-500 to-blue-600 rounded-lg flex items-center justify-center">
                <MessageSquare className="w-5 h-5 text-white" />
              </div>
              <h1 className="text-lg font-semibold text-gray-900 dark:text-white">
                RunMyModel
              </h1>
            </div>
            <button
              onClick={() => setSidebarCollapsed(true)}
              className="p-1 hover:bg-gray-100 dark:hover:bg-gray-700 rounded transition-colors"
            >
              <ChevronLeft className="w-4 h-4" />
            </button>
          </div>
          
          <button
            onClick={handleNewChat}
            className="w-full btn btn-primary py-2 px-4 rounded-lg flex items-center justify-center space-x-2"
          >
            <Plus className="w-4 h-4" />
            <span>New Chat</span>
          </button>
        </div>

        {/* Current Model */}
        <div className="p-4 border-b border-gray-200 dark:border-gray-700">
          <div className="flex items-center justify-between mb-2">
            <h3 className="text-sm font-medium text-gray-700 dark:text-gray-300">Current Model</h3>
            <button
              onClick={() => setShowModelSelector(true)}
              className="text-xs text-blue-600 hover:text-blue-700 dark:text-blue-400 dark:hover:text-blue-300"
            >
              Change
            </button>
          </div>
          <div className="text-sm text-gray-900 dark:text-white font-medium">
            {currentModel || 'No model selected'}
          </div>
          <div className="text-xs text-gray-500 dark:text-gray-400 mt-1">
            {availableModels.length} models available
          </div>
        </div>

        {/* System Prompts */}
        <div className="p-4 border-b border-gray-200 dark:border-gray-700">
          <button
            onClick={() => setShowSystemPrompts(true)}
            className="w-full btn btn-secondary py-2 px-4 rounded-lg flex items-center justify-center space-x-2"
          >
            <Settings className="w-4 h-4" />
            <span>System Prompts</span>
          </button>
        </div>

        {/* Chat History */}
        <div className="flex-1 overflow-y-auto">
          <div className="p-4">
            <h3 className="text-sm font-medium text-gray-700 dark:text-gray-300 mb-3">
              Recent Chats
            </h3>
            <div className="space-y-1">
              {chats.length === 0 ? (
                <div className="text-sm text-gray-500 dark:text-gray-400 text-center py-8">
                  No chats yet. Start a new conversation!
                </div>
              ) : (
                chats.map((chat) => (
                  <div
                    key={chat.id}
                    className={`sidebar-item cursor-pointer ${
                      currentChatId === chat.id ? 'active' : ''
                    }`}
                    onClick={() => setCurrentChat(chat.id)}
                  >
                    <MessageSquare className="w-4 h-4 flex-shrink-0" />
                    <div className="flex-1 min-w-0">
                      <div className="text-sm font-medium truncate">
                        {chat.title}
                      </div>
                      <div className="text-xs text-gray-500 dark:text-gray-400">
                        {formatDate(chat.updatedAt)}
                      </div>
                    </div>
                    <button
                      onClick={(e) => {
                        e.stopPropagation()
                        deleteChat(chat.id)
                      }}
                      className="p-1 hover:bg-gray-200 dark:hover:bg-gray-600 rounded opacity-0 group-hover:opacity-100 transition-opacity"
                    >
                      <Trash2 className="w-3 h-3" />
                    </button>
                  </div>
                ))
              )}
            </div>
          </div>
        </div>

        {/* Navigation */}
        <div className="p-4 border-t border-gray-200 dark:border-gray-700">
          <div className="space-y-2">
            <button
              onClick={onShowModelBrowser}
              className="w-full btn btn-secondary py-2 px-4 rounded-lg flex items-center justify-center space-x-2"
            >
              <Download className="w-4 h-4" />
              <span>Browse Models</span>
            </button>
            <button
              onClick={onShowCustomModels}
              className="w-full btn btn-secondary py-2 px-4 rounded-lg flex items-center justify-center space-x-2"
            >
              <Settings className="w-4 h-4" />
              <span>Custom Models</span>
            </button>
          </div>
        </div>

        {/* Footer */}
        <div className="p-4 border-t border-gray-200 dark:border-gray-700">
          <div className="flex items-center justify-between">
            <button
              onClick={toggleTheme}
              className="btn btn-ghost p-2 rounded-lg"
              title="Toggle Theme"
            >
              {theme === 'light' ? <Moon className="w-4 h-4" /> : <Sun className="w-4 h-4" />}
            </button>
            <button
              onClick={onShowAbout}
              className="btn btn-ghost p-2 rounded-lg"
              title="About RunMyModel"
            >
              <Info className="w-4 h-4" />
            </button>
            <a
              href="https://runmymodel.org"
              target="_blank"
              rel="noopener noreferrer"
              className="btn btn-ghost p-2 rounded-lg"
              title="Visit RunMyModel.org"
            >
              <ExternalLink className="w-4 h-4" />
            </a>
          </div>
        </div>
      </div>

      {showModelSelector && (
        <ModelSelector 
          onSelectModel={(model) => {
            setCurrentModel(model.id)
            setShowModelSelector(false)
          }}
          onClose={() => setShowModelSelector(false)}
          currentModel={currentModel || undefined}
        />
      )}
      
      {showSystemPrompts && (
        <SystemPromptSelector onClose={() => setShowSystemPrompts(false)} />
      )}
    </>
  )
}
