; Script generated with the Venis Install Wizard & modified by defnax

; Define your application name
!define APPNAME "RetroShare"
!define VERSION "0.3.0"
!define APPNAMEANDVERSION "${APPNAME} ${VERSION}"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\RetroShare"
InstallDirRegKey HKLM "Software\${APPNAME}" ""
OutFile "RetroShare_${VERSION}_v0.3.0_setup.exe"
BrandingText "${APPNAMEANDVERSION}"
; Use compression
SetCompressor LZMA

; Modern interface settings
!include "MUI.nsh"

!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_FINISHPAGE_RUN "$INSTDIR\RetroShare.exe"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_RESERVEFILE_LANGDLL

LangString TEXT_FLocations_TITLE ${LANG_ENGLISH} "Choose Default Save Locations"
LangString TEXT_FLocations_SUBTITLE ${LANG_ENGLISH} "Choose the folders to save your downloads and torrents to."

!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

Section "RetroShare Core" Section1

  ; Set Section properties
  SetOverwrite on

  ; Clears previous error logs
  Delete "$INSTDIR\*.log"
	
  ; Set Section Files and Shortcuts
  SetOutPath "$INSTDIR\"
  File /r "release\*"
	
SectionEnd

Section "File Association" section2
  ; Delete any existing keys
  DeleteRegKey HKCR "Applications\btdownloadgui.exe"
  DeleteRegKey HKCR "Applications\Azureus.exe"

  ; Write the file association
  WriteRegStr HKCR .pqi "" retroshare
  WriteRegStr HKCR .pqi "Content Type" application/x-bittorrent
  WriteRegStr HKCR "MIME\Database\Content Type\application/x-bittorrent" Extension .pqi
  WriteRegStr HKCR retorshare "" "PQI File"
  WriteRegBin HKCR retroshare EditFlags 00000100
  WriteRegStr HKCR "retroshare\shell" "" open
  WriteRegStr HKCR "retroshare\shell\open\command" "" `"$INSTDIR\RetroShare.exe" "%1"`
SectionEnd

Section "Start Menu Shortcuts" section3

  SetOutPath "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\${APPNAME}"
  CreateShortCut "$SMPROGRAMS\${APPNAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\RetroShare.exe" "" "$INSTDIR\RetroShare.exe" 0

SectionEnd

Section "Desktop Shortcuts" section4

  CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\RetroShare.exe" "" "$INSTDIR\RetroShare.exe" 0
  
SectionEnd

Section -FinishSection

  WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
  WriteRegStr HKLM "Software\${APPNAME}" "Version" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${Section1} "RetroShare program files"
!insertmacro MUI_DESCRIPTION_TEXT ${Section2} "Associate RetroShare with .pqi file extension"
!insertmacro MUI_DESCRIPTION_TEXT ${Section3} "Create RetroShare Start Menu shortcuts"
!insertmacro MUI_DESCRIPTION_TEXT ${Section4} "Create RetroShare Desktop shortcut"	
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section "Uninstall"
  
  ; Remove file association registry keys
  DeleteRegKey HKCR .pqi
  DeleteRegKey HKCR "MIME\Database\Content Type\application/x-bittorrent"
  DeleteRegKey HKCR retroshare
	
  ; Remove program/uninstall regsitry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
  DeleteRegKey HKLM SOFTWARE\${APPNAME}

  ; Remove files and uninstaller
  Delete $INSTDIR\RetroShare.exe
  Delete $INSTDIR\*.dll
  Delete $INSTDIR\*.dat
  Delete $INSTDIR\*.txt
  Delete $INSTDIR\*.ini
  Delete $INSTDIR\*.log

	

  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${APPNAME}\*.*"

  ; Remove desktop icon
  Delete "$DESKTOP\${APPNAME}.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\${APPNAME}"
  RMDir "$INSTDIR"

SectionEnd

Function .onInit



  ;This was written by Vytautas - http://nsis.sourceforge.net/archive/nsisweb.php?page=453
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "RetroShare") i .r1 ?e' 
  Pop $R0 
  StrCmp $R0 0 +3 
    MessageBox MB_OK "The ${APPNAME} installer is already running." 
    Abort 

FunctionEnd







; eof