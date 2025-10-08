/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 RunMyModel.org
 */

import React, { useState } from 'react'
import { 
  MessageSquare, 
  Plus, 
  Settings, 
  Bot, 
  Trash2, 
  ChevronRight,
  ChevronDown,
  Folder,
  FolderOpen
} from 'lucide-react'
import { Button } from './ui/button'
import { Card, CardContent } from './ui/card'
import { useChatStore } from '../store/chatStore'
import { useModelStore } from '../store/modelStore'
import { useAppStore } from '../store/appStore'

interface SidebarProps {
  onShowAbout: () => void
  onShowModels: () => void
  onShowCustomModels: () => void
  onShowModelBrowser: () => void
}

export const ModernSidebar: React.FC<SidebarProps> = ({
  onShowAbout,
  onShowModels,
  onShowCustomModels,
  onShowModelBrowser
}) => {
  const [expandedFolders, setExpandedFolders] = useState<Set<string>>(new Set())
  const { chats, currentChatId, addChat, setCurrentChat, deleteChat } = useChatStore()
  const { currentModel } = useModelStore()
  const { theme, setTheme } = useAppStore()

  const handleNewChat = () => {
    const chatId = addChat({
      title: 'New Chat',
      messages: [],
      model: currentModel || 'default'
    })
    setCurrentChat(chatId)
  }

  const toggleFolder = (folderId: string) => {
    const newExpanded = new Set(expandedFolders)
    if (newExpanded.has(folderId)) {
      newExpanded.delete(folderId)
    } else {
      newExpanded.add(folderId)
    }
    setExpandedFolders(newExpanded)
  }

  const groupedChats = chats.reduce((acc, chat) => {
    const folder = chat.model || 'default'
    if (!acc[folder]) {
      acc[folder] = []
    }
    acc[folder].push(chat)
    return acc
  }, {} as Record<string, typeof chats>)

  return (
    <div className="w-80 bg-gray-50 dark:bg-gray-900 border-r border-gray-200 dark:border-gray-700 flex flex-col h-full">
      {/* Header */}
      <div className="p-4 border-b border-gray-200 dark:border-gray-700">
        <div className="flex items-center justify-between mb-4">
          <h1 className="text-xl font-semibold text-gray-900 dark:text-white">
            RunMyModel Desktop
          </h1>
          <Button
            variant="ghost"
            size="icon"
            onClick={handleNewChat}
            className="h-8 w-8"
          >
            <Plus size={16} />
          </Button>
        </div>
        
        {/* Model Selector */}
        <Card className="mb-4">
          <CardContent className="p-3">
            <div className="flex items-center gap-2">
              <Bot size={16} className="text-gray-600 dark:text-gray-400" />
              <span className="text-sm font-medium text-gray-900 dark:text-white">
                Current Model
              </span>
            </div>
            <div className="mt-2">
              <div className="text-sm text-gray-600 dark:text-gray-400 truncate">
                {currentModel || 'No model selected'}
              </div>
              <Button
                variant="outline"
                size="sm"
                className="w-full mt-2"
                onClick={onShowModels}
              >
                Change Model
              </Button>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Chat List */}
      <div className="flex-1 overflow-y-auto p-4">
        <div className="space-y-2">
          {Object.entries(groupedChats).map(([folderName, folderChats]) => {
            const isExpanded = expandedFolders.has(folderName)
            return (
              <div key={folderName}>
                <button
                  onClick={() => toggleFolder(folderName)}
                  className="flex items-center gap-2 w-full p-2 text-left hover:bg-gray-100 dark:hover:bg-gray-800 rounded-md transition-colors"
                >
                  {isExpanded ? <ChevronDown size={16} /> : <ChevronRight size={16} />}
                  {isExpanded ? <FolderOpen size={16} /> : <Folder size={16} />}
                  <span className="text-sm font-medium text-gray-700 dark:text-gray-300">
                    {folderName}
                  </span>
                  <span className="text-xs text-gray-500 ml-auto">
                    {folderChats.length}
                  </span>
                </button>
                
                {isExpanded && (
                  <div className="ml-6 space-y-1">
                    {folderChats.map((chat) => (
                      <div
                        key={chat.id}
                        className={`flex items-center gap-2 p-2 rounded-md cursor-pointer transition-colors group ${
                          currentChatId === chat.id
                            ? 'bg-blue-100 dark:bg-blue-900/30 text-blue-900 dark:text-blue-100'
                            : 'hover:bg-gray-100 dark:hover:bg-gray-800'
                        }`}
                        onClick={() => setCurrentChat(chat.id)}
                      >
                        <MessageSquare size={14} className="text-gray-500" />
                        <span className="text-sm flex-1 truncate">
                          {chat.title}
                        </span>
                        <div className="opacity-0 group-hover:opacity-100 transition-opacity">
                          <Button
                            variant="ghost"
                            size="icon"
                            className="h-6 w-6"
                            onClick={(e) => {
                              e.stopPropagation()
                              deleteChat(chat.id)
                            }}
                          >
                            <Trash2 size={12} />
                          </Button>
                        </div>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            )
          })}
          
          {chats.length === 0 && (
            <div className="text-center py-8">
              <MessageSquare size={32} className="mx-auto mb-2 text-gray-400" />
              <p className="text-sm text-gray-500 dark:text-gray-400">
                No conversations yet
              </p>
              <Button
                variant="outline"
                size="sm"
                className="mt-2"
                onClick={handleNewChat}
              >
                Start New Chat
              </Button>
            </div>
          )}
        </div>
      </div>

      {/* Footer */}
      <div className="p-4 border-t border-gray-200 dark:border-gray-700 space-y-2">
        <Button
          variant="ghost"
          className="w-full justify-start"
          onClick={onShowModelBrowser}
        >
          <Bot size={16} className="mr-2" />
          Browse Models
        </Button>
        
        <Button
          variant="ghost"
          className="w-full justify-start"
          onClick={onShowCustomModels}
        >
          <Settings size={16} className="mr-2" />
          Custom Models
        </Button>
        
        <Button
          variant="ghost"
          className="w-full justify-start"
          onClick={onShowAbout}
        >
          <Settings size={16} className="mr-2" />
          About
        </Button>
        
        <div className="flex items-center justify-between pt-2">
          <span className="text-xs text-gray-500">Theme</span>
          <Button
            variant="outline"
            size="sm"
            onClick={() => setTheme(theme === 'dark' ? 'light' : 'dark')}
          >
            {theme === 'dark' ? '☀️' : '🌙'}
          </Button>
        </div>
      </div>
    </div>
  )
}
