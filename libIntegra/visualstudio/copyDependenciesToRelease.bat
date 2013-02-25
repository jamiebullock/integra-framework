copy "..\..\libIntegra_dependencies\xmlrpc-c\bin\Release-Win32\libxmlrpc.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\xmlrpc-c\bin\Release-Win32\libxmlrpc_abyss.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\xmlrpc-c\bin\Release-Win32\libxmlrpc_server.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\xmlrpc-c\bin\Release-Win32\libxmlrpc_server_abyss.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\xmlrpc-c\bin\Release-Win32\libxmlrpc_util.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\xmlrpc-c\bin\Release-Win32\libxmlrpc_xmlparse.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\xmlrpc-c\bin\Release-Win32\libxmlrpc_xmltok.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\libxml2-2.7.8.win32\bin\libxml2.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\pthreads-win32\lib\pthreadVC2.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\bin\iconv.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\zlib-1.2.5\bin\zlib1.dll" "..\..\build\Release\server\"
copy "..\..\libIntegra_dependencies\lua-5.2.0\lua52.dll" "..\..\build\Release\server\"

copy "..\data\CollectionSchema.xsd" "..\..\build\Release\server\"
copy "..\data\id2guid.csv" "..\..\build\Release\server"

rd /s /q ..\..\build\Release\modules
mkdir ..\..\build\Release\modules
xcopy "..\..\modules" "..\..\build\Release\modules" /Y /Q

copy "..\..\host\Pd\Integra_Host.pd" "..\..\build\Release\host\extra"

rd /s /q ..\..\build\Release\gui\BlockLibrary
mkdir ..\..\build\Release\gui\BlockLibrary
xcopy "..\..\blocks" "..\..\build\Release\gui\BlockLibrary" /E /Y /Q

CALL documentation_deployment\compileAllDocumentation.bat ..\..\build\Release\