# Toolchain file that takes care of the cross compilation for the Core Module

# Setup cross compilation
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Setup how the toolchain should be located
if(MINGW OR CYGWIN OR WIN32)
    set(UTIL_SEARCH_CMD where)
elseif(UNIX OR APPLE)
    set(UTIL_SEARCH_CMD which)
endif()

# Set selected toolchain prefix
set(TOOLCHAIN_PREFIX arm-none-eabi-)

# Locate the toolchain
execute_process(
  COMMAND ${UTIL_SEARCH_CMD} ${TOOLCHAIN_PREFIX}gcc
  OUTPUT_VARIABLE BINUTILS_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
get_filename_component(ARM_TOOLCHAIN_DIR ${BINUTILS_PATH} DIRECTORY)

# Without that flag CMake is not able to pass test compilation check
if (${CMAKE_VERSION} VERSION_EQUAL "3.6.0" OR ${CMAKE_VERSION} VERSION_GREATER "3.6")
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
else()
    MESSAGE( STATUS "CMAKE_C_FLAGS: " ${CMAKE_CURRENT_SOURCE_DIR} )
endif()

# Set all compilers
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

# Setup default CFLAGS
add_definitions("-D__weak=__attribute__((weak))")
add_definitions("-D__packed=__attribute__((__packed__))")
add_definitions("-DUSE_HAL_DRIVER")
add_definitions("-DSTM32L083xx")
add_definitions("-DHAL_IWDG_MODULE_ENABLED")

add_compile_options(-mcpu=cortex-m0plus)
add_compile_options(-mthumb)
add_compile_options(-mlittle-endian)
add_compile_options(-Wall)
add_compile_options(-pedantic)
add_compile_options(-Wextra)
add_compile_options(-Wmissing-include-dirs)
add_compile_options(-Wswitch-default)
add_compile_options(-Wswitch-enum)
add_compile_options(-ffunction-sections)
add_compile_options(-fdata-sections)
add_compile_options(-std=c11)

# Setup HARDWARIO TOWER special CFLAGS
if(DEFINED SCHEDULER_INTERVAL)
    add_definitions("-DTWR_SCHEDULER_INTERVAL_MS=${SCHEDULER_INTERVAL}")
endif()

add_definitions("-DBAND=868")

# Setup utils
set(CMAKE_OBJCOPY ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")

set(CMAKE_SYSROOT ${ARM_TOOLCHAIN_DIR}/../arm-none-eabi)
set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
