file(GLOB_RECURSE VIRTUALOS_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/component/RTT/*.c
    ${CMAKE_CURRENT_LIST_DIR}/dal/*.c
    ${CMAKE_CURRENT_LIST_DIR}/core/*.c
    ${CMAKE_CURRENT_LIST_DIR}/protocol/modbus/*.c
    ${CMAKE_CURRENT_LIST_DIR}/utils/*.c
    ${CMAKE_CURRENT_LIST_DIR}/driver/*.c
)

set(VIRTUALOS_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/component/RTT
)

# add_link_options(
#     -T${CMAKE_CURRENT_LIST_DIR}/core/virtual_os.ld
# )