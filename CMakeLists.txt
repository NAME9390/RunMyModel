cmake_minimum_required(VERSION 3.16)
project(RunMyModelDesktop VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt meta-object compiler and resource compiler
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 components - NO WebEngine needed!
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Include directories
include_directories(src-cpp/include)

# Create executable with all source files
set(RUNMYMODEL_SOURCES
    src-cpp/src/main.cpp
    src-cpp/src/mainwindow.cpp
    src-cpp/src/backend.cpp
    src-cpp/src/model_manager.cpp
    src-cpp/src/huggingface_client.cpp
    src-cpp/src/system_info.cpp
)

set(RUNMYMODEL_HEADERS
    src-cpp/include/mainwindow.h
    src-cpp/include/backend.h
    src-cpp/include/model_manager.h
    src-cpp/include/huggingface_client.h
    src-cpp/include/system_info.h
)

add_executable(RunMyModelDesktop ${RUNMYMODEL_SOURCES} ${RUNMYMODEL_HEADERS})

# Link Qt6 libraries - NO WebEngine!
target_link_libraries(RunMyModelDesktop
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# Set properties
set_target_properties(RunMyModelDesktop PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# Copy llms.txt to build directory
add_custom_command(TARGET RunMyModelDesktop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/llms.txt
            ${CMAKE_BINARY_DIR}/llms.txt
)

# Install rules
install(TARGETS RunMyModelDesktop
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Install llms.txt
install(FILES ${CMAKE_SOURCE_DIR}/llms.txt
    DESTINATION .
)
