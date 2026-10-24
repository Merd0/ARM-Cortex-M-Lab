set(L11_EX ${CMAKE_SOURCE_DIR}/lessons/11-startup/example)
set(L11_PLATFORM_DIR ${CMAKE_SOURCE_DIR}/platform/mps2-an505)
set(L11_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/layout.ld)
l11_add_custom_startup_firmware(project-reset
    SOURCES src/main.c ${L11_EX}/fixtures.c ${L11_EX}/l11_vectors.c
    ${CMAKE_SOURCE_DIR}/lessons/11-startup/solutions/01-reset-sequence/l11_reset.c)
target_link_options(project-reset PRIVATE -L${L11_PLATFORM_DIR}/linker)
