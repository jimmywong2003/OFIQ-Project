@echo off
REM Build script for ICAO Compliance Checker
REM This script builds only the ICAO compliance checker application

echo ========================================
echo Building ICAO Compliance Checker
echo ========================================

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo ERROR: CMakeLists.txt not found in current directory
    echo Please run this script from the project root directory
    pause
    exit /b 1
)

REM Create build directory if it doesn't exist
if not exist "build_test" (
    echo Creating build directory...
    mkdir build_test
)

REM Configure with CMake
echo Configuring project with CMake...
cd build_test
cmake .. -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed
    cd ..
    pause
    exit /b 1
)

REM Build only the ICAO compliance checker
echo Building ICAO compliance checker...
cmake --build . --config Release --target icao_compliance_checker
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed
    cd ..
    pause
    exit /b 1
)

cd ..

REM Check if the executable was created
if exist "build_test\icao_compliance_checker\Release\icao_compliance_checker.exe" (
    echo.
    echo ========================================
    echo BUILD SUCCESSFUL!
    echo ========================================
    echo Executable created at:
    echo   build_test\icao_compliance_checker\Release\icao_compliance_checker.exe
    echo.
    echo To run the application:
    echo   build_test\icao_compliance_checker\Release\icao_compliance_checker.exe -i image.jpg -c data
    echo.
) else (
    echo ERROR: Executable not found at expected location
    pause
    exit /b 1
)

REM Test with a sample image if available
if exist "image\jimmywong.jpg" (
    echo.
    echo ========================================
    echo TESTING WITH SAMPLE IMAGE
    echo ========================================
    echo Testing with image\jimmywong.jpg...
    echo.
    build_test\icao_compliance_checker\Release\icao_compliance_checker.exe -i image\jimmywong.jpg -c data
) else if exist "data\tests\images\c-01-frontal.png" (
    echo.
    echo ========================================
    echo TESTING WITH SAMPLE IMAGE
    echo ========================================
    echo Testing with data\tests\images\c-01-frontal.png...
    echo.
    build_test\icao_compliance_checker\Release\icao_compliance_checker.exe -i data\tests\images\c-01-frontal.png -c data
) else (
    echo.
    echo Note: No sample image found for testing
    echo To test with your own image:
    echo   build_test\icao_compliance_checker\Release\icao_compliance_checker.exe -i YOUR_IMAGE.jpg -c data
)

pause
