add_subdirectory(jsmn)
add_subdirectory(minmea)

target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PUBLIC
    jsmn
    minmea
)
