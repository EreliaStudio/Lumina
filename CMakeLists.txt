cmake_minimum_required(VERSION 3.10)
project(Lumina)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Base directories
set(LUMINA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LUMINA_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# Collect all header and source files recursively
file(GLOB_RECURSE LUMINA_HEADERS
    ${LUMINA_INCLUDE_DIR}/*.hpp
    ${LUMINA_INCLUDE_DIR}/*.h
)
file(GLOB_RECURSE LUMINA_SOURCES ${LUMINA_SRC_DIR}/*.cpp)

# Add the executable target
add_executable(Lumina ${LUMINA_SOURCES} ${LUMINA_HEADERS})

target_include_directories(Lumina PUBLIC ${LUMINA_INCLUDE_DIR})

# Installation rules
# Install the executable to 
install(TARGETS Lumina DESTINATION .)

# Install the contents of predefined_header to includes
install(DIRECTORY ${PROJECT_SOURCE_DIR}/predefined_header/ DESTINATION includes)
