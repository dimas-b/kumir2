!include MUI2.nsh
 
# This include is used only by build system
# For manual build change this line to something line:
#  OutFile "kumir2-...exe"
!include outfilename.nsh

Name "�����-2.x"
InstallDir "$PROGRAMFILES\Kumir2x"
RequestExecutionLevel admin


!insertmacro MUI_PAGE_LICENSE "LICENSE_RU.rtf"

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "Russian"

!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0

Function RefreshShellIcons
  ; By jerome tremblay - april 2003
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
  (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd

Section "Kumir" Kumir

	RMDir /r /REBOOTOK "$INSTDIR\lib\kumir2\plugins" ; Prevent conflicting modules from previous intallation

	SetOutPath "$INSTDIR"
	File LICENSE_RU.rtf
	File vcredist_x86.exe
	SetOutPath "$INSTDIR\bin"
	File /r bin\*
	SetOutPath "$INSTDIR\lib"
	File /r lib\*
	SetOutPath "$INSTDIR\share"
	File /r share\*

        SetOutPath "$INSTDIR\llvm-mingw"
        File /nonfatal /r llvm-mingw\*

        SetOutPath "$INSTDIR\python"
        File /nonfatal /r python\*


	ExecWait '"$INSTDIR\vcredist_x86.exe" /passive'
	Delete /REBOOTOK "$INSTDIR\vcredist_x86.exe"
	
        WriteRegStr HKCR ".kum" "" "ru.niisi.kumir2.program"
        WriteRegStr HKCR "ru.niisi.kumir2.program" "" "��������� �����"
        WriteRegStr HKCR "ru.niisi.kumir2.program\shell\open\command" "" '"$INSTDIR\bin\kumir2-open.exe" "%1"'
        WriteRegStr HKCR "ru.niisi.kumir2.program\shell\������� ����������� ����\command" "" 'wscript.exe "$INSTDIR\bin\kumir2-llvmc-w.vbs" "%L"'
        WriteRegStr HKCR "ru.niisi.kumir2.program\DefaultIcon" "" "$INSTDIR\share\icons\kumir2-kum.ico"
        WriteRegStr HKCR ".kod" "" "ru.niisi.kumir2.bytecode"
        WriteRegStr HKCR "ru.niisi.kumir2.bytecode" "" "����������� ������� �����"        
        WriteRegStr HKCR "ru.niisi.kumir2.bytecode\shell\open\command" "" '"$INSTDIR\bin\kumir2-run.exe" "%1" "%*"'
        WriteRegStr HKCR "ru.niisi.kumir2.bytecode\DefaultIcon" "" "$INSTDIR\share\icons\kumir2-kod.ico"
        
        Call RefreshShellIcons

	CreateDirectory "$SMPROGRAMS\�����2"
	CreateShortCut "$SMPROGRAMS\�����2\�����-��������.lnk" "$INSTDIR\bin\kumir2-classic.exe"
	CreateShortCut "$SMPROGRAMS\�����2\����� ��� ������� �������.lnk" "$INSTDIR\bin\kumir2-highgrade.exe"
	CreateShortCut "$SMPROGRAMS\�����2\�����-���.lnk" "$INSTDIR\bin\kumir2-ide.exe"
	CreateShortCut "$SMPROGRAMS\�����2\����� ��� ��������.lnk" "$INSTDIR\bin\kumir2-teacher.exe"

	; Uninstaller registration
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\kumir2" "DisplayName" "�����2"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\kumir2" "UninstallString" "$INSTDIR\uninstall.exe"

	WriteUninstaller $INSTDIR\uninstall.exe
	
SectionEnd

Section "Uninstall"
	Delete /REBOOTOK "$INSTDIR\uninstall.exe"
	RMDir /r /REBOOTOK "$INSTDIR"
	RMDir /r /REBOOTOK "$SMPROGRAMS\�����2"

        DeleteRegKey HKCR ".kum"
        DeleteRegKey HKCR "ru.niisi.kumir2.program"
        DeleteRegKey HKCR ".kod"
        DeleteRegKey HKCR "ru.niisi.kumir2.bytecode"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\kumir2"

SectionEnd
