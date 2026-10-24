foreach(opt IN ITEMS o0 o1 o2 o3 os)
    add_library(project-work-${opt} OBJECT ${CMAKE_SOURCE_DIR}/lessons/12-debug-dwt/example/workload.c)
    string(SUBSTRING ${opt} 1 1 level)
    target_compile_options(project-work-${opt} PRIVATE -O${level})
    add_firmware(project-profile-${opt} SOURCES src/main.c
        ${CMAKE_SOURCE_DIR}/lessons/12-debug-dwt/solutions/01-valid-measurement/measurement.c
        $<TARGET_OBJECTS:project-work-${opt}>)
endforeach()
