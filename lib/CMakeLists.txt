add_subdirectory(jsmn)
add_subdirectory(minmea)
add_subdirectory(newlib)

target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PUBLIC
    jsmn
    minmea
)
