#!/bin/bash

# Build script for ICAO Compliance Checker
# This script builds only the ICAO compliance checker application

echo "========================================"
echo "Building ICAO Compliance Checker"
echo "========================================"

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "ERROR: CMakeLists.txt not found in current directory"
    echo "Please run this script from the project root directory"
    exit 1
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir -p build
fi

# Configure with CMake
echo "Configuring project with CMake..."
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    cd ..
    exit 1
fi

# Build only the ICAO compliance checker
echo "Building ICAO compliance checker..."
cmake --build . --config Release --target icao_compliance_checker
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    cd ..
    exit 1
fi

cd ..

# Check if the executable was created
if [ -f "build/icao_compliance_checker/icao_compliance_checker" ]; then
    echo ""
    echo "========================================"
    echo "BUILD SUCCESSFUL!"
    echo "========================================"
    echo "Executable created at:"
    echo "  build/icao_compliance_checker/icao_compliance_checker"
    echo ""
    echo "To run the application:"
    echo "  ./build/icao_compliance_checker/icao_compliance_checker -i image.jpg -c data"
    echo ""
else
    echo "ERROR: Executable not found at expected location"
    exit 1
fi

# Test with a sample image if available
if [ -f "image/jimmywong.jpg" ]; then
    echo ""
    echo "========================================"
    echo "TESTING WITH SAMPLE IMAGE"
    echo "========================================"
    echo "Testing with image/jimmywong.jpg..."
    echo ""
    ./build/icao_compliance_checker/icao_compliance_checker -i image/jimmywong.jpg -c data
elif [ -f "data/tests/images/c-01-frontal.png" ]; then
    echo ""
    echo "========================================"
    echo "TESTING WITH SAMPLE IMAGE"
    echo "========================================"
    echo "Testing with data/tests/images/c-01-frontal.png..."
    echo ""
    ./build/icao_compliance_checker/icao_compliance_checker -i data/tests/images/c-01-frontal.png -c data
else
    echo ""
    echo "Note: No sample image found for testing"
    echo "To test with your own image:"
    echo "  ./build/icao_compliance_checker/icao_compliance_checker -i YOUR_IMAGE.jpg -c data"
fi
