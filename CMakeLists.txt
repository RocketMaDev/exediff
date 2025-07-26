cmake_minimum_required(VERSION 3.10)
project(exediff_exepatch)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define directories
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_SOURCE_DIR}/include")
set(BUILD_DIR "${CMAKE_BINARY_DIR}")

# Include directories
include_directories(${INC_DIR})

# Find all C source files except exediff.c and exepatch.c
file(GLOB_RECURSE ALL_C_SRCS "${SRC_DIR}/*.c")
list(REMOVE_ITEM ALL_C_SRCS "${SRC_DIR}/exediff.c" "${SRC_DIR}/exepatch.c")

# Set common compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpie -fstack-protector -Wall -Wextra")

# Set common link flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z now -z noexecstack -fpie -fstack-protector")

# Handle DEBUG options
if(DEFINED DEBUG)
    if(DEBUG EQUAL 1)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -O2")
    elseif(DEBUG EQUAL 2)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")
    endif()
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O2")
endif()

# Find required libraries
find_library(CAPSTONE_LIB capstone REQUIRED)
find_library(ELF_LIB elf REQUIRED)
find_library(KEYSTONE_LIB keystone REQUIRED)

# Create exediff executable
if(EXISTS "${SRC_DIR}/exediff.c")
    add_executable(exediff ${ALL_C_SRCS} "${SRC_DIR}/exediff.c")
    target_link_libraries(exediff ${CAPSTONE_LIB} ${ELF_LIB} ${KEYSTONE_LIB})
endif()

# Create exepatch executable
if(EXISTS "${SRC_DIR}/exepatch.c")
    add_executable(exepatch ${ALL_C_SRCS} "${SRC_DIR}/exepatch.c")
    target_link_libraries(exepatch ${CAPSTONE_LIB} ${ELF_LIB} ${KEYSTONE_LIB})
endif()

# Set output directory for executables
set_target_properties(exediff exepatch PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}
)

# Custom clean target (CMake handles this automatically, but kept for reference)
# Use: make clean or ninja clean
