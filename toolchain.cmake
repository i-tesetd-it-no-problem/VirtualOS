# 设置 sysroot 路径（交叉编译工具链路径）
set(CMAKE_SYSROOT "C:/compile_environment/arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-arm-none-eabi")

###########################################
# 以下不需要修改

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(COMPILER_ROOT_PATH "${CMAKE_SYSROOT}/bin")

set(CMAKE_C_COMPILER "${COMPILER_ROOT_PATH}/arm-none-eabi-gcc.exe" CACHE STRING "C Compiler")
set(CMAKE_CXX_COMPILER "${COMPILER_ROOT_PATH}/arm-none-eabi-g++.exe" CACHE STRING "C++ Compiler")
set(CMAKE_ASM_COMPILER "${COMPILER_ROOT_PATH}/arm-none-eabi-gcc.exe" CACHE STRING "ASM Compiler")
set(CMAKE_AR "${COMPILER_ROOT_PATH}/arm-none-eabi-ar.exe" CACHE STRING "Archiver")
set(CMAKE_OBJCOPY "${COMPILER_ROOT_PATH}/arm-none-eabi-objcopy.exe" CACHE STRING "Objcopy")
set(CMAKE_OBJDUMP "${COMPILER_ROOT_PATH}/arm-none-eabi-objdump.exe" CACHE STRING "Objdump")
set(CMAKE_SIZE "${COMPILER_ROOT_PATH}/arm-none-eabi-size.exe" CACHE STRING "Size")

include_directories("${CMAKE_SYSROOT}/include")
include_directories("${CMAKE_SYSROOT}/arm-none-eabi/include")

set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)