Base64 plugin for NSIS

This is a base64 plugin for NSIS is based on [dselkirk's base64 code](http://forums.winamp.com/showthread.php?s=&threadid=149880&highlight=base64) and has been modified:

+ Compiled with Visual Studio 6 and works with all versions of Windows that NSIS supports including Windows 95
+ Supports base64 padding

```
Name "Base64 test"
OutFile "base64.exe"
SilentInstall silent

Section
  base64::Encode "this is a test"
  Pop $0
  MessageBox MB_OK "Encoded: $0"
  base64::Decode $0
  Pop $0
  MessageBox MB_OK "Decoded: $0"
SectionEnd
```