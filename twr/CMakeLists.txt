add_subdirectory(src)
add_subdirectory(stm)

target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PUBLIC
    inc
)
