foreach(variant IN ITEMS normal denied)
    add_firmware(project-protected-${variant} SOURCES src/main.c
        ${CMAKE_SOURCE_DIR}/lessons/06-mpu/example/svc_handler.S
        ${CMAKE_SOURCE_DIR}/lessons/06-mpu/example/psp_start.S
        ${CMAKE_SOURCE_DIR}/lessons/06-mpu/example/mpu_probe.S
        DEFINES APP_${variant}=1 OPTIONS -mgeneral-regs-only)
    target_link_libraries(project-protected-${variant} PRIVATE lab-diagnostic)
endforeach()
