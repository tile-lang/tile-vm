cmake_minimum_required(VERSION 3.10)
project(tile VERSION 2.0)

set(CMAKE_C_STANDARD 11)

# include for external
include_directories(
)

# linked libs dir
link_directories(
)

# linked libs
set(LINK_LIBS
)

# src for external
file(GLOB EXTERN_SOURCES
)

# include
include_directories(
    "include"
)

# tvm
file(GLOB TVM_SOURCES
    "src/tvm.c"
)

# tasm
file(GLOB TASM_SOURCES
    "src/tasm.c"
)

# executable
add_executable(tvm ${TVM_SOURCES})
add_executable(tasm ${TASM_SOURCES})
# target_link_libraries(xxxx ${LINK_LIBS})