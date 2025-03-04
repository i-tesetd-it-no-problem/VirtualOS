set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m0) # 关键修改部分

set(COMPILER_FLAGS
    -mcpu=${CMAKE_SYSTEM_PROCESSOR}
    -mthumb
    -mthumb-interwork
    -ffunction-sections
    -fdata-sections
    -fno-common
    -fmessage-length=0
)

set(CMAKE_C_COMPILER "arm-none-eabi-gcc" CACHE STRING "C Compiler" FORCE)
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++" CACHE STRING "C++ Compiler" FORCE)
set(CMAKE_ASM_COMPILER "arm-none-eabi-gcc" CACHE STRING "ASM Compiler" FORCE)
set(CMAKE_AR "arm-none-eabi-ar" CACHE STRING "Archiver" FORCE)
set(CMAKE_OBJCOPY "arm-none-eabi-objcopy" CACHE STRING "Objcopy" FORCE)
set(CMAKE_OBJDUMP "arm-none-eabi-objdump" CACHE STRING "Objdump" FORCE)
set(CMAKE_SIZE "arm-none-eabi-size" CACHE STRING "Size" FORCE)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)