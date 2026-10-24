set(node_sources ${CMAKE_CURRENT_SOURCE_DIR}/nonsecure/kernel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nonsecure/tasks.S ${CMAKE_CURRENT_SOURCE_DIR}/nonsecure/vectors.c
    ${CMAKE_SOURCE_DIR}/lessons/15-mini-kernel/example/context.S
    ${CMAKE_SOURCE_DIR}/lessons/15-mini-kernel/example/frame.c
    ${CMAKE_SOURCE_DIR}/common/src/console.c ${CMAKE_SOURCE_DIR}/common/src/tiny_libc.c)
set(number 0)
foreach(mode IN ITEMS normal kernel-write cross-task secure-read stack-limit spin bad-svc)
    l16_pair(node-${mode} ${CMAKE_CURRENT_SOURCE_DIR}/secure/policy.c
        SECURE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/secure/service.c ${CMAKE_CURRENT_SOURCE_DIR}/secure/process.S
        NS_SOURCES ${node_sources}
        DEFINES NODE_TEST=${number} L15_NONSECURE=1 L15_UNPRIV=1 L15_CUSTOM_TASKS=1 L16_EXTERNAL_VECTORS=1)
    target_link_libraries(node-${mode}-nonsecure PRIVATE cmsis-core lab-diagnostic)
    target_include_directories(node-${mode}-nonsecure PRIVATE
        ${CMAKE_SOURCE_DIR}/platform/mps2-an505/include ${CMAKE_SOURCE_DIR}/common/include)
    target_link_options(node-${mode}-nonsecure PRIVATE -Wl,-z,noexecstack)
    target_link_options(node-${mode}-secure PRIVATE -Wl,-z,noexecstack)
    math(EXPR number "${number}+1")
endforeach()
