REM copy dependencies to output folder.  This batch file expects parameter 1 to be the output folder, and paramter 2 to be the configuration type (Debug\Release)

copy "..\externals\win32\pthreads-win32\lib\pthreadVC2.dll" "%1"
copy "..\externals\win32\zlib-1.2.5\bin\zlib1.dll" "%1"
copy "..\externals\win32\lua-5.2.0\lua52.dll" "%1"
copy "..\externals\libpd\libs\libpd.dll" "%1"
copy "..\externals\win32\fftw-3.3.3\libfftw3f-3.dll" "%1"
copy "..\externals\portaudio\build\msvc\Win32\%2\portaudio_x86.dll" "%1"
copy "..\externals\portmidi\%2\portmidi.dll" "%1"
copy "..\externals\win32\libsndfile\bin\libsndfile-1.dll" "%1"
copy "..\externals\win32\libxml2-2.7.8.win32\bin\libxml2.dll" "%1"
copy "..\externals\win32\iconv\iconv.dll" "%1"

copy "..\data\*.*" "%1"

