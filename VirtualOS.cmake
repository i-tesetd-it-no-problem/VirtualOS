cmake_minimum_required(VERSION 3.21)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_STANDARD 11) 
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

# 默认Debug模式
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "Build type" FORCE)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
if(BUILD_TYPE_UPPER STREQUAL "DEBUG")
    add_compile_options(-Og -g)
elseif(BUILD_TYPE_UPPER STREQUAL "RELEASE")
    add_compile_options(-Os)
endif()

# VirtualOS 源文件
file(GLOB_RECURSE VIRTUALOS_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/component/RTT/*.c
    ${CMAKE_CURRENT_LIST_DIR}/dal/*.c
    ${CMAKE_CURRENT_LIST_DIR}/core/*.c
    ${CMAKE_CURRENT_LIST_DIR}/protocol/modbus/*.c
    ${CMAKE_CURRENT_LIST_DIR}/utils/*.c
    ${CMAKE_CURRENT_LIST_DIR}/driver/*.c
)

# 静态库
add_library(VirtualOS STATIC ${VIRTUALOS_SOURCES})

# VirtualOS 头文件
target_include_directories(VirtualOS PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/component/RTT/
)

# 链接器脚本
set(VIRTUAL_OS_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/core/virtualos.ld)

# 设置链接器选项
target_link_options(VirtualOS PUBLIC 
    -T${VIRTUAL_OS_LINKER_SCRIPT}
)