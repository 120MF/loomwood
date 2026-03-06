file(GLOB CONFIG_RESOURCES "${PROJECT_SOURCE_DIR}/config/*.toml")

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CONFIG_RESOURCES})

# Copy config resources into the build tree so they are available under
# ${CMAKE_CURRENT_BINARY_DIR}/config during/after configure.
set(LOOM_CONFIG_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/config")
file(MAKE_DIRECTORY "${LOOM_CONFIG_OUTPUT_DIR}")

foreach (_cfg IN LISTS CONFIG_RESOURCES)
    get_filename_component(_cfg_name "${_cfg}" NAME)
    configure_file("${_cfg}" "${LOOM_CONFIG_OUTPUT_DIR}/${_cfg_name}" COPYONLY)
endforeach ()
