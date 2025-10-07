import { create } from 'zustand'
import { persist, createJSONStorage } from 'zustand/middleware'
import { backendService, ChatMessage, ChatRequest } from '../services/backendService'

export interface Chat {
  id: string
  title: string
  messages: ChatMessage[]
  model: string
  createdAt: number
  updatedAt: number
}

export interface ChatState {
  chats: Chat[]
  currentChatId: string | null
  isLoading: boolean
  error: string | null
  addChat: (chat: Omit<Chat, 'id' | 'createdAt' | 'updatedAt'>) => string
  updateChat: (id: string, updates: Partial<Chat>) => void
  deleteChat: (id: string) => void
  setCurrentChat: (id: string | null) => void
  addMessage: (chatId: string, message: Omit<ChatMessage, 'timestamp'>) => void
  updateMessage: (chatId: string, messageIndex: number, updates: Partial<ChatMessage>) => void
  sendMessage: (content: string, model: string) => Promise<void>
  setLoading: (loading: boolean) => void
  setError: (error: string | null) => void
}

export const useChatStore = create<ChatState>()(
  persist(
    (set, get) => ({
      chats: [],
      currentChatId: null,
      isLoading: false,
      error: null,

      addChat: (chatData) => {
        const id = crypto.randomUUID()
        const chat: Chat = {
          ...chatData,
          id,
          createdAt: Date.now(),
          updatedAt: Date.now()
        }
        set(state => ({ 
          chats: [...state.chats, chat],
          currentChatId: id
        }))
        return id
      },

      updateChat: (id, updates) => {
        set(state => ({
          chats: state.chats.map(chat => 
            chat.id === id 
              ? { ...chat, ...updates, updatedAt: Date.now() }
              : chat
          )
        }))
      },

      deleteChat: (id) => {
        set(state => ({
          chats: state.chats.filter(chat => chat.id !== id),
          currentChatId: state.currentChatId === id ? null : state.currentChatId
        }))
      },

      setCurrentChat: (id) => set({ currentChatId: id }),

      addMessage: (chatId, messageData) => {
        const message: ChatMessage = {
          ...messageData,
          timestamp: Date.now()
        }
        set(state => ({
          chats: state.chats.map(chat =>
            chat.id === chatId
              ? { ...chat, messages: [...chat.messages, message], updatedAt: Date.now() }
              : chat
          )
        }))
      },

      updateMessage: (chatId, messageIndex, updates) => {
        set(state => ({
          chats: state.chats.map(chat =>
            chat.id === chatId
              ? {
                  ...chat,
                  messages: chat.messages.map((msg, index) =>
                    index === messageIndex ? { ...msg, ...updates } : msg
                  ),
                  updatedAt: Date.now()
                }
              : chat
          )
        }))
      },

      sendMessage: async (content, model) => {
        const state = get()
        const currentChatId = state.currentChatId

        if (!currentChatId) {
          // Create a new chat
          const chatId = state.addChat({
            title: content.slice(0, 50) + (content.length > 50 ? '...' : ''),
            messages: [],
            model
          })
          set({ currentChatId: chatId })
        }

        const chatId = get().currentChatId!
        
        // Add user message
        state.addMessage(chatId, {
          role: 'user',
          content
        })

        // Add empty assistant message
        const messages = get().chats.find(chat => chat.id === chatId)?.messages || []
        const assistantMessageIndex = messages.length
        state.addMessage(chatId, {
          role: 'assistant',
          content: ''
        })

        set({ isLoading: true, error: null })

        try {
          // Prepare messages for Hugging Face
          const chatMessages: ChatMessage[] = [
            ...messages,
            { role: 'user', content, timestamp: Date.now() }
          ]

          const request: ChatRequest = {
            model,
            messages: chatMessages,
            temperature: 0.7,
            max_tokens: 1000
          }

          const response = await backendService.chat(request)
          
          // Update the assistant message with the response
          state.updateMessage(chatId, assistantMessageIndex, {
            content: response.content
          })

          set({ isLoading: false })
        } catch (error) {
          console.error('Error sending message:', error)
          state.updateMessage(chatId, assistantMessageIndex, {
            content: `Error: ${error instanceof Error ? error.message : 'Failed to send message'}`
          })
          set({ 
            isLoading: false, 
            error: error instanceof Error ? error.message : 'Failed to send message'
          })
        }
      },

      setLoading: (loading) => set({ isLoading: loading }),
      setError: (error) => set({ error })
    }),
    {
      name: 'chat-store',
      storage: createJSONStorage(() => localStorage),
      partialize: (state) => ({ 
        chats: state.chats,
        currentChatId: state.currentChatId
      })
    }
  )
)

// Computed selectors
export const useCurrentChat = () => {
  const { chats, currentChatId } = useChatStore()
  return chats.find(chat => chat.id === currentChatId) || null
}