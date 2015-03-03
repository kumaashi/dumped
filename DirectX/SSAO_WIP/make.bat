cl main.cpp render.cpp system.cpp input.cpp /EHsc /MP7 /Ox /arch:SSE2 /IC:\WinDDK\7600.16385.1\inc\atl71 /link /LIBPATH:"C:\WinDDK\7600.16385.1\lib\ATL\i386"

if %ERRORLEVEL% == 0 main.exe ELSE "EROIRR"

