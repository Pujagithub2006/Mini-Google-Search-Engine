@echo off
echo =========================================
echo   Compiling C++ Backend for Mini Search
echo =========================================
echo.

REM Check if g++ is available
g++ --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: g++ compiler not found
    echo.
    echo Please install one of the following:
    echo 1. MinGW-w64: https://www.mingw-w64.org/
    echo 2. TDM-GCC: https://jmeubank.github.io/tdm-gcc/
    echo 3. Visual Studio with C++ tools
    echo.
    echo Or use an IDE like Dev-C++, Code::Blocks, or Visual Studio
    pause
    exit /b 1
)

echo Found g++ compiler!
echo.

REM Check if source file exists
if not exist "mini_search_api.cpp" (
    echo ERROR: mini_search_api.cpp not found
    echo Please make sure the C++ source file is in this directory
    pause
    exit /b 1
)

echo Compiling mini_search_api.cpp...
echo.

REM Compile the C++ program with C++14 standard
g++ -o mini_search_api.exe mini_search_api.cpp -std=c++14

if errorlevel 1 (
    echo.
    echo ERROR: Compilation failed!
    echo Please check the error messages above
    pause
    exit /b 1
)

echo.
echo SUCCESS: Compilation completed!
echo Created: mini_search_api.exe
echo.

REM Copy the executable to main directory for server access
echo Copying executable to main directory...
copy mini_search_api.exe ..\ >nul
if errorlevel 1 (
    echo WARNING: Could not copy executable to main directory
    echo You may need to manually copy mini_search_api.exe to the main folder
) else (
    echo Executable copied to main directory for server access
)

echo.

REM Test the compiled program
echo Testing the compiled program...
echo.

if exist "Docs\technology.txt" (
    echo Testing search functionality...
    mini_search_api.exe search technology
    echo.
    echo Testing suggestions functionality...
    mini_search_api.exe suggestions tech
) else (
    echo WARNING: Docs folder or technology.txt not found
    echo Cannot test search functionality
    echo Make sure your Docs folder contains the text files
)

echo.
echo Backend compilation complete!
echo You can now start the server with run_server.bat
pause