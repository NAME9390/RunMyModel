# Development Guide

This document provides detailed information for developers working on RunMyModel Desktop.

## 🏗️ Architecture

### Frontend Architecture

The application follows a modern React architecture with the following key components:

- **Components**: Reusable UI components in `src/components/`
- **Stores**: State management using Zustand in `src/store/`
- **Services**: API and backend communication in `src/services/`
- **Types**: TypeScript type definitions in `src/types/`

### State Management

The application uses Zustand for state management with three main stores:

1. **AppStore**: Global application state (theme, settings)
2. **ChatStore**: Chat-related state (messages, conversations)
3. **ModelStore**: Model management state (installed models, current model)

### Component Structure

```
src/components/
├── ui/                    # Reusable UI components
│   ├── button.tsx        # Button component with variants
│   ├── input.tsx         # Input component
│   ├── textarea.tsx      # Textarea component
│   └── card.tsx          # Card component
├── ModernChatWindow.tsx  # Main chat interface
├── ModernSidebar.tsx     # Sidebar with navigation
└── ...                   # Other components
```

## 🎨 Design System

### Color Palette

The application uses a custom design system with CSS variables:

```css
:root {
  --background: 0 0% 100%;
  --foreground: 222.2 84% 4.9%;
  --primary: 221.2 83.2% 53.3%;
  --secondary: 210 40% 96%;
  --muted: 210 40% 96%;
  --accent: 210 40% 96%;
  --destructive: 0 84.2% 60.2%;
  --border: 214.3 31.8% 91.4%;
  --input: 214.3 31.8% 91.4%;
  --ring: 221.2 83.2% 53.3%;
}
```

### Dark Mode

Dark mode is implemented using CSS custom properties and Tailwind's dark mode classes:

```css
.dark {
  --background: 222.2 84% 4.9%;
  --foreground: 210 40% 98%;
  /* ... other dark mode variables */
}
```

## 🔧 Build System

### Vite Configuration

The project uses Vite for building with the following configuration:

- **TypeScript**: Full TypeScript support with type checking
- **Tailwind CSS**: Utility-first CSS framework
- **PostCSS**: CSS processing with autoprefixer
- **Hot Module Replacement**: Fast development with HMR

### Build Process

1. **TypeScript Compilation**: `tsc --noEmit`
2. **Frontend Build**: `vite build`
3. **Asset Optimization**: Minification and compression
4. **Platform Packaging**: Linux and Windows executables

## 📦 Packaging

### Universal Build Script

The `build.sh` script handles cross-platform builds:

```bash
#!/bin/bash
# Builds for Linux and Windows
# Creates distribution packages
# Handles dependencies and prerequisites
```

### Platform-Specific Packaging

#### Linux
- Shell script launcher
- Desktop entry file
- TAR.GZ distribution package

#### Windows
- Batch file launcher
- Installer script
- ZIP distribution package

## 🧪 Testing

### Type Checking

```bash
npm run type-check
```

### Linting

```bash
npm run lint
```

### Manual Testing

1. **Development Server**: `npm run dev`
2. **Production Build**: `npm run build && npm run preview`
3. **Cross-Platform**: Test on both Linux and Windows

## 🚀 Deployment

### Development Deployment

1. **Local Development**:
   ```bash
   npm run dev
   ```

2. **Production Build**:
   ```bash
   ./build.sh
   ```

### Distribution

1. **Create Packages**:
   ```bash
   ./build.sh
   ```

2. **Upload to Releases**:
   - Linux: `executables/runmymodel-desktop-linux.tar.gz`
   - Windows: `executables/runmymodel-desktop-windows.zip`

## 🔍 Debugging

### Development Tools

- **React DevTools**: Browser extension for React debugging
- **TypeScript**: Compile-time error checking
- **ESLint**: Code quality and style checking
- **Browser DevTools**: Runtime debugging

### Common Issues

1. **Build Failures**: Check Node.js version and dependencies
2. **Type Errors**: Run `npm run type-check` for detailed errors
3. **Styling Issues**: Verify Tailwind CSS classes and custom properties

## 📝 Code Style

### TypeScript

- Use strict TypeScript configuration
- Define proper interfaces for all data structures
- Use type assertions sparingly
- Prefer type guards over type assertions

### React

- Use functional components with hooks
- Implement proper error boundaries
- Use React.memo for performance optimization
- Follow React best practices for state management

### CSS

- Use Tailwind CSS utility classes
- Define custom CSS variables for theming
- Use CSS modules for component-specific styles
- Follow mobile-first responsive design

## 🔄 State Management Patterns

### Zustand Stores

```typescript
// Example store structure
interface AppState {
  // State
  theme: 'light' | 'dark'
  
  // Actions
  setTheme: (theme: 'light' | 'dark') => void
}

export const useAppStore = create<AppState>()(
  persist(
    (set) => ({
      theme: 'light',
      setTheme: (theme) => set({ theme }),
    }),
    {
      name: 'app-store',
      storage: createJSONStorage(() => localStorage),
    }
  )
)
```

### Component State

- Use local state for component-specific data
- Use stores for shared application state
- Implement proper state updates and side effects

## 🌐 API Integration

### Backend Services

The application integrates with backend services through:

- **Hugging Face API**: Model inference and management
- **Local Backend**: C++ backend for system integration
- **File System**: Local file operations and storage

### Error Handling

```typescript
try {
  const response = await api.call()
  // Handle success
} catch (error) {
  console.error('API Error:', error)
  // Handle error state
}
```

## 📚 Resources

- [React Documentation](https://react.dev/)
- [TypeScript Handbook](https://www.typescriptlang.org/docs/)
- [Tailwind CSS Docs](https://tailwindcss.com/docs)
- [Zustand Guide](https://github.com/pmndrs/zustand)
- [Vite Guide](https://vitejs.dev/guide/)