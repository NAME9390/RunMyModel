# Contributing to RunMyModel Desktop

Thank you for your interest in contributing to RunMyModel Desktop! This document provides guidelines for contributing to the project.

## 🤝 How to Contribute

### Reporting Issues

Before creating an issue, please:

1. **Search existing issues** to avoid duplicates
2. **Check the documentation** for common solutions
3. **Provide detailed information** about your problem

When creating an issue, include:

- **Description**: Clear description of the problem
- **Steps to Reproduce**: Detailed steps to reproduce the issue
- **Expected Behavior**: What you expected to happen
- **Actual Behavior**: What actually happened
- **Environment**: OS, Node.js version, browser version
- **Screenshots**: If applicable, include screenshots

### Suggesting Features

Feature requests are welcome! Please include:

- **Use Case**: Why this feature would be useful
- **Proposed Solution**: How you think it should work
- **Alternatives**: Other solutions you've considered
- **Additional Context**: Any other relevant information

## 🛠️ Development Setup

### Prerequisites

- **Node.js**: Version 18 or higher
- **npm**: Comes with Node.js
- **Git**: For version control

### Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork**:
   ```bash
   git clone https://github.com/your-username/runmymodel-desktop.git
   cd runmymodel-desktop
   ```

3. **Install dependencies**:
   ```bash
   npm install
   ```

4. **Start development server**:
   ```bash
   npm run dev
   ```

5. **Make your changes** and test them

### Development Workflow

1. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes**:
   - Write clean, readable code
   - Follow the existing code style
   - Add tests if applicable
   - Update documentation if needed

3. **Test your changes**:
   ```bash
   npm run type-check
   npm run lint
   npm run build
   ```

4. **Commit your changes**:
   ```bash
   git add .
   git commit -m "Add: brief description of your changes"
   ```

5. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create a Pull Request** on GitHub

## 📝 Code Style Guidelines

### TypeScript

- Use **strict TypeScript** configuration
- Define **proper interfaces** for all data structures
- Use **type guards** instead of type assertions when possible
- **Document complex types** with JSDoc comments

```typescript
/**
 * Represents a chat message in the application
 */
interface ChatMessage {
  /** Unique identifier for the message */
  id: string
  /** Role of the message sender */
  role: 'user' | 'assistant'
  /** Content of the message */
  content: string
  /** Timestamp when the message was created */
  timestamp: number
}
```

### React

- Use **functional components** with hooks
- Implement **proper error boundaries**
- Use **React.memo** for performance optimization
- Follow **React best practices** for state management

```typescript
// Good: Functional component with proper typing
interface ButtonProps {
  variant?: 'primary' | 'secondary'
  size?: 'sm' | 'md' | 'lg'
  children: React.ReactNode
  onClick?: () => void
}

const Button: React.FC<ButtonProps> = ({ 
  variant = 'primary', 
  size = 'md', 
  children, 
  onClick 
}) => {
  return (
    <button 
      className={`btn btn-${variant} btn-${size}`}
      onClick={onClick}
    >
      {children}
    </button>
  )
}
```

### CSS/Styling

- Use **Tailwind CSS** utility classes
- Define **custom CSS variables** for theming
- Follow **mobile-first** responsive design
- Use **semantic class names** for custom styles

```css
/* Good: Using CSS variables and Tailwind */
.custom-component {
  @apply bg-background text-foreground border border-border;
  --custom-property: value;
}

/* Bad: Hard-coded values */
.bad-component {
  background-color: #ffffff;
  color: #000000;
  border: 1px solid #e5e5e5;
}
```

## 🧪 Testing

### Manual Testing

Before submitting a PR, please test:

1. **Functionality**: All features work as expected
2. **Cross-platform**: Test on both Linux and Windows
3. **Responsive**: Test on different screen sizes
4. **Accessibility**: Test with keyboard navigation
5. **Performance**: Check for any performance regressions

### Automated Testing

Run the following commands before submitting:

```bash
# Type checking
npm run type-check

# Linting
npm run lint

# Build test
npm run build
```

## 📋 Pull Request Guidelines

### Before Submitting

- [ ] **Code follows** the project's style guidelines
- [ ] **Tests pass** and new tests are added if needed
- [ ] **Documentation is updated** if necessary
- [ ] **Commits are squashed** into logical units
- [ ] **PR description** explains the changes clearly

### PR Description Template

```markdown
## Description
Brief description of the changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Tested on Linux
- [ ] Tested on Windows
- [ ] Manual testing completed

## Screenshots (if applicable)
Add screenshots to help explain your changes

## Additional Notes
Any additional information about the changes
```

## 🏷️ Commit Message Format

Use clear, descriptive commit messages:

```
type: brief description

Detailed description of the changes (if needed)

- Bullet point for specific changes
- Another bullet point if needed
```

### Commit Types

- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code style changes (formatting, etc.)
- **refactor**: Code refactoring
- **test**: Adding or updating tests
- **chore**: Maintenance tasks

### Examples

```
feat: add dark mode toggle to settings

fix: resolve chat message rendering issue

docs: update installation instructions

refactor: simplify state management logic
```

## 🐛 Bug Reports

When reporting bugs, please include:

1. **Clear title** describing the issue
2. **Detailed description** of the problem
3. **Steps to reproduce** the issue
4. **Expected vs actual behavior**
5. **Environment details** (OS, Node.js version, etc.)
6. **Screenshots or error messages** if applicable

## 💡 Feature Requests

When suggesting features:

1. **Clear title** describing the feature
2. **Detailed description** of the proposed feature
3. **Use case** explaining why it would be useful
4. **Proposed implementation** (if you have ideas)
5. **Additional context** or examples

## 📞 Getting Help

If you need help:

1. **Check the documentation** first
2. **Search existing issues** for similar problems
3. **Ask in discussions** for general questions
4. **Create an issue** for bugs or feature requests

## 📄 License

By contributing to RunMyModel Desktop, you agree that your contributions will be licensed under the Mozilla Public License 2.0 (MPL-2.0).

## 🙏 Recognition

Contributors will be recognized in:

- **README.md**: Listed as contributors
- **Release notes**: Mentioned in relevant releases
- **GitHub**: Shown in the contributors section

Thank you for contributing to RunMyModel Desktop! 🚀