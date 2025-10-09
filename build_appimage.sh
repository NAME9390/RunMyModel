#!/bin/bash

# Build AppImage for Linux distribution
set -e

echo "📦 Building AppImage for RunMyModel Desktop..."
echo ""

# Build the application first
echo "Step 1: Building application..."
./build_native.sh

# Create AppDir structure
echo ""
echo "Step 2: Creating AppDir structure..."
APPDIR="AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/lib"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"

# Copy executable
cp build/RunMyModelDesktop "$APPDIR/usr/bin/"
cp llms.txt "$APPDIR/usr/bin/"

# Copy Qt libraries
echo "Step 3: Copying Qt dependencies..."
QT_LIBS="libQt6Core.so.6 libQt6Gui.so.6 libQt6Widgets.so.6 libQt6Network.so.6 libQt6DBus.so.6"
for lib in $QT_LIBS; do
    if [ -f "/usr/lib/$lib" ]; then
        cp "/usr/lib/$lib" "$APPDIR/usr/lib/" 2>/dev/null || true
    fi
done

# Copy Qt plugins
mkdir -p "$APPDIR/usr/plugins/platforms"
cp /usr/lib/qt6/plugins/platforms/libqxcb.so "$APPDIR/usr/plugins/platforms/" 2>/dev/null || true

# Create desktop file
cat > "$APPDIR/usr/share/applications/runmymodel-desktop.desktop" << 'EOF'
[Desktop Entry]
Type=Application
Name=RunMyModel Desktop
Comment=Run AI models locally
Exec=RunMyModelDesktop
Icon=runmymodel-desktop
Categories=Development;Science;
Terminal=false
EOF

# Create icon (placeholder - you should provide a real icon)
echo "Step 4: Creating icon..."
convert -size 256x256 xc:transparent \
    -fill '#667eea' -draw 'circle 128,128 128,32' \
    -fill white -gravity center -pointsize 80 -annotate +0+0 '🤖' \
    "$APPDIR/usr/share/icons/hicolor/256x256/apps/runmymodel-desktop.png" 2>/dev/null || \
    echo "Note: ImageMagick not found, skipping icon creation"

# Create AppRun script
cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="${HERE}/usr/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="${HERE}/usr/plugins/platforms"
cd "${HERE}/usr/bin"
exec "${HERE}/usr/bin/RunMyModelDesktop" "$@"
EOF

chmod +x "$APPDIR/AppRun"

# Download appimagetool if not present
if [ ! -f "appimagetool-x86_64.AppImage" ]; then
    echo "Step 5: Downloading appimagetool..."
    wget -q "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
    chmod +x appimagetool-x86_64.AppImage
fi

# Build AppImage
echo ""
echo "Step 6: Building AppImage..."
ARCH=x86_64 ./appimagetool-x86_64.AppImage "$APPDIR" "RunMyModelDesktop-v0.2.0-ALPHA-x86_64.AppImage"

echo ""
echo "✅ AppImage built successfully!"
echo "📦 Output: RunMyModelDesktop-v0.2.0-ALPHA-x86_64.AppImage"
echo ""
echo "To run: ./RunMyModelDesktop-v0.2.0-ALPHA-x86_64.AppImage"

