import React from 'react'
import { X, ExternalLink, Github, Heart } from 'lucide-react'

interface AboutModalProps {
  onClose: () => void
}

export const AboutModal: React.FC<AboutModalProps> = ({ onClose }) => {
  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
      <div className="bg-white dark:bg-gray-800 rounded-lg shadow-xl w-full max-w-2xl max-h-[80vh] flex flex-col">
        {/* Header */}
        <div className="flex items-center justify-between p-6 border-b border-gray-200 dark:border-gray-700">
          <h2 className="text-xl font-semibold text-gray-900 dark:text-white">
            About RunMyModel Desktop
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
          <div className="text-center mb-8">
            <div className="w-16 h-16 bg-gradient-to-br from-blue-500 to-blue-600 rounded-2xl flex items-center justify-center mx-auto mb-4">
              <span className="text-2xl font-bold text-white">RM</span>
            </div>
            <h3 className="text-2xl font-bold text-gray-900 dark:text-white mb-2">
              RunMyModel Desktop
            </h3>
            <p className="text-gray-600 dark:text-gray-400">
              Version 1.0.0
            </p>
          </div>

          <div className="space-y-6">
            <div>
              <h4 className="text-lg font-semibold text-gray-900 dark:text-white mb-3">
                About
              </h4>
              <p className="text-gray-600 dark:text-gray-400 leading-relaxed">
                RunMyModel Desktop is a powerful desktop application that allows you to run large language models 
                locally on your machine using Ollama. Chat with models like LLaMA, Mistral, CodeLlama, and more 
                in a beautiful ChatGPT-style interface.
              </p>
            </div>

            <div>
              <h4 className="text-lg font-semibold text-gray-900 dark:text-white mb-3">
                Features
              </h4>
              <ul className="space-y-2 text-gray-600 dark:text-gray-400">
                <li className="flex items-start space-x-2">
                  <span className="text-blue-600 dark:text-blue-400 mt-1">•</span>
                  <span>Run LLMs locally with Ollama integration</span>
                </li>
                <li className="flex items-start space-x-2">
                  <span className="text-blue-600 dark:text-blue-400 mt-1">•</span>
                  <span>ChatGPT-style interface with markdown support</span>
                </li>
                <li className="flex items-start space-x-2">
                  <span className="text-blue-600 dark:text-blue-400 mt-1">•</span>
                  <span>Model management and download progress tracking</span>
                </li>
                <li className="flex items-start space-x-2">
                  <span className="text-blue-600 dark:text-blue-400 mt-1">•</span>
                  <span>System prompts for different use cases</span>
                </li>
                <li className="flex items-start space-x-2">
                  <span className="text-blue-600 dark:text-blue-400 mt-1">•</span>
                  <span>Dark/light theme support</span>
                </li>
                <li className="flex items-start space-x-2">
                  <span className="text-blue-600 dark:text-blue-400 mt-1">•</span>
                  <span>Cross-platform support (Windows, macOS, Linux)</span>
                </li>
              </ul>
            </div>

            <div>
              <h4 className="text-lg font-semibold text-gray-900 dark:text-white mb-3">
                Technology Stack
              </h4>
              <div className="grid grid-cols-2 gap-3 text-sm">
                <div className="bg-gray-50 dark:bg-gray-700 p-3 rounded-lg">
                  <div className="font-medium text-gray-900 dark:text-white">Frontend</div>
                  <div className="text-gray-600 dark:text-gray-400">React + TypeScript + Tailwind CSS</div>
                </div>
                <div className="bg-gray-50 dark:bg-gray-700 p-3 rounded-lg">
                  <div className="font-medium text-gray-900 dark:text-white">Backend</div>
                  <div className="text-gray-600 dark:text-gray-400">Tauri + Rust</div>
                </div>
                <div className="bg-gray-50 dark:bg-gray-700 p-3 rounded-lg">
                  <div className="font-medium text-gray-900 dark:text-white">LLM Engine</div>
                  <div className="text-gray-600 dark:text-gray-400">Ollama CLI</div>
                </div>
                <div className="bg-gray-50 dark:bg-gray-700 p-3 rounded-lg">
                  <div className="font-medium text-gray-900 dark:text-white">State Management</div>
                  <div className="text-gray-600 dark:text-gray-400">Zustand</div>
                </div>
              </div>
            </div>

            <div>
              <h4 className="text-lg font-semibold text-gray-900 dark:text-white mb-3">
                Links
              </h4>
              <div className="space-y-3">
                <a
                  href="https://runmymodel.org"
                  target="_blank"
                  rel="noopener noreferrer"
                  className="flex items-center space-x-3 p-3 bg-gray-50 dark:bg-gray-700 rounded-lg hover:bg-gray-100 dark:hover:bg-gray-600 transition-colors"
                >
                  <ExternalLink className="w-5 h-5 text-blue-600 dark:text-blue-400" />
                  <div>
                    <div className="font-medium text-gray-900 dark:text-white">RunMyModel.org</div>
                    <div className="text-sm text-gray-600 dark:text-gray-400">Visit our website</div>
                  </div>
                </a>
                <a
                  href="https://github.com/runmymodel/runmymodel-desktop"
                  target="_blank"
                  rel="noopener noreferrer"
                  className="flex items-center space-x-3 p-3 bg-gray-50 dark:bg-gray-700 rounded-lg hover:bg-gray-100 dark:hover:bg-gray-600 transition-colors"
                >
                  <Github className="w-5 h-5 text-gray-600 dark:text-gray-400" />
                  <div>
                    <div className="font-medium text-gray-900 dark:text-white">GitHub Repository</div>
                    <div className="text-sm text-gray-600 dark:text-gray-400">Source code and issues</div>
                  </div>
                </a>
                <a
                  href="https://ollama.ai"
                  target="_blank"
                  rel="noopener noreferrer"
                  className="flex items-center space-x-3 p-3 bg-gray-50 dark:bg-gray-700 rounded-lg hover:bg-gray-100 dark:hover:bg-gray-600 transition-colors"
                >
                  <ExternalLink className="w-5 h-5 text-green-600 dark:text-green-400" />
                  <div>
                    <div className="font-medium text-gray-900 dark:text-white">Ollama</div>
                    <div className="text-sm text-gray-600 dark:text-gray-400">Learn more about Ollama</div>
                  </div>
                </a>
              </div>
            </div>
          </div>
        </div>

        {/* Footer */}
        <div className="p-6 border-t border-gray-200 dark:border-gray-700">
          <div className="flex items-center justify-between">
            <div className="flex items-center space-x-2 text-sm text-gray-500 dark:text-gray-400">
              <span>Made with</span>
              <Heart className="w-4 h-4 text-red-500" />
              <span>by RunMyModel.org</span>
            </div>
            <button
              onClick={onClose}
              className="btn btn-primary"
            >
              Close
            </button>
          </div>
        </div>
      </div>
    </div>
  )
}
