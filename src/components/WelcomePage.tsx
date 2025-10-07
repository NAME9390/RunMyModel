import React from 'react'
import { 
  MessageSquare, 
  Zap, 
  Shield, 
  Globe, 
  Code, 
  Brain, 
  Sparkles,
  ArrowRight,
  Star,
  CheckCircle
} from 'lucide-react'

interface WelcomePageProps {
  onGetStarted: () => void
}

export const WelcomePage: React.FC<WelcomePageProps> = ({ onGetStarted }) => {
  const features = [
    {
      icon: Brain,
      title: '50+ AI Models',
      description: 'Access to the largest collection of open-source AI models from Hugging Face'
    },
    {
      icon: Zap,
      title: 'Lightning Fast',
      description: 'Optimized for speed with cloud-based inference and local caching'
    },
    {
      icon: Shield,
      title: 'Privacy First',
      description: 'Your conversations stay private with local storage and optional cloud sync'
    },
    {
      icon: Code,
      title: 'Code Generation',
      description: 'Specialized models for programming, debugging, and code completion'
    },
    {
      icon: Globe,
      title: 'Multilingual',
      description: 'Support for 100+ languages with translation and multilingual models'
    },
    {
      icon: Sparkles,
      title: 'Custom Models',
      description: 'Upload and use your own models with our custom model support'
    }
  ]

  const categories = [
    { name: 'Text Generation', count: 15, color: 'bg-blue-500' },
    { name: 'Code Generation', count: 5, color: 'bg-green-500' },
    { name: 'Question Answering', count: 6, color: 'bg-purple-500' },
    { name: 'Summarization', count: 8, color: 'bg-orange-500' },
    { name: 'Translation', count: 8, color: 'bg-pink-500' },
    { name: 'Sentiment Analysis', count: 3, color: 'bg-indigo-500' },
    { name: 'Named Entity Recognition', count: 3, color: 'bg-red-500' },
    { name: 'Text Classification', count: 4, color: 'bg-yellow-500' }
  ]

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-white to-purple-50 dark:from-gray-900 dark:via-gray-800 dark:to-gray-900">
      {/* Hero Section */}
      <div className="container mx-auto px-6 py-16">
        <div className="text-center mb-16">
          <div className="inline-flex items-center justify-center w-20 h-20 bg-gradient-to-r from-blue-600 to-purple-600 rounded-2xl mb-8">
            <Brain className="w-10 h-10 text-white" />
          </div>
          
          <h1 className="text-5xl font-bold text-gray-900 dark:text-white mb-6">
            Welcome to{' '}
            <span className="bg-gradient-to-r from-blue-600 to-purple-600 bg-clip-text text-transparent">
              RunMyModel Desktop
            </span>
          </h1>
          
          <p className="text-xl text-gray-600 dark:text-gray-300 mb-8 max-w-3xl mx-auto">
            Experience the power of AI with the most comprehensive collection of open-source models. 
            Chat, code, translate, analyze, and create with state-of-the-art AI models from Hugging Face.
          </p>
          
          <div className="flex flex-col sm:flex-row gap-4 justify-center items-center mb-12">
            <button
              onClick={onGetStarted}
              className="bg-gradient-to-r from-blue-600 to-purple-600 text-white px-8 py-4 rounded-xl font-semibold text-lg hover:from-blue-700 hover:to-purple-700 transition-all duration-200 transform hover:scale-105 shadow-lg hover:shadow-xl flex items-center gap-2"
            >
              Get Started
              <ArrowRight className="w-5 h-5" />
            </button>
            
            <div className="flex items-center gap-2 text-gray-600 dark:text-gray-400">
              <Star className="w-5 h-5 text-yellow-500 fill-current" />
              <span className="font-medium">50+ Models Available</span>
            </div>
          </div>
        </div>

        {/* Stats */}
        <div className="grid grid-cols-2 md:grid-cols-4 gap-6 mb-16">
          <div className="bg-white dark:bg-gray-800 rounded-xl p-6 text-center shadow-lg">
            <div className="text-3xl font-bold text-blue-600 dark:text-blue-400 mb-2">50+</div>
            <div className="text-gray-600 dark:text-gray-400">AI Models</div>
          </div>
          <div className="bg-white dark:bg-gray-800 rounded-xl p-6 text-center shadow-lg">
            <div className="text-3xl font-bold text-green-600 dark:text-green-400 mb-2">100+</div>
            <div className="text-gray-600 dark:text-gray-400">Languages</div>
          </div>
          <div className="bg-white dark:bg-gray-800 rounded-xl p-6 text-center shadow-lg">
            <div className="text-3xl font-bold text-purple-600 dark:text-purple-400 mb-2">8</div>
            <div className="text-gray-600 dark:text-gray-400">Categories</div>
          </div>
          <div className="bg-white dark:bg-gray-800 rounded-xl p-6 text-center shadow-lg">
            <div className="text-3xl font-bold text-orange-600 dark:text-orange-400 mb-2">∞</div>
            <div className="text-gray-600 dark:text-gray-400">Possibilities</div>
          </div>
        </div>

        {/* Features Grid */}
        <div className="mb-16">
          <h2 className="text-3xl font-bold text-center text-gray-900 dark:text-white mb-12">
            Why Choose RunMyModel Desktop?
          </h2>
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-8">
            {features.map((feature, index) => (
              <div
                key={index}
                className="bg-white dark:bg-gray-800 rounded-xl p-6 shadow-lg hover:shadow-xl transition-all duration-200 transform hover:scale-105"
              >
                <div className="w-12 h-12 bg-gradient-to-r from-blue-500 to-purple-500 rounded-lg flex items-center justify-center mb-4">
                  <feature.icon className="w-6 h-6 text-white" />
                </div>
                <h3 className="text-xl font-semibold text-gray-900 dark:text-white mb-2">
                  {feature.title}
                </h3>
                <p className="text-gray-600 dark:text-gray-400">
                  {feature.description}
                </p>
              </div>
            ))}
          </div>
        </div>

        {/* Model Categories */}
        <div className="mb-16">
          <h2 className="text-3xl font-bold text-center text-gray-900 dark:text-white mb-12">
            Explore AI Model Categories
          </h2>
          <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
            {categories.map((category, index) => (
              <div
                key={index}
                className="bg-white dark:bg-gray-800 rounded-xl p-6 shadow-lg hover:shadow-xl transition-all duration-200 transform hover:scale-105 cursor-pointer"
              >
                <div className={`w-8 h-8 ${category.color} rounded-lg mb-3`}></div>
                <h3 className="font-semibold text-gray-900 dark:text-white mb-1">
                  {category.name}
                </h3>
                <p className="text-sm text-gray-600 dark:text-gray-400">
                  {category.count} models
                </p>
              </div>
            ))}
          </div>
        </div>

        {/* Popular Models Preview */}
        <div className="mb-16">
          <h2 className="text-3xl font-bold text-center text-gray-900 dark:text-white mb-12">
            Popular Models
          </h2>
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
            {[
              { name: 'DialoGPT Medium', author: 'Microsoft', downloads: '1M+', category: 'Conversational AI' },
              { name: 'GPT-Neo 1.3B', author: 'EleutherAI', downloads: '1.2M+', category: 'Text Generation' },
              { name: 'CodeGen 2B', author: 'Salesforce', downloads: '600K+', category: 'Code Generation' },
              { name: 'BLOOM 7B1', author: 'BigScience', downloads: '150K+', category: 'Multilingual' },
              { name: 'FLAN-T5 Large', author: 'Google', downloads: '1M+', category: 'Instruction Following' },
              { name: 'StarCoder', author: 'BigCode', downloads: '300K+', category: 'Code Generation' }
            ].map((model, index) => (
              <div
                key={index}
                className="bg-white dark:bg-gray-800 rounded-xl p-6 shadow-lg hover:shadow-xl transition-all duration-200"
              >
                <div className="flex items-center justify-between mb-3">
                  <h3 className="font-semibold text-gray-900 dark:text-white">
                    {model.name}
                  </h3>
                  <div className="flex items-center gap-1 text-yellow-500">
                    <Star className="w-4 h-4 fill-current" />
                    <span className="text-sm font-medium">4.8</span>
                  </div>
                </div>
                <p className="text-sm text-gray-600 dark:text-gray-400 mb-2">
                  by {model.author}
                </p>
                <div className="flex items-center justify-between">
                  <span className="text-xs bg-blue-100 dark:bg-blue-900 text-blue-800 dark:text-blue-200 px-2 py-1 rounded">
                    {model.category}
                  </span>
                  <span className="text-sm text-gray-500 dark:text-gray-400">
                    {model.downloads} downloads
                  </span>
                </div>
              </div>
            ))}
          </div>
        </div>

        {/* CTA Section */}
        <div className="bg-gradient-to-r from-blue-600 to-purple-600 rounded-2xl p-12 text-center text-white">
          <h2 className="text-3xl font-bold mb-4">
            Ready to Start Your AI Journey?
          </h2>
          <p className="text-xl mb-8 opacity-90">
            Join thousands of users who are already exploring the future of AI
          </p>
          <button
            onClick={onGetStarted}
            className="bg-white text-blue-600 px-8 py-4 rounded-xl font-semibold text-lg hover:bg-gray-100 transition-all duration-200 transform hover:scale-105 shadow-lg hover:shadow-xl flex items-center gap-2 mx-auto"
          >
            Start Chatting with AI
            <MessageSquare className="w-5 h-5" />
          </button>
        </div>

        {/* Footer */}
        <div className="text-center mt-16 text-gray-600 dark:text-gray-400">
          <p className="mb-4">
            Powered by <span className="font-semibold text-blue-600 dark:text-blue-400">Hugging Face</span> • 
            Built with <span className="font-semibold text-purple-600 dark:text-purple-400">Tauri</span> • 
            Designed for <span className="font-semibold text-green-600 dark:text-green-400">Everyone</span>
          </p>
          <div className="flex items-center justify-center gap-6 text-sm">
            <div className="flex items-center gap-2">
              <CheckCircle className="w-4 h-4 text-green-500" />
              <span>Open Source</span>
            </div>
            <div className="flex items-center gap-2">
              <CheckCircle className="w-4 h-4 text-green-500" />
              <span>Privacy Focused</span>
            </div>
            <div className="flex items-center gap-2">
              <CheckCircle className="w-4 h-4 text-green-500" />
              <span>Cross Platform</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}
