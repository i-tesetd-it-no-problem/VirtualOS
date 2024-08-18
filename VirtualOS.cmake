cmake_minimum_required(VERSION 3.21)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 默然设置Debug模式
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "Build type" FORCE)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)

if(BUILD_TYPE_UPPER STREQUAL "DEBUG")
    add_compile_options(-Og -g)
elseif(BUILD_TYPE_UPPER STREQUAL "RELEASE")
    add_compile_options(-Os)
endif()

# 框架所有头文件
set(FRAMEWORK_INCLUDE_DIRS
    ${FRAMEWORK_ROOT_DIR}/Component/FAL/
    ${FRAMEWORK_ROOT_DIR}/Component/FlashDB/
    ${FRAMEWORK_ROOT_DIR}/Component/LittleFs/
    ${FRAMEWORK_ROOT_DIR}/Component/RTT/
    ${FRAMEWORK_ROOT_DIR}/Protocol/modbus/
    ${FRAMEWORK_ROOT_DIR}/Driver/arm_core/
    ${FRAMEWORK_ROOT_DIR}/DAL/
    ${FRAMEWORK_ROOT_DIR}/VFS/
    ${FRAMEWORK_ROOT_DIR}/Utilities/button/
    ${FRAMEWORK_ROOT_DIR}/Utilities/hash/string_hash/
    ${FRAMEWORK_ROOT_DIR}/Utilities/crc/
    ${FRAMEWORK_ROOT_DIR}/Utilities/list/
    ${FRAMEWORK_ROOT_DIR}/Utilities/qfsm/
    ${FRAMEWORK_ROOT_DIR}/Utilities/queue/
    ${FRAMEWORK_ROOT_DIR}/Utilities/slog/
    ${FRAMEWORK_ROOT_DIR}/Utilities/soft_iic/
    ${FRAMEWORK_ROOT_DIR}/Utilities/stimer/
    ${FRAMEWORK_ROOT_DIR}/Utilities/simple_shell/
    ${FRAMEWORK_ROOT_DIR}/Utilities/h_tree/
    ${FRAMEWORK_ROOT_DIR}/Driver/system/
)

# 框架所需最小头文件(具体项目具体添加)
file(GLOB_RECURSE BASE_SOURCES
    ${FRAMEWORK_ROOT_DIR}/DAL/*.c
    ${FRAMEWORK_ROOT_DIR}/VFS/*.c
    ${FRAMEWORK_ROOT_DIR}/Utilities/hash/string_hash/*.c
    ${FRAMEWORK_ROOT_DIR}/Utilities/list/*.c
    ${FRAMEWORK_ROOT_DIR}/Utilities/queue/*.c
    ${FRAMEWORK_ROOT_DIR}/Utilities/stimer/*.c
    ${FRAMEWORK_ROOT_DIR}/Utilities/h_tree/*.c
    ${FRAMEWORK_ROOT_DIR}/Utilities/simple_shell/*.c
    ${FRAMEWORK_ROOT_DIR}/Driver/system/*.c
)

# 设置交叉编译工具链
set(CMAKE_TOOLCHAIN_FILE ${FRAMEWORK_ROOT_DIR}/toolchain.cmake)

# 包含目录
include_directories(${FRAMEWORK_INCLUDE_DIRS})
