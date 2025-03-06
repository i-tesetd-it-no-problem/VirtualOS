cmake_minimum_required(VERSION 3.21)

project(VirtualOS C ASM)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_STANDARD 11) 
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/virtualos_install)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -g3")
endif()

file(GLOB_RECURSE VIRTUALOS_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/component/RTT/*.c
    ${CMAKE_CURRENT_LIST_DIR}/dal/*.c
    ${CMAKE_CURRENT_LIST_DIR}/core/*.c
    ${CMAKE_CURRENT_LIST_DIR}/protocol/modbus/*.c
    ${CMAKE_CURRENT_LIST_DIR}/utils/*.c
    ${CMAKE_CURRENT_LIST_DIR}/driver/*.c
)

add_library(VirtualOS STATIC ${VIRTUALOS_SOURCES})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_FLAGS})

target_include_directories(VirtualOS PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/component/RTT/
)

install(
    TARGETS VirtualOS
    ARCHIVE DESTINATION lib
)

install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION include
)

install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/component/RTT/
    DESTINATION include/component/RTT
    FILES_MATCHING PATTERN "*.h"
)

install(
    FILES ${CMAKE_CURRENT_LIST_DIR}/core/virtual_os.ld
    DESTINATION ldscript
)

# rm -rf build;rm -rf virtualos_install;cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake;cmake --build build --target install;