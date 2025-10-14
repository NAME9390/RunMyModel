#!/bin/bash
# RunMyModel Desktop v0.5.0-PRE-RELEASE - MSI Build Script
# Creates a Windows MSI installer package

set -e

echo "🚀 Building RunMyModel Desktop MSI v0.5.0-PRE-RELEASE"
echo "=================================================="

# Configuration
APP_NAME="RunMyModelDesktop"
APP_VERSION="0.5.0-PRE-RELEASE"
APP_ID="org.runmymodel.desktop"
BUILD_DIR="build_msi"
MSI_DIR="msi"
EXECUTABLES_DIR="../executables"

# Clean previous builds
echo "🧹 Cleaning previous builds..."
rm -rf $BUILD_DIR $MSI_DIR
mkdir -p $BUILD_DIR $MSI_DIR

# Check if we're on Windows or have Wine
if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]] || command -v wine &> /dev/null; then
    echo "✅ Windows environment detected"
    WINE_PREFIX=""
else
    echo "❌ Windows environment not detected. This script requires Windows or Wine."
    echo "Please run this script on Windows or install Wine for cross-compilation."
    exit 1
fi

# Check dependencies
echo "🔍 Checking dependencies..."
if ! command -v makensis &> /dev/null; then
    echo "❌ NSIS not found. Please install NSIS (Nullsoft Scriptable Install System)"
    echo "Download from: https://nsis.sourceforge.io/Download"
    exit 1
fi

# Build the application (cross-compilation if needed)
echo "🔨 Building application..."
cd ../app

# For cross-compilation, we'll need to set up a Windows build environment
# This is a simplified version - in practice, you'd need proper cross-compilation setup
if [[ "$OSTYPE" != "msys" ]] && [[ "$OSTYPE" != "cygwin" ]]; then
    echo "⚠️  Cross-compilation not fully implemented. Please build on Windows."
    echo "For now, creating placeholder files..."
    
    # Create placeholder executable (in real scenario, this would be the actual Windows build)
    mkdir -p build
    echo "#!/bin/bash" > build/RunMyModelDesktop.exe
    echo "echo 'This is a placeholder Windows executable'" >> build/RunMyModelDesktop.exe
    chmod +x build/RunMyModelDesktop.exe
    
    # Create placeholder libraries
    mkdir -p build/lib
    touch build/lib/libggml.dll
    touch build/lib/libllama.dll
    touch build/lib/libggml-cuda.dll
else
    # On Windows, use the actual build script
    ./build.sh
fi

# Copy executable to MSI build directory
echo "📦 Preparing MSI structure..."
cp build/RunMyModelDesktop.exe ../executables/$BUILD_DIR/ 2>/dev/null || cp build/RunMyModelDesktop ../executables/$BUILD_DIR/RunMyModelDesktop.exe
cp -r build/lib ../executables/$BUILD_DIR/
cp -r models ../executables/$BUILD_DIR/
cp -r config ../executables/$BUILD_DIR/

# Create MSI directory structure
cd ../executables
mkdir -p $MSI_DIR/ProgramFiles/RunMyModel
mkdir -p $MSI_DIR/ProgramMenu/RunMyModel
mkdir -p $MSI_DIR/DesktopShortcut

