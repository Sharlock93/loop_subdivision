@echo off
set libs_win=user32.lib gdi32.lib OpenGL32.lib
cl /nologo -Iinclude src\main.cpp /std:c++latest  /Fe./build/main.exe -Fo./build/main.obj -Fd./build/main.pdb %libs_win%  -W3 /Zi /MT /EHsc
:: cl /nologo -Iinclude src\faceindex2directededge.cpp  /Fe./build/faceindex2directededge.exe -Fo./build/faceindex2directededge.obj -Fd./build/faceindex2directededge.pdb %libs_win%  -W3 /Zi /MT /EHsc
