# RunMyModel Desktop Icons

This directory contains the application icons for different platforms.

## Required Icons

- `32x32.png` - 32x32 pixel PNG icon
- `128x128.png` - 128x128 pixel PNG icon  
- `128x128@2x.png` - 256x256 pixel PNG icon (retina)
- `icon.icns` - macOS icon bundle
- `icon.ico` - Windows icon file

## Icon Design

The RunMyModel Desktop icon should feature:
- A modern, minimalist design
- Blue gradient background (#2563eb to #1d4ed8)
- "RM" monogram or a stylized "R" symbol
- Clean, professional appearance
- High contrast for visibility at small sizes

## Creating Icons

You can create these icons using:
- Figma, Sketch, or Adobe Illustrator for design
- Online icon generators for format conversion
- ImageMagick for batch processing

Example ImageMagick commands:
```bash
# Convert PNG to ICO
magick convert icon-256.png icon.ico

# Convert PNG to ICNS (requires additional tools)
# Use iconutil on macOS or online converters
```
