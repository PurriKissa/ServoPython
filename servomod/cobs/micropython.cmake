add_library(usermod_cobs INTERFACE)

target_sources(usermod_cobs INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/cobs.c
)

target_include_directories(usermod_cobs INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(usermod INTERFACE usermod_cobs)
