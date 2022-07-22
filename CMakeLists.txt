# Set helper variables
set(OUT_DIR out)
set(TOOLCHAIN_DIR toolchain)

# Set extension for final binary and output directory
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${OUT_DIR}/${TYPE}/)

# Set default build type to "debug"
if(NOT DEFINED TYPE)
    set(TYPE debug)
ENDIF()

# Create the final executable 'firmware.elf'
add_executable(${CMAKE_PROJECT_NAME})

if(DEFINED ENV{FW_VERSION})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC FW_VERSION=\"$ENV{FW_VERSION}\")
endif()

if(DEFINED ENV{GIT_VERSION})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC GIT_VERSION=\"$ENV{GIT_VERSION}\")
endif()

string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S" UTC)
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC BUILD_DATE=\"${BUILD_DATE}\")

# Add debug CFLAGS
if(TYPE MATCHES "debug")
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -g3)
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -Og)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC DEBUG)
ENDIF()

# Add release CFLAGS
if(TYPE MATCHES "release")
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -Os)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC RELEASE)
ENDIF()

target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -T${CMAKE_CURRENT_SOURCE_DIR}/sys/lkr/stm32l083cz.ld)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -Wl,-Map=${CMAKE_SOURCE_DIR}/${OUT_DIR}/${TYPE}/${CMAKE_PROJECT_NAME}.map)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -mcpu=cortex-m0plus)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -mthumb)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -mlittle-endian)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -Wl,-lc)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -Wl,-lm)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -static)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -Wl,--gc-sections)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -Wl,--print-memory-usage)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -Wl,-u,__errno)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC --specs=nosys.specs)

# All other directories with source files
# You need to update the specific CMakeLists files in the respective directories if you want to update the SDK
add_subdirectory(twr)
add_subdirectory(bcl)
add_subdirectory(stm)
add_subdirectory(lib)
add_subdirectory(sys)

# Generate the final "firmware.bin" in "out" directory and root directory
include(${TOOLCHAIN_DIR}/utils.cmake)
generate_object(${CMAKE_PROJECT_NAME} .bin binary)
