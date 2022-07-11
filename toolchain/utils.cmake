# File with some aditional functions and macros that can be used in any CMakeLists file

# Add a command to generate firmare.bin in "out" folder and in root folder
function(generate_object target suffix type)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND echo "Creating ${OUT_DIR}/${TYPE}/${target}${suffix} from ${OUT_DIR}/${TYPE}/${target}${CMAKE_EXECUTABLE_SUFFIX}..."
        COMMAND ${CMAKE_OBJCOPY} -O ${type}
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target}${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_SOURCE_DIR}/${OUT_DIR}/${TYPE}/${target}${suffix}"
        COMMAND ${CMAKE_OBJCOPY} -O ${type}
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target}${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_SOURCE_DIR}/${target}${suffix}"
    )
endfunction()
