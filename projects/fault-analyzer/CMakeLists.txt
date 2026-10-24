set(fa_decoder ${CMAKE_SOURCE_DIR}/lessons/08-faults/solutions/01-fault-report/decode.c)
add_library(lab-diagnostic INTERFACE)
target_sources(lab-diagnostic INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/diagnostic.c ${fa_decoder})
target_include_directories(lab-diagnostic INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lessons/08-faults/example)
add_firmware(project-fault-cases SOURCES src/cases.c)
target_link_libraries(project-fault-cases PRIVATE lab-diagnostic)
foreach(kind IN ITEMS udf unaligned bus mpu escalated)
    add_firmware(project-fault-${kind} SOURCES src/live.c src/entry.S
        ${CMAKE_SOURCE_DIR}/lessons/06-mpu/example/psp_start.S
        DEFINES FA_${kind}=1 OPTIONS -mgeneral-regs-only)
    target_link_libraries(project-fault-${kind} PRIVATE lab-diagnostic)
endforeach()
