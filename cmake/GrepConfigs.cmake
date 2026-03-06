add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/configs"
        "${CMAKE_CURRENT_BINARY_DIR}/configs"
)