# Copy application files
cp -r $BUILD_DIR/* $MSI_DIR/ProgramFiles/RunMyModel/

# Create desktop shortcut
cat > $MSI_DIR/DesktopShortcut/RunMyModelDesktop.lnk << 'EOF'
[InternetShortcut]
URL=file:///C:/Program Files/RunMyModel/RunMyModelDesktop.exe
IconFile=C:\Program Files\RunMyModel\RunMyModelDesktop.exe
IconIndex=0
EOF

# Create start menu shortcut
cat > $MSI_DIR/ProgramMenu/RunMyModel/RunMyModelDesktop.lnk << 'EOF'
[InternetShortcut]
URL=file:///C:/Program Files/RunMyModel/RunMyModelDesktop.exe
IconFile=C:\Program Files\RunMyModel\RunMyModelDesktop.exe
IconIndex=0
EOF

# Create NSIS installer script
cat > $MSI_DIR/installer.nsi << 'EOF'
; RunMyModel Desktop v0.5.0-PRE-RELEASE Installer Script
; NSIS (Nullsoft Scriptable Install System) script

!define APP_NAME "RunMyModel Desktop"
!define APP_VERSION "0.5.0-PRE-RELEASE"
!define APP_PUBLISHER "RunMyModel Team"
!define APP_WEB_SITE "https://github.com/runmymodel/runmymodel-desktop"
!define APP_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
!define APP_UNINST_ROOT_KEY "HKLM"

; MUI Settings
!include "MUI2.nsh"

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Language Selection Dialog Settings
!define MUI_LANGDLL_REGISTRY_ROOT "${APP_UNINST_ROOT_KEY}"
!define MUI_LANGDLL_REGISTRY_KEY "${APP_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

; MUI Settings
!define MUI_WELCOMEPAGE_TITLE "Welcome to ${APP_NAME} ${APP_VERSION}"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${APP_NAME}.$\r$\n$\r$\n${APP_NAME} is a professional-grade local LLM platform that brings enterprise-level AI capabilities to your local machine.$\r$\n$\r$\nClick Next to continue."

!define MUI_LICENSEPAGE_TEXT_TOP "Please review the license terms before installing ${APP_NAME}."
!define MUI_LICENSEPAGE_TEXT_BOTTOM "If you accept the terms of the agreement, click I Agree to continue. You must accept the agreement to install ${APP_NAME}."

!define MUI_COMPONENTSPAGE_TEXT_TOP "Choose which components of ${APP_NAME} you want to install."

!define MUI_DIRECTORYPAGE_TEXT_TOP "Setup will install ${APP_NAME} in the following folder. To install in a different folder, click Browse and select another folder. Click Next to continue."

!define MUI_FINISHPAGE_TITLE "Completing the ${APP_NAME} Setup Wizard"
!define MUI_FINISHPAGE_TEXT "Setup has finished installing ${APP_NAME} on your computer.$\r$\n$\r$\nClick Finish to close this wizard."

!define MUI_FINISHPAGE_RUN "$INSTDIR\RunMyModelDesktop.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Run ${APP_NAME}"

; Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${APP_NAME} ${APP_VERSION}"
OutFile "RunMyModelDesktop-v${APP_VERSION}-x64.msi"
InstallDir "$PROGRAMFILES64\RunMyModel"
InstallDirRegKey HKLM "Software\RunMyModel" ""
ShowInstDetails show
ShowUnInstDetails show

; Request application privileges for Windows Vista
RequestExecutionLevel admin

; Version Information
VIProductVersion "0.5.0.0"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "Comments" "Professional-grade local LLM platform"
VIAddVersionKey "CompanyName" "${APP_PUBLISHER}"
VIAddVersionKey "LegalCopyright" "© 2024 RunMyModel Team"
VIAddVersionKey "FileDescription" "${APP_NAME} Installer"
VIAddVersionKey "FileVersion" "${APP_VERSION}"
VIAddVersionKey "ProductVersion" "${APP_VERSION}"
VIAddVersionKey "InternalName" "RunMyModelDesktop"
VIAddVersionKey "LegalTrademarks" ""
VIAddVersionKey "OriginalFilename" "RunMyModelDesktop-v${APP_VERSION}-x64.msi"

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  
  ; Main executable
  File "ProgramFiles\RunMyModel\RunMyModelDesktop.exe"
  
  ; Libraries
  File /r "ProgramFiles\RunMyModel\lib"
  
  ; Models directory
  File /r "ProgramFiles\RunMyModel\models"
  
  ; Configuration
  File /r "ProgramFiles\RunMyModel\config"
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\uninst.exe"
  
  ; Registry entries
  WriteRegStr HKLM "Software\RunMyModel" "" $INSTDIR
  WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "DisplayName" "${APP_NAME}"
  WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "DisplayIcon" "$INSTDIR\RunMyModelDesktop.exe"
  WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "DisplayVersion" "${APP_VERSION}"
  WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "URLInfoAbout" "${APP_WEB_SITE}"
  WriteRegStr ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}" "Publisher" "${APP_PUBLISHER}"
  
  ; Start Menu shortcut
  CreateDirectory "$SMPROGRAMS\RunMyModel"
  CreateShortCut "$SMPROGRAMS\RunMyModel\RunMyModel Desktop.lnk" "$INSTDIR\RunMyModelDesktop.exe"
  CreateShortCut "$SMPROGRAMS\RunMyModel\Uninstall.lnk" "$INSTDIR\uninst.exe"
  
  ; Desktop shortcut
  CreateShortCut "$DESKTOP\RunMyModel Desktop.lnk" "$INSTDIR\RunMyModelDesktop.exe"
SectionEnd

Section "Desktop Shortcut" SEC02
  CreateShortCut "$DESKTOP\RunMyModel Desktop.lnk" "$INSTDIR\RunMyModelDesktop.exe"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${APP_NAME}.url" "InternetShortcut" "URL" "${APP_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\RunMyModel\Website.lnk" "$INSTDIR\${APP_NAME}.url"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${APP_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${APP_UNINST_KEY}" "DisplayName" "${APP_NAME}"
  WriteRegStr HKLM "${APP_UNINST_KEY}" "DisplayVersion" "${APP_VERSION}"
  WriteRegStr HKLM "${APP_UNINST_KEY}" "URLInfoAbout" "${APP_WEB_SITE}"
  WriteRegStr HKLM "${APP_UNINST_KEY}" "Publisher" "${APP_PUBLISHER}"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "Core application files and libraries"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "Create a desktop shortcut for easy access"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  ; Remove files
  Delete "$INSTDIR\RunMyModelDesktop.exe"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\${APP_NAME}.url"
  
  ; Remove directories
  RMDir /r "$INSTDIR\lib"
  RMDir /r "$INSTDIR\models"
  RMDir /r "$INSTDIR\config"
  RMDir "$INSTDIR"
  
  ; Remove shortcuts
  Delete "$SMPROGRAMS\RunMyModel\RunMyModel Desktop.lnk"
  Delete "$SMPROGRAMS\RunMyModel\Website.lnk"
  Delete "$SMPROGRAMS\RunMyModel\Uninstall.lnk"
  RMDir "$SMPROGRAMS\RunMyModel"
  Delete "$DESKTOP\RunMyModel Desktop.lnk"
  
  ; Remove registry entries
  DeleteRegKey ${APP_UNINST_ROOT_KEY} "${APP_UNINST_KEY}"
  DeleteRegKey HKLM "Software\RunMyModel"
  
  SetAutoClose true
SectionEnd
EOF

# Create LICENSE file for the installer
cat > $MSI_DIR/LICENSE << 'EOF'
Mozilla Public License Version 2.0
==================================

1. Definitions
--------------

1.1. "Contributor"
    means each individual or legal entity that creates, contributes to
    the creation of, or owns Covered Software.

1.2. "Contributor Version"
    means the combination of the Contributions of others (if any) used
    by a Contributor and that particular Contributor's Contribution.

1.3. "Contribution"
    means Covered Software of a particular Contributor.

1.4. "Covered Software"
    means Source Code Form to which the initial Contributor has attached
    the notice in Exhibit A, the Executable Form of such Source Code
    Form, and Modifications of such Source Code Form, in each case
    including portions thereof.

1.5. "Incompatible With Secondary Licenses"
    means

    (a) that the initial Contributor has attached the notice described
        in Exhibit B to the Covered Software; or

    (b) that the Covered Software was made available under the terms of
        version 1.1 or earlier of the License, but not also under the
        terms of a Secondary License.

1.6. "Executable Form"
    means any form of the work other than Source Code Form.

1.7. "Larger Work"
    means a work that combines Covered Software with other material, in
    a separate file or files, that is not Covered Software.

1.8. "License"
    means this document.

1.9. "Licensable"
    means having the right to grant, to the maximum extent possible,
    whether at the time of the initial grant or subsequently acquired,
    any and all of the rights conveyed by this License.

1.10. "Modifications"
    means any of the following:

    (a) any file in Source Code Form that results from an addition to,
        deletion from, or modification of the contents of Covered
        Software; or

    (b) any new file in Source Code Form that contains any Covered
        Software.

1.11. "Patent Claims" of a Contributor
    means any patent claim(s), including without limitation, method,
    process, and apparatus claims, in any patent Licensable by such
    Contributor that would be infringed, but for the grant of the
    License, by making, using, selling, offering for sale, having made,
    import, or otherwise transfer of either its Contributions or its
    Contributor Version.

1.12. "Secondary License"
    means either the GNU General Public License, Version 2.0, the GNU
    Lesser General Public License, Version 2.1, the GNU Affero General
    Public License, Version 3.0, or any later versions of those
    licenses.

1.13. "Source Code Form"
    means the form of the work preferred for making modifications.

1.14. "You" (or "Your")
    means an individual or a legal entity exercising rights under this
    License. For legal entities, "You" includes any entity that
    controls, is controlled by, or is under common control with You. For
    purposes of this definition, "control" means (a) the power, direct
    or indirect, to cause the direction or management of such entity,
    whether by contract or otherwise, or (b) ownership of more than
    fifty percent (50%) of the outstanding shares or beneficial
    ownership of such entity.

2. License Grants and Conditions
-------------------------------

2.1. Grants

Each Contributor hereby grants You a world-wide, royalty-free,
non-exclusive license:

(a) under intellectual property rights (other than patent or trademark)
    Licensable by such Contributor to use, reproduce, make available,
    modify, display, perform, distribute, and otherwise exploit its
    Contributions, either on an unmodified basis, with Modifications,
    or as part of a Larger Work; and

(b) under Patent Claims of such Contributor to make, use, sell, offer
    for sale, have made, import, and otherwise transfer either its
    Contributions or its Contributor Version.

2.2. Effective Date

The licenses granted in Section 2.1 with respect to any Contribution
become effective for each Contribution on the date the Contributor
first distributes such Contribution.

2.3. Limitations on Grant Scope

The licenses granted in this Section 2 are the only rights granted under
this License. No additional rights or licenses will be implied from the
distribution, licensing or sale of Covered Software under this License.
Notwithstanding Section 2.1(b) above, no patent license is granted by a
Contributor:

(a) for any code that a Contributor has removed from Covered Software;
    or

(b) for infringements caused by: (i) Your and any other third party's
    modifications of Covered Software, or (ii) the combination of its
    Contributions with other software (except as part of its Contributor
    Version); or

(c) under Patent Claims infringed by Covered Software in the absence of
    its Contributions.

This License does not grant any rights in the trademarks, service marks,
or logos of any Contributor (except as may be necessary to comply with
the notice requirements in Section 3.4).

2.4. Subsequent Licenses

No Contributor makes additional grants as a result of Your choice to
distribute the Covered Software under a subsequent version of this
License (see Section 10.2) or under the terms of a Secondary License
(if permitted under the terms of Section 3.3).

2.5. Representation

Each Contributor represents that the Contributor believes its
Contributions are its original creation(s) or it has sufficient rights
to grant the rights to its Contributions conveyed by this License.

2.6. Fair Use

This License is not intended to limit any rights You have under
applicable copyright doctrines of fair use, fair dealing, or other
equivalents.

2.7. Conditions

Sections 3.1, 3.2, 3.3, and 3.4 are conditions of the licenses granted
in Section 2.1.

3. Responsibilities
------------------

3.1. Distribution of Source Form

All distribution of Covered Software in Source Code Form, including any
Modifications that You create or to which You contribute, must be under
the terms of this License. You must inform recipients that the Source
Code Form of the Covered Software is governed by the terms of this
License, and how they can obtain a copy of this License. You may not
attempt to alter or restrict the recipients' rights in the Source Code
Form.

3.2. Distribution of Executable Form

If You distribute Covered Software in Executable Form then:

(a) such Covered Software must also be made available in Source Code
    Form, as described in Section 3.1, and You must inform recipients of
    the Executable Form how they can obtain a copy of such Source Code
    Form. However, You do not need to provide Recipients with a copy of
    the Source Code Form corresponding to the Executable Form of the
    Covered Software, and You may choose to distribute the Executable
    Form of the Covered Software under a different license, provided that
    the Source Code Form of the Covered Software is made available under
    this License, as described in Section 3.1, and You inform recipients
    of the Executable Form how they can obtain a copy of such Source Code
    Form.

(b) You may distribute such Executable Form under the terms of this
    License, or under the terms of a Secondary License, if permitted under
    the terms of Section 3.3.

3.3. Distribution of a Larger Work

You may create and distribute a Larger Work under terms of Your choice,
provided that You also comply with the requirements of this License for
the Covered Software. If the Larger Work is a combination of Covered
Software with a work governed by one or more Secondary Licenses, and the
Covered Software is not Incompatible With Secondary Licenses, this
License permits You to additionally distribute such Covered Software
under the terms of such Secondary License(s), so that the recipient of
the Larger Work may, at their option, further distribute the Covered
Software under the terms of either this License or such Secondary
License(s).

3.4. Notices

You may not remove or alter the substance of any license notices
(including copyright notices, patent notices, disclaimers of warranty, or
limitations of liability) contained within the Source Code Form of the
Covered Software, except that You may alter any license notices to the
extent required to remedy known factual inaccuracies.

3.5. Application of Additional Terms

You may choose to offer, and to charge a fee for, acceptance of support,
warranty, indemnity, or other liability obligations and/or rights
consistent with this License. However, in accepting such obligations,
You may act only on Your own behalf and on Your sole responsibility, not
on behalf of any other Contributor, and only if You agree to indemnify,
defend, and hold each Contributor harmless for any liability incurred by,
or claims asserted against, such Contributor by reason of your accepting
any such warranty or support.

4. Inability to Comply Due to Statute or Regulation
--------------------------------------------------

If it is impossible for You to comply with any of the terms of this
License with respect to some or all of the Covered Software due to
statute, judicial order, or regulation then You must: (a) comply with
the terms of this License to the maximum extent possible; and (b)
describe the limitations and the code they affect. Such description must
be placed in a text file included with all distributions of the Covered
Software under this License. Except to the extent prohibited by statute
or regulation, such description must be sufficiently detailed for a
recipient of ordinary skill to be able to understand it.

5. Termination
-------------

5.1. The rights granted under this License will terminate automatically
if You fail to comply with terms herein and fail to cure such breach
within 30 days of becoming aware of the breach. Provisions which, by
their nature, must remain in effect beyond the termination of this
License shall survive.

5.2. If You initiate litigation against any entity by asserting a patent
infringement claim (excluding declaratory judgment actions, counter-claims,
and cross-claims) alleging that a Contributor Version directly or
indirectly infringes any patent, then the rights granted to You by any
and all Contributors for the Covered Software under Section 2.1 of this
License shall terminate.

5.3. In the event of termination under Sections 5.1 or 5.2 above, all
end user license agreements (excluding distributors and resellers) which
have been validly granted by You or Your distributors under this License
prior to termination shall survive termination.

6. Disclaimer of Warranty
-------------------------

Covered Software is provided under this License on an "as is" basis,
without warranty of any kind, either expressed, implied, or statutory,
including, without limitation, warranties that the Covered Software is
free of defects, merchantable, fit for a particular purpose or
non-infringing. The entire risk as to the quality and performance of the
Covered Software is with You. Should any Covered Software prove
defective in any respect, You (not any Contributor) assume the cost of
any necessary servicing, repair or correction. This disclaimer of
warranty constitutes an essential part of this License. No use of any
Covered Software is authorized under this License except under this
disclaimer.

7. Limitation of Liability
-------------------------

Under no circumstances and under no legal theory, whether tort (including
negligence), contract, or otherwise, shall any Contributor, or anyone
who distributes Covered Software as permitted above, be liable to You
for any direct, indirect, special, incidental, or consequential damages
of any character including, without limitation, damages for loss of
goodwill, work stoppage, computer failure or malfunction, or any and
all other commercial damages or losses, even if such party shall have
been informed of the possibility of such damages. This limitation of
liability shall not apply to liability for death or personal injury
resulting from such party's negligence to the extent applicable law
prohibits such limitation. Some jurisdictions do not allow the exclusion
or limitation of incidental or consequential damages, so this exclusion
and limitation may not apply to You.

8. Litigation
------------

Any litigation relating to this License may be brought only in the
courts of a jurisdiction where the defendant maintains its principal
place of business and such litigation will be governed by laws of that
jurisdiction, without reference to its conflict-of-law provisions.
Nothing in this Section shall prevent a party's ability to bring
cross-claims or counter-claims.

9. Miscellaneous
---------------

This License represents the complete agreement concerning the subject
matter hereof. If any provision of this License is held to be
unenforceable, such provision shall be reformed only to the extent
necessary to make it enforceable. Any law or regulation which provides
that the language of a contract shall be construed against the drafter
shall not be used to construe this License against a Contributor.

10. Versions of the License
--------------------------

10.1. New Versions

Mozilla Foundation is the license steward. Except as provided in Section
10.3, no one other than the license steward has the right to modify or
publish new versions of this License. Each version will be given a
distinguishing version number.

10.2. Effect of New Versions

You may distribute the Covered Software under the terms of the version
of the License under which You originally received the Covered Software,
or under the terms of any subsequent version published by the license
steward.

10.3. Modified Versions

If you create software not governed by this License, and you want to
create a new license for such software, you may create and use a
modified version of this License if you rename the license and remove
any references to the name of the license steward (except to note that
such modified license differs from this License).

10.4. Distributing Source Code Form that is Incompatible With Secondary
Licenses

If You choose to distribute Source Code Form that is Incompatible With
Secondary Licenses under the terms of this version of the License, the
notice described in Exhibit B of this License must be attached.

Exhibit A - Source Code Form License Notice
------------------------------------------

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular
file, then You may include the notice in a location (such as a LICENSE
file in a relevant directory) where a recipient would be likely to look
for such a notice.

You may add additional accurate notices of copyright ownership.

Exhibit B - "Incompatible With Secondary Licenses" Notice
--------------------------------------------------------

  This Source Code Form is "Incompatible With Secondary Licenses" as
  defined by the Mozilla Public License, v. 2.0.
EOF

# Build MSI using NSIS
echo "🔨 Building MSI installer..."
makensis $MSI_DIR/installer.nsi

# Move the final MSI to executables directory
if [ -f $MSI_DIR/*.msi ]; then
    mv $MSI_DIR/*.msi $APP_NAME-v$APP_VERSION-x64.msi
    echo "✅ MSI created: $APP_NAME-v$APP_VERSION-x64.msi"
else
    echo "❌ MSI creation failed"
    exit 1
fi

# Clean up build files
echo "🧹 Cleaning up..."
rm -rf $BUILD_DIR $MSI_DIR

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ MSI build complete!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📦 Output: $APP_NAME-v$APP_VERSION-x64.msi"
echo "📁 Location: $(pwd)/$APP_NAME-v$APP_VERSION-x64.msi"
echo "💾 Size: $(du -h $APP_NAME-v$APP_VERSION-x64.msi | cut -f1)"
echo ""
echo "🚀 To install: Double-click the MSI file on Windows"
echo "📋 To uninstall: Use Add/Remove Programs in Windows Control Panel"
echo ""
