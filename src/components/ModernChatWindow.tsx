/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 RunMyModel.org
 */

import React, { useState, useRef, useEffect } from 'react'
import { Send, Bot, User, Copy, Check } from 'lucide-react'
import { Button } from './ui/button'
import { Textarea } from './ui/textarea'
import { Card, CardContent } from './ui/card'
import { useChatStore, useCurrentChat } from '../store/chatStore'
import { useModelStore } from '../store/modelStore'
import ReactMarkdown from 'react-markdown'
import { Prism as SyntaxHighlighter } from 'react-syntax-highlighter'
import { oneDark } from 'react-syntax-highlighter/dist/esm/styles/prism'

interface MessageProps {
  message: {
    role: 'user' | 'assistant'
    content: string
    timestamp: number
  }
  index: number
  onCopy: (text: string, index: number) => void
  copiedMessageId: string | null
}

const Message: React.FC<MessageProps> = ({ message, index, onCopy, copiedMessageId }) => {
  const isCopied = copiedMessageId === index.toString()

  return (
    <div className="group relative">
      <div className={`flex gap-4 p-4 ${message.role === 'user' ? 'bg-gray-50 dark:bg-gray-800/50' : ''}`}>
        <div className={`flex-shrink-0 w-8 h-8 rounded-full flex items-center justify-center ${
          message.role === 'user' 
            ? 'bg-blue-500 text-white' 
            : 'bg-gray-200 dark:bg-gray-700 text-gray-600 dark:text-gray-300'
        }`}>
          {message.role === 'user' ? <User size={16} /> : <Bot size={16} />}
        </div>
        
        <div className="flex-1 min-w-0">
          <div className="prose prose-sm max-w-none dark:prose-invert">
            <ReactMarkdown
              components={{
                code({ node, className, children, ...props }: any) {
                  const match = /language-(\w+)/.exec(className || '')
                  return match ? (
                    <SyntaxHighlighter
                      style={oneDark as any}
                      language={match[1]}
                      PreTag="div"
                      {...props}
                    >
                      {String(children).replace(/\n$/, '')}
                    </SyntaxHighlighter>
                  ) : (
                    <code className={className} {...props}>
                      {children}
                    </code>
                  )
                },
              }}
            >
              {message.content}
            </ReactMarkdown>
          </div>
        </div>

        <div className="flex-shrink-0 opacity-0 group-hover:opacity-100 transition-opacity">
          <Button
            variant="ghost"
            size="icon"
            onClick={() => onCopy(message.content, index)}
            className="h-8 w-8"
          >
            {isCopied ? <Check size={14} /> : <Copy size={14} />}
          </Button>
        </div>
      </div>
    </div>
  )
}

export const ModernChatWindow: React.FC = () => {
  const [input, setInput] = useState('')
  const [copiedMessageId, setCopiedMessageId] = useState<string | null>(null)
  const messagesEndRef = useRef<HTMLDivElement>(null)
  const textareaRef = useRef<HTMLTextAreaElement>(null)
  
  const { sendMessage, isLoading } = useChatStore()
  const currentChat = useCurrentChat()
  const { currentModel } = useModelStore()

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' })
  }

  useEffect(() => {
    scrollToBottom()
  }, [currentChat?.messages])

  const handleSend = async () => {
    if (!input.trim() || !currentModel || isLoading) return

    const messageToSend = input.trim()
    setInput('')
    
    try {
      await sendMessage(messageToSend, currentModel)
    } catch (error) {
      console.error('Error sending message:', error)
    }
  }

  const handleKeyPress = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault()
      handleSend()
    }
  }

  const copyToClipboard = async (text: string, messageIndex: number) => {
    try {
      await navigator.clipboard.writeText(text)
      setCopiedMessageId(messageIndex.toString())
      setTimeout(() => setCopiedMessageId(null), 2000)
    } catch (error) {
      console.error('Failed to copy text:', error)
    }
  }

  if (!currentChat) {
    return (
      <div className="flex-1 flex items-center justify-center bg-gray-50 dark:bg-gray-900">
        <Card className="w-full max-w-2xl mx-4">
          <CardContent className="p-8 text-center">
            <Bot size={48} className="mx-auto mb-4 text-gray-400" />
            <h2 className="text-2xl font-semibold mb-2">Welcome to RunMyModel Desktop</h2>
            <p className="text-gray-600 dark:text-gray-400 mb-6">
              Start a conversation by typing a message below. Select a model from the sidebar to begin.
            </p>
            <div className="text-sm text-gray-500 dark:text-gray-500">
              <p>• Powered by Hugging Face models</p>
              <p>• Run locally on your machine</p>
              <p>• Privacy-focused AI chat</p>
            </div>
          </CardContent>
        </Card>
      </div>
    )
  }

  return (
    <div className="flex-1 flex flex-col bg-white dark:bg-gray-900">
      {/* Messages */}
      <div className="flex-1 overflow-y-auto">
        {currentChat.messages.length === 0 ? (
          <div className="flex items-center justify-center h-full">
            <div className="text-center">
              <Bot size={48} className="mx-auto mb-4 text-gray-400" />
              <h3 className="text-lg font-medium mb-2">Start a conversation</h3>
              <p className="text-gray-600 dark:text-gray-400">
                Type a message below to begin chatting with {currentModel}
              </p>
            </div>
          </div>
        ) : (
          <div className="max-w-4xl mx-auto">
            {currentChat.messages.map((message: any, index: number) => (
              <Message
                key={index}
                message={message}
                index={index}
                onCopy={copyToClipboard}
                copiedMessageId={copiedMessageId}
              />
            ))}
            {isLoading && (
              <div className="flex gap-4 p-4">
                <div className="flex-shrink-0 w-8 h-8 rounded-full bg-gray-200 dark:bg-gray-700 flex items-center justify-center">
                  <Bot size={16} className="text-gray-600 dark:text-gray-300" />
                </div>
                <div className="flex-1">
                  <div className="flex items-center space-x-2">
                    <div className="w-2 h-2 bg-gray-400 rounded-full animate-bounce"></div>
                    <div className="w-2 h-2 bg-gray-400 rounded-full animate-bounce" style={{ animationDelay: '0.1s' }}></div>
                    <div className="w-2 h-2 bg-gray-400 rounded-full animate-bounce" style={{ animationDelay: '0.2s' }}></div>
                  </div>
                </div>
              </div>
            )}
            <div ref={messagesEndRef} />
          </div>
        )}
      </div>

      {/* Input */}
      <div className="border-t bg-white dark:bg-gray-900 p-4">
        <div className="max-w-4xl mx-auto">
          <div className="flex gap-2">
            <Textarea
              ref={textareaRef}
              value={input}
              onChange={(e) => setInput(e.target.value)}
              onKeyDown={handleKeyPress}
              placeholder={`Message ${currentModel}...`}
              className="flex-1 min-h-[60px] max-h-[200px] resize-none"
              disabled={isLoading}
            />
            <Button
              onClick={handleSend}
              disabled={!input.trim() || !currentModel || isLoading}
              size="icon"
              className="h-[60px] w-[60px]"
            >
              <Send size={20} />
            </Button>
          </div>
          <div className="flex justify-between items-center mt-2 text-xs text-gray-500">
            <span>Press Enter to send, Shift+Enter for new line</span>
            <span>{currentModel}</span>
          </div>
        </div>
      </div>
    </div>
  )
}
