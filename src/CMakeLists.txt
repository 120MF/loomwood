file(GLOB_RECURSE LOOM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
target_sources(${PROJECT_NAME} PUBLIC ${LOOM_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PUBLIC
        SDL3::SDL3 spdlog::spdlog EnTT::EnTT tomlplusplus::tomlplusplus rmlui_core
        stdc++exp
)