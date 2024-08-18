set(CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "")
set(CMAKE_SYSTEM_NAME "Generic" CACHE STRING "")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 指定本地GCC编译器的bin文件路径,其他部分不需要修改
set(COMPILER_ROOT_PATH "D:/Tool/compile_environment/gcc-arm-none-eabi-10.3-2021.10/bin")

set(CMAKE_C_COMPILER "${COMPILER_ROOT_PATH}/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${COMPILER_ROOT_PATH}/arm-none-eabi-g++.exe")
set(CMAKE_ASM_COMPILER "${COMPILER_ROOT_PATH}/arm-none-eabi-gcc.exe")
set(CMAKE_AR "${COMPILER_ROOT_PATH}/arm-none-eabi-ar.exe")
set(CMAKE_OBJCOPY "${COMPILER_ROOT_PATH}/arm-none-eabi-objcopy.exe")
set(CMAKE_OBJDUMP "${COMPILER_ROOT_PATH}/arm-none-eabi-objdump.exe")
set(CMAKE_SIZE "${COMPILER_ROOT_PATH}/arm-none-eabi-size.exe")
set(CMAKE_LINKER "${COMPILER_ROOT_PATH}/arm-none-eabi-ld.exe")
