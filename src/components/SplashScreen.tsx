import React, { useState, useEffect } from 'react'
import { Loader2 } from 'lucide-react'

export const SplashScreen: React.FC = () => {
  const [progress, setProgress] = useState(0)
  const [status, setStatus] = useState('Initializing...')

  useEffect(() => {
    const steps = [
      { progress: 20, status: 'Loading Ollama...' },
      { progress: 40, status: 'Checking system info...' },
      { progress: 60, status: 'Loading models...' },
      { progress: 80, status: 'Preparing interface...' },
      { progress: 100, status: 'Ready!' },
    ]

    let currentStep = 0
    const interval = setInterval(() => {
      if (currentStep < steps.length) {
        setProgress(steps[currentStep].progress)
        setStatus(steps[currentStep].status)
        currentStep++
      } else {
        clearInterval(interval)
      }
    }, 500)

    return () => clearInterval(interval)
  }, [])

  return (
    <div className="fixed inset-0 bg-gradient-to-br from-blue-600 via-blue-700 to-blue-800 flex items-center justify-center">
      <div className="text-center text-white">
        {/* Logo */}
        <div className="mb-8">
          <div className="w-24 h-24 bg-white bg-opacity-20 rounded-3xl flex items-center justify-center mx-auto mb-4 backdrop-blur-sm">
            <span className="text-4xl font-bold text-white">RM</span>
          </div>
          <h1 className="text-4xl font-bold mb-2">RunMyModel Desktop</h1>
          <p className="text-blue-100 text-lg">Run LLMs locally with Ollama</p>
        </div>

        {/* Progress */}
        <div className="w-80 mx-auto mb-6">
          <div className="flex items-center justify-between mb-2">
            <span className="text-sm text-blue-100">{status}</span>
            <span className="text-sm text-blue-100">{progress}%</span>
          </div>
          <div className="w-full bg-white bg-opacity-20 rounded-full h-2 backdrop-blur-sm">
            <div
              className="bg-white h-2 rounded-full transition-all duration-500 ease-out"
              style={{ width: `${progress}%` }}
            />
          </div>
        </div>

        {/* Loading indicator */}
        <div className="flex items-center justify-center space-x-2">
          <Loader2 className="w-5 h-5 animate-spin" />
          <span className="text-blue-100">Loading...</span>
        </div>

        {/* Footer */}
        <div className="absolute bottom-8 left-0 right-0">
          <p className="text-blue-200 text-sm">
            Powered by <span className="font-semibold">RunMyModel.org</span>
          </p>
        </div>
      </div>
    </div>
  )
}
