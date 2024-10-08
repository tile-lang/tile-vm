cmake_minimum_required(VERSION 3.10)
project(tile VERSION 0.1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_GENERATOR_PLATFORM x86)

set(STB_INCLUDE_DIR "extern/stb/include")

if (WIN32)
    if (MSVC)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
        set(LIBFFI_INCLUDE_DIR "extern/libffi-msvc/include")
        set(LIBFFI_LIB_DIR "extern/libffi-msvc/lib")
        set (LIBFFI_LIBS "ffi.lib")
    else()  # MINGW
        message("LIBFFI: MINGW")
        set(LIBFFI_INCLUDE_DIR "extern/libffi-mingw32/include") # or libffi-mingw64
        set(LIBFFI_LIB_DIR "extern/libffi-mingw32/lib")
        set (LIBFFI_LIBS "libffi")
    endif()
else()  # Linux or other platforms
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBFFI REQUIRED libffi)
    set(LIBFFI_INCLUDE_DIR ${LIBFFI_INCLUDE_DIRS})
    set(LIBFFI_LIB_DIR ${LIBFFI_LIBRARY_DIRS})
    set(LIBFFI_LIBS ${LIBFFI_LIBRARIES})

    list(APPEND LIBFFI_LIBS dl)
endif()

# Include directories
include_directories(
    ${STB_INCLUDE_DIR}
    ${LIBFFI_INCLUDE_DIR}
)

# Link directories
link_directories(
    ${LIBFFI_LIB_DIR}
)

# Linked libraries
set(LINK_LIBS
    ${LIBFFI_LIBS}
)

# Source files for external libraries
file(GLOB EXTERN_SOURCES
)

# Include directories for project
include_directories(
    "include"
)

# Source files for tvm
file(GLOB TVM_SOURCES
    "src/tvm.c"
    "src/tci.c"
)

# Source files for tasm
file(GLOB TASM_SOURCES
    "src/tasm.c"
)

# Add executables
add_executable(tvm ${TVM_SOURCES})
add_executable(tasm ${TASM_SOURCES})

if(MSVC)
    # target_link_options(tvm PRIVATE "/INCREMENTAL:YES")
endif()

# Link libraries to tvm
target_link_libraries(tvm ${LINK_LIBS})
