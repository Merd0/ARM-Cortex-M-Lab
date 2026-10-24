# Dual images use lesson-local memory partitions; shared platform is unchanged.
set(l16_platform ${CMAKE_SOURCE_DIR}/platform/mps2-an505)
function(l16_pair name policy)
    cmake_parse_arguments(APP "" "" "SECURE_SOURCES;NS_SOURCES;DEFINES" ${ARGN})
    set(l16_platform ${CMAKE_SOURCE_DIR}/platform/mps2-an505)
    set(l16_root ${CMAKE_SOURCE_DIR}/lessons/16-trustzone-advanced)
    set(secure ${name}-secure)
    set(nonsecure ${name}-nonsecure)
    set(imports ${CMAKE_CURRENT_BINARY_DIR}/${name}-imports.o)
    add_executable(${secure} ${l16_root}/secure/main.c ${policy} ${APP_SECURE_SOURCES}
        ${l16_platform}/src/startup_mps2_an505.c
        ${l16_platform}/src/system_init.c ${l16_platform}/src/uart_cmsdk.c
        ${CMAKE_SOURCE_DIR}/common/src/console.c ${CMAKE_SOURCE_DIR}/common/src/tiny_libc.c)
    target_link_libraries(${secure} PRIVATE cmsis-core gcc)
    target_include_directories(${secure} PRIVATE ${l16_platform}/include ${CMAKE_SOURCE_DIR}/common/include)
    target_compile_options(${secure} PRIVATE -mcmse -mgeneral-regs-only)
    target_link_options(${secure} PRIVATE -mcmse
        -T${l16_root}/secure/memory.ld
        -Wl,--cmse-implib,--out-implib=${imports}
        -Wl,--undefined=l16_service,--undefined=l16_report
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${secure}.map)
    set_property(TARGET ${secure} APPEND PROPERTY LINK_DEPENDS ${l16_root}/secure/memory.ld)
    add_custom_command(TARGET ${secure} POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} $<TARGET_FILE:${secure}>
        BYPRODUCTS ${imports})
    if(APP_NS_SOURCES)
        add_executable(${nonsecure} ${APP_NS_SOURCES} ${l16_root}/nonsecure/startup.c)
    else()
        add_executable(${nonsecure} ${l16_root}/nonsecure/main.c ${l16_root}/nonsecure/startup.c)
    endif()
    target_compile_definitions(${secure} PRIVATE ${APP_DEFINES})
    target_compile_definitions(${nonsecure} PRIVATE ${APP_DEFINES})
    target_compile_options(${nonsecure} PRIVATE -mgeneral-regs-only)
    target_link_libraries(${nonsecure} PRIVATE ${imports} gcc)
    target_link_options(${nonsecure} PRIVATE -T${l16_root}/nonsecure/memory.ld
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${nonsecure}.map)
    set_property(TARGET ${nonsecure} APPEND PROPERTY LINK_DEPENDS ${l16_root}/nonsecure/memory.ld)
    add_dependencies(${nonsecure} ${secure})
    if(name STREQUAL "16-invalid-entry")
        target_compile_definitions(${secure} PRIVATE L16_INVALID_ENTRY=1)
        target_compile_definitions(${nonsecure} PRIVATE L16_INVALID_ENTRY=1)
    endif()
    foreach(target ${secure} ${nonsecure})
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${target}> ${CMAKE_CURRENT_BINARY_DIR}/${target}.bin
            COMMAND ${CMAKE_OBJDUMP} -d -S $<TARGET_FILE:${target}> > ${CMAKE_CURRENT_BINARY_DIR}/${target}.lst)
    endforeach()
endfunction()
l16_pair(16-trustzone example/policy.c)
l16_pair(16-invalid-entry example/policy.c)
l16_pair(16-exercise-policy exercises/01-secure-service/policy.c)
l16_pair(16-solution-policy solutions/01-secure-service/policy.c)
