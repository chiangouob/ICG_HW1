add_executable(ICG_2026_HW1
    "main.cpp"
    "std_image.cpp"
)

target_link_libraries(ICG_2026_HW1
    glfw
    glm::glm
    glad
    tinyobjloader
)

add_custom_command(TARGET ICG_2026_HW1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
)

add_custom_command(TARGET ICG_2026_HW1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/asset
    ${CMAKE_CURRENT_BINARY_DIR}/asset
)