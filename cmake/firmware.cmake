# ---------------------------------------------------------------------------
# add_firmware(): bir lesson/proje icin calistirilabilir firmware uretir.
#
#   add_firmware(02-registers
#       SOURCES src/main.c
#       OPTIMIZATION -O0          # opsiyonel, lesson bazinda
#       DEFINES FOO=1             # opsiyonel
#   )
#
# Uretilenler:
#   <name>.elf   ELF (GDB ve QEMU bunu kullanir)
#   <name>.map   linker map file
#   <name>.bin   ham binary
#   <name>.lst   kaynak ile karisik disassembly (objdump -d -S)
#   + build sonunda arm-none-eabi-size ciktisi
#
# Ek hedefler:
#   run-<name>     QEMU'da calistir
#   debug-<name>   QEMU'yu durdurup GDB bekler (port 3333)
#   disasm-<name>  disassembly'i ekrana bas
# ---------------------------------------------------------------------------

find_program(QEMU_SYSTEM_ARM qemu-system-arm)

function(add_firmware name)
    cmake_parse_arguments(FW "" "OPTIMIZATION" "SOURCES;DEFINES;OPTIONS" ${ARGN})

    if(NOT FW_SOURCES)
        message(FATAL_ERROR "add_firmware(${name}): SOURCES bos olamaz")
    endif()

    add_executable(${name} ${FW_SOURCES})
    target_link_libraries(${name} PRIVATE platform-mps2-an505)

    if(FW_DEFINES)
        target_compile_definitions(${name} PRIVATE ${FW_DEFINES})
    endif()
    if(FW_OPTIONS)
        target_compile_options(${name} PRIVATE ${FW_OPTIONS})
    endif()
    if(FW_OPTIMIZATION)
        # Lesson kendi optimizasyon seviyesini zorlayabilir (ders 09).
        target_compile_options(${name} PRIVATE ${FW_OPTIMIZATION})
    endif()

    target_link_options(${name} PRIVATE
        "-Wl,-Map=$<TARGET_FILE_DIR:${name}>/${name}.map")

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${name}>
                $<TARGET_FILE_DIR:${name}>/${name}.bin
        COMMAND ${CMAKE_OBJDUMP} -d -S --demangle $<TARGET_FILE:${name}>
                > $<TARGET_FILE_DIR:${name}>/${name}.lst
        COMMAND ${CMAKE_SIZE_UTIL} $<TARGET_FILE:${name}>
        COMMENT "[${name}] bin + map + disassembly + size")

    if(QEMU_SYSTEM_ARM)
        add_custom_target(run-${name}
            COMMAND ${QEMU_SYSTEM_ARM} -M mps2-an505 -cpu cortex-m33
                    -nographic -monitor none -serial stdio
                    -kernel $<TARGET_FILE:${name}>
            DEPENDS ${name} USES_TERMINAL
            COMMENT "QEMU: ${name} calistiriliyor (cikis: Ctrl-A X)")

        add_custom_target(debug-${name}
            COMMAND ${QEMU_SYSTEM_ARM} -M mps2-an505 -cpu cortex-m33
                    -nographic -monitor none -serial stdio
                    -kernel $<TARGET_FILE:${name}> -S -gdb tcp::3333
            DEPENDS ${name} USES_TERMINAL
            COMMENT "QEMU: ${name} GDB bekliyor (port 3333)")
    endif()

    add_custom_target(disasm-${name}
        COMMAND ${CMAKE_COMMAND} -E cat $<TARGET_FILE_DIR:${name}>/${name}.lst
        DEPENDS ${name} USES_TERMINAL)
endfunction()
