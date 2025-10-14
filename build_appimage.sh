#!/bin/bash
# RunMyModel Desktop v0.5.0-PRE-RELEASE - AppImage Build Script
# Creates a portable Linux AppImage executable

set -e

echo "🚀 Building RunMyModel Desktop AppImage v0.5.0-PRE-RELEASE"
echo "=================================================="

# Configuration
APP_NAME="RunMyModelDesktop"
APP_VERSION="0.5.0-PRE-RELEASE"
APP_ID="org.runmymodel.desktop"
BUILD_DIR="build_appimage"
APPIMAGE_DIR="appimage"
EXECUTABLES_DIR="../executables"

# Clean previous builds
echo "🧹 Cleaning previous builds..."
rm -rf $BUILD_DIR $APPIMAGE_DIR
mkdir -p $BUILD_DIR $APPIMAGE_DIR

# Check dependencies
echo "🔍 Checking dependencies..."
if ! command -v linuxdeploy &> /dev/null; then
    echo "❌ linuxdeploy not found. Installing..."
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
    sudo mv linuxdeploy-x86_64.AppImage /usr/local/bin/linuxdeploy
fi

if ! command -v linuxdeploy-plugin-qt &> /dev/null; then
    echo "❌ linuxdeploy-plugin-qt not found. Installing..."
    wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
    chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
    sudo mv linuxdeploy-plugin-qt-x86_64.AppImage /usr/local/bin/linuxdeploy-plugin-qt
fi

# Build the application
echo "🔨 Building application..."
cd ../app
./build.sh

# Copy executable to AppImage build directory
echo "📦 Preparing AppImage structure..."
cp build/RunMyModelDesktop ../executables/$BUILD_DIR/
cp -r build/lib ../executables/$BUILD_DIR/
cp -r models ../executables/$BUILD_DIR/
cp -r config ../executables/$BUILD_DIR/

# Create AppDir structure
cd ../executables
mkdir -p $APPIMAGE_DIR/usr/bin
mkdir -p $APPIMAGE_DIR/usr/share/applications
mkdir -p $APPIMAGE_DIR/usr/share/icons/hicolor/256x256/apps
mkdir -p $APPIMAGE_DIR/usr/share/metainfo

# Copy application files
cp $BUILD_DIR/RunMyModelDesktop $APPIMAGE_DIR/usr/bin/
cp -r $BUILD_DIR/lib $APPIMAGE_DIR/usr/lib/
cp -r $BUILD_DIR/models $APPIMAGE_DIR/usr/share/runmymodel/
cp -r $BUILD_DIR/config $APPIMAGE_DIR/usr/share/runmymodel/

# Create desktop file
cat > $APPIMAGE_DIR/usr/share/applications/$APP_ID.desktop << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=RunMyModel Desktop
Comment=Professional-grade local LLM platform
Exec=RunMyModelDesktop
Icon=$APP_ID
Categories=Office;Education;Science;
StartupNotify=true
MimeType=application/x-gguf;
EOF

# Create AppStream metadata
cat > $APPIMAGE_DIR/usr/share/metainfo/$APP_ID.appdata.xml << EOF
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>$APP_ID</id>
  <metadata_license>MPL-2.0</metadata_license>
  <project_license>MPL-2.0</project_license>
  <name>RunMyModel Desktop</name>
  <summary>Professional-grade local LLM platform</summary>
  <description>
    <p>
      RunMyModel Desktop is a cutting-edge desktop application that brings 
      enterprise-level AI capabilities to your local machine. Built with Qt6 
      and powered by llama.cpp, it provides a seamless interface for running, 
      fine-tuning, and managing Large Language Models completely offline.
    </p>
  </description>
  <launchable type="desktop-id">$APP_ID.desktop</launchable>
  <url type="homepage">https://github.com/runmymodel/runmymodel-desktop</url>
  <url type="bugtracker">https://github.com/runmymodel/runmymodel-desktop/issues</url>
  <screenshots>
    <screenshot type="default">
      <caption>Main Interface</caption>
    </screenshot>
  </screenshots>
  <releases>
    <release version="$APP_VERSION" date="$(date +%Y-%m-%d)">
      <description>
        <p>Pre-release version with advanced features and improvements.</p>
      </description>
    </release>
  </releases>
  <provides>
    <binary>RunMyModelDesktop</binary>
  </provides>
  <categories>
    <category>Office</category>
    <category>Education</category>
    <category>Science</category>
  </categories>
  <keywords>
    <keyword>AI</keyword>
    <keyword>LLM</keyword>
    <keyword>Machine Learning</keyword>
    <keyword>Local</keyword>
    <keyword>Offline</keyword>
  </keywords>
</component>
EOF

# Create icon (placeholder - you can replace with actual icon)
echo "🎨 Creating application icon..."
convert -size 256x256 xc:transparent -fill "#0078d4" -draw "circle 128,128 128,64" -fill white -pointsize 72 -gravity center -annotate +0+0 "RMM" $APPIMAGE_DIR/usr/share/icons/hicolor/256x256/apps/$APP_ID.png 2>/dev/null || {
    # Fallback: create a simple colored square
    echo "Creating fallback icon..."
    echo "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==" | base64 -d > $APPIMAGE_DIR/usr/share/icons/hicolor/256x256/apps/$APP_ID.png
}

# Create AppRun script
cat > $APPIMAGE_DIR/AppRun << 'EOF'
#!/bin/bash
# AppRun script for RunMyModel Desktop

# Get the directory where the AppImage is mounted
HERE="$(dirname "$(readlink -f "${0}")")"

# Set environment variables
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="${HERE}/usr/lib/qt6/plugins"
export QML2_IMPORT_PATH="${HERE}/usr/lib/qt6/qml"

# Change to the application directory
cd "${HERE}/usr/share/runmymodel"

# Run the application
exec "${HERE}/usr/bin/RunMyModelDesktop" "$@"
EOF

chmod +x $APPIMAGE_DIR/AppRun

# Build AppImage using linuxdeploy
echo "🔨 Building AppImage..."
linuxdeploy \
    --appdir $APPIMAGE_DIR \
    --output appimage \
    --plugin qt \
    --desktop-file $APPIMAGE_DIR/usr/share/applications/$APP_ID.desktop \
    --icon-file $APPIMAGE_DIR/usr/share/icons/hicolor/256x256/apps/$APP_ID.png \
    --appimage-extract-and-run

# Move the final AppImage to executables directory
if [ -f $APPIMAGE_DIR/*.AppImage ]; then
    mv $APPIMAGE_DIR/*.AppImage $APP_NAME-v$APP_VERSION-x86_64.AppImage
    echo "✅ AppImage created: $APP_NAME-v$APP_VERSION-x86_64.AppImage"
else
    echo "❌ AppImage creation failed"
    exit 1
fi

# Clean up build files
echo "🧹 Cleaning up..."
rm -rf $BUILD_DIR $APPIMAGE_DIR

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ AppImage build complete!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📦 Output: $APP_NAME-v$APP_VERSION-x86_64.AppImage"
echo "📁 Location: $(pwd)/$APP_NAME-v$APP_VERSION-x86_64.AppImage"
echo "💾 Size: $(du -h $APP_NAME-v$APP_VERSION-x86_64.AppImage | cut -f1)"
echo ""
echo "🚀 To run: ./$APP_NAME-v$APP_VERSION-x86_64.AppImage"
echo "📋 To install: chmod +x $APP_NAME-v$APP_VERSION-x86_64.AppImage && ./$APP_NAME-v$APP_VERSION-x86_64.AppImage"
echo ""
