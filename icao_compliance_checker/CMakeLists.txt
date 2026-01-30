cmake_minimum_required(VERSION 3.26)

project(icao_compliance_checker)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(OpenCV REQUIRED COMPONENTS core calib3d imgcodecs imgproc dnn ml)
find_package(taocpp-json REQUIRED)  
find_package(magic_enum REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../OFIQlib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../OFIQlib/modules
)

# Add executable
add_executable(icao_compliance_checker
    src/icao_compliance_checker.cpp
)

# Link libraries
target_link_libraries(icao_compliance_checker
    ofiq_lib
    ${OpenCV_LIBS}
    taocpp::json
    magic_enum::magic_enum
)

# Copy configuration files to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config/icao_compliance_config.json
    ${CMAKE_CURRENT_BINARY_DIR}/icao_compliance_config.json
    COPYONLY
)

# Install target (optional)
install(TARGETS icao_compliance_checker
    RUNTIME DESTINATION bin
)

# Copy config file to install directory
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/icao_compliance_config.json
    DESTINATION share/icao_compliance_checker
)

# Copy ofiq_lib.dll to the same directory as icao_compliance_checker.exe
install(FILES $<TARGET_FILE:ofiq_lib>
    DESTINATION bin
    CONFIGURATIONS Release
)
