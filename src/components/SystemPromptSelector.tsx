import React, { useState } from 'react'
import { X, Wand2, Code, Lightbulb, Search, BookOpen, Check } from 'lucide-react'
import { useChatStore, useCurrentChat } from '../store/chatStore'

interface SystemPromptSelectorProps {
  onClose: () => void
}

const systemPrompts = [
  {
    id: 'creative',
    name: 'Creative Writer',
    description: 'Helps with creative writing, storytelling, and artistic expression',
    content: 'You are a creative writing assistant. Help users with storytelling, character development, plot ideas, and creative expression. Be imaginative and inspiring.',
    icon: Wand2,
    category: 'creative' as const,
  },
  {
    id: 'coder',
    name: 'Code Assistant',
    description: 'Specialized in programming, debugging, and software development',
    content: 'You are a programming assistant. Help users write, debug, and understand code. Provide clear explanations and best practices. Support multiple programming languages.',
    icon: Code,
    category: 'coding' as const,
  },
  {
    id: 'analyst',
    name: 'Data Analyst',
    description: 'Analyzes data, creates insights, and helps with research',
    content: 'You are a data analysis assistant. Help users analyze data, create insights, and conduct research. Be thorough, analytical, and evidence-based.',
    icon: Search,
    category: 'analysis' as const,
  },
  {
    id: 'researcher',
    name: 'Research Assistant',
    description: 'Helps with academic research, fact-checking, and knowledge synthesis',
    content: 'You are a research assistant. Help users with academic research, fact-checking, and knowledge synthesis. Be accurate, thorough, and cite sources when possible.',
    icon: BookOpen,
    category: 'research' as const,
  },
  {
    id: 'general',
    name: 'General Assistant',
    description: 'A helpful, harmless, and honest AI assistant',
    content: 'You are a helpful AI assistant. Be honest, harmless, and helpful. Provide accurate information and assist users with their questions and tasks.',
    icon: Lightbulb,
    category: 'general' as const,
  },
]

export const SystemPromptSelector: React.FC<SystemPromptSelectorProps> = ({ onClose }) => {
  const [selectedPrompt, setSelectedPrompt] = useState<string | null>(null)
  const [customPrompt, setCustomPrompt] = useState('')
  const { updateChat } = useChatStore()
  const currentChat = useCurrentChat()

  const handleApplyPrompt = () => {
    if (!currentChat) return

    const prompt = selectedPrompt 
      ? systemPrompts.find(p => p.id === selectedPrompt)?.content || customPrompt
      : customPrompt

    if (prompt) {
      // Add system message to the chat
      updateChat(currentChat.id, {
        messages: [
          { role: 'system', content: prompt, timestamp: Date.now() },
          ...currentChat.messages
        ]
      })
      onClose()
    }
  }

  const handleCustomPrompt = () => {
    setSelectedPrompt(null)
  }

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
      <div className="bg-white dark:bg-gray-800 rounded-lg shadow-xl w-full max-w-2xl max-h-[80vh] flex flex-col">
        {/* Header */}
        <div className="flex items-center justify-between p-6 border-b border-gray-200 dark:border-gray-700">
          <h2 className="text-xl font-semibold text-gray-900 dark:text-white">
            System Prompts
          </h2>
          <button
            onClick={onClose}
            className="p-2 hover:bg-gray-100 dark:hover:bg-gray-700 rounded-lg transition-colors"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Content */}
        <div className="flex-1 overflow-y-auto p-6">
          <div className="mb-6">
            <h3 className="text-lg font-medium text-gray-900 dark:text-white mb-4">
              Choose a System Prompt
            </h3>
            <div className="grid gap-3">
              {systemPrompts.map((prompt) => {
                const Icon = prompt.icon
                return (
                  <div
                    key={prompt.id}
                    className={`card p-4 cursor-pointer transition-colors ${
                      selectedPrompt === prompt.id
                        ? 'ring-2 ring-blue-500 bg-blue-50 dark:bg-blue-900/20'
                        : 'hover:bg-gray-50 dark:hover:bg-gray-700'
                    }`}
                    onClick={() => setSelectedPrompt(prompt.id)}
                  >
                    <div className="flex items-start space-x-3">
                      <div className="w-10 h-10 bg-gradient-to-br from-blue-500 to-blue-600 rounded-lg flex items-center justify-center flex-shrink-0">
                        <Icon className="w-5 h-5 text-white" />
                      </div>
                      <div className="flex-1">
                        <div className="flex items-center space-x-2">
                          <h4 className="font-medium text-gray-900 dark:text-white">
                            {prompt.name}
                          </h4>
                          {selectedPrompt === prompt.id && (
                            <Check className="w-4 h-4 text-blue-600" />
                          )}
                        </div>
                        <p className="text-sm text-gray-600 dark:text-gray-400 mt-1">
                          {prompt.description}
                        </p>
                        <div className="text-xs text-gray-500 dark:text-gray-500 mt-2">
                          {prompt.content.slice(0, 100)}...
                        </div>
                      </div>
                    </div>
                  </div>
                )
              })}
            </div>
          </div>

          {/* Custom Prompt */}
          <div>
            <h3 className="text-lg font-medium text-gray-900 dark:text-white mb-4">
              Custom System Prompt
            </h3>
            <div className="space-y-3">
              <button
                onClick={handleCustomPrompt}
                className={`w-full card p-4 text-left transition-colors ${
                  selectedPrompt === null
                    ? 'ring-2 ring-blue-500 bg-blue-50 dark:bg-blue-900/20'
                    : 'hover:bg-gray-50 dark:hover:bg-gray-700'
                }`}
              >
                <h4 className="font-medium text-gray-900 dark:text-white">
                  Write your own prompt
                </h4>
                <p className="text-sm text-gray-600 dark:text-gray-400 mt-1">
                  Create a custom system prompt for your specific needs
                </p>
              </button>
              
              <textarea
                value={customPrompt}
                onChange={(e) => setCustomPrompt(e.target.value)}
                placeholder="Enter your custom system prompt here..."
                className="w-full input min-h-[120px] resize-none"
                rows={5}
              />
            </div>
          </div>
        </div>

        {/* Footer */}
        <div className="p-6 border-t border-gray-200 dark:border-gray-700">
          <div className="flex items-center justify-between">
            <div className="text-sm text-gray-500 dark:text-gray-400">
              System prompts help define the AI's behavior and personality
            </div>
            <div className="flex space-x-3">
              <button
                onClick={onClose}
                className="btn btn-secondary"
              >
                Cancel
              </button>
              <button
                onClick={handleApplyPrompt}
                className="btn btn-primary"
                disabled={!selectedPrompt && !customPrompt.trim()}
              >
                Apply Prompt
              </button>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}
