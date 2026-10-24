cmake_minimum_required(VERSION 3.21)

project(arm-cortex-m-lab
        VERSION 0.1.0
        DESCRIPTION "Practical Arm Cortex-M learning lab (QEMU + GDB)"
        LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

add_compile_options(-Wall -Wextra -Wshadow -Wundef)

include(cmake/firmware.cmake)

add_subdirectory(external)
add_subdirectory(platform/mps2-an505)
add_subdirectory(tools/capability-probe)
add_subdirectory(lessons)
add_subdirectory(projects)
add_subdirectory(final-project)

message(STATUS "-----------------------------------------------------")
message(STATUS " Target      : Cortex-M33 / QEMU mps2-an505")
message(STATUS " Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS " Compiler    : ${CMAKE_C_COMPILER}")
message(STATUS " QEMU        : ${QEMU_SYSTEM_ARM}")
message(STATUS "-----------------------------------------------------")
