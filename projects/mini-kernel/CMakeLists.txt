set(kernel ${CMAKE_SOURCE_DIR}/lessons/15-mini-kernel)
add_firmware(project-kernel SOURCES ${kernel}/example/main.c ${kernel}/example/context.S
    ${kernel}/example/frame.c src/application.c OPTIONS -mgeneral-regs-only)
target_link_options(project-kernel PRIVATE -Wl,-z,noexecstack)
