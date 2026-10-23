set(L12_COMMON example/main.c example/debug_chain.c)
add_firmware(12-debug-dwt SOURCES ${L12_COMMON} example/workload.c)
add_firmware(12-debug-dwt-broken SOURCES ${L12_COMMON} example/workload.c
    DEFINES L12_BROKEN_STORE=1)
add_firmware(12-profile-io SOURCES ${L12_COMMON} example/workload.c
    DEFINES L12_IO_IN_WINDOW=1)

# Only workload.c changes optimization; startup, console and driver keep the preset.
foreach(opt IN ITEMS o0 o2)
    add_library(l12-work-${opt} OBJECT example/workload.c)
    if(opt STREQUAL "o0")
        target_compile_options(l12-work-${opt} PRIVATE -O0)
    else()
        target_compile_options(l12-work-${opt} PRIVATE -O2)
    endif()
    add_firmware(12-profile-${opt} SOURCES ${L12_COMMON}
        $<TARGET_OBJECTS:l12-work-${opt}>)
endforeach()

add_firmware(12-exercise-measurement SOURCES exercise_driver.c
    exercises/01-valid-measurement/measurement.c)
add_firmware(12-solution-measurement SOURCES exercise_driver.c
    solutions/01-valid-measurement/measurement.c)

# Small, deliberately compatible API subset: compare emitted code, not headers.
foreach(api IN ITEMS cmsis teaching)
    add_library(l12-api-${api} OBJECT example/header_probe.c)
    target_include_directories(l12-api-${api} PRIVATE
        ${CMAKE_SOURCE_DIR}/core/include
        ${CMAKE_SOURCE_DIR}/platform/mps2-an505/include)
    target_link_libraries(l12-api-${api} PRIVATE cmsis-core)
    if(api STREQUAL "teaching")
        target_compile_definitions(l12-api-${api} PRIVATE L12_TEACHING=1)
    endif()
endforeach()
