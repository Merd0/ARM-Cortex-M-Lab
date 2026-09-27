# Lesson 11 firmware hedefleri - her biri TEK bir ogretim amacina hizmet eder.
#
#   11-startup           repo'nun GERCEK paylasilan startup yolunu gozler
#                        (add_firmware; startup_mps2_an505.c + system_init.c)
#   11-startup-broken    ders-yerel startup, .bss sifirlamasi BILEREK yok
#   11-exercise-startup  "Complete the Reset Sequence" - ogrenci doldurur
#   11-solution-startup  referans cozum
#
# Paylasilan altyapiya DOKUNULMAZ.

set(L11_EX ${CMAKE_CURRENT_SOURCE_DIR}/example)

# ---------------------------------------------------------------------------
# 1) Gercek startup yolu: normal add_firmware
# ---------------------------------------------------------------------------
add_firmware(11-startup
    SOURCES ${L11_EX}/main.c ${L11_EX}/main_entry.S ${L11_EX}/fixtures.c)
target_include_directories(11-startup PRIVATE ${L11_EX})

# ---------------------------------------------------------------------------
# 2) Ders-yerel ozel startup hedefleri
#
# NEDEN AYRI BIR YARDIMCI?
#   add_firmware() her hedefi platform-mps2-an505 INTERFACE kutuphanesine
#   baglar; o kutuphane startup_mps2_an505.c'yi KAYNAK olarak ekler.
#   Oradaki Reset_Handler ve __vector_table WEAK degil. Kendi Reset_Handler'ini
#   getiren bir hedef add_firmware ile "multiple definition" hatasi alir.
#
#   Bu yardimci, platform kutuphanesinin YALNIZCA startup DISINDAKI parcalarini
#   ve add_firmware'in yan urunlerini (.bin/.map/.lst/size, run-/debug-/disasm-)
#   ders-yerel olarak tekrarlar. Paylasilan CMake'e dokunmaz.
#
#   Derleyici/CPU/ABI bayraklari global CMAKE_C_FLAGS / CMAKE_ASM_FLAGS /
#   CMAKE_EXE_LINKER_FLAGS'ten (toolchain dosyasi) gelir - add_firmware ile
#   AYNI. Tests/build-probes.sh, bu hedefin paylasilan startup nesnesini link
#   etmedigini ve beklenen yan urunleri urettigini denetler.
# ---------------------------------------------------------------------------
set(L11_PLATFORM_DIR ${CMAKE_SOURCE_DIR}/platform/mps2-an505)
set(L11_LINKER_SCRIPT ${L11_PLATFORM_DIR}/linker/mps2-an505-secure.ld)

function(l11_add_custom_startup_firmware name)
    cmake_parse_arguments(FW "" "" "SOURCES;DEFINES" ${ARGN})

    add_executable(${name}
        ${FW_SOURCES}
        # platform-mps2-an505 kaynaklari - startup_mps2_an505.c HARIC
        ${L11_PLATFORM_DIR}/src/system_init.c
        ${L11_PLATFORM_DIR}/src/uart_cmsdk.c
        ${CMAKE_SOURCE_DIR}/common/src/console.c
        ${CMAKE_SOURCE_DIR}/common/src/tiny_libc.c)

    target_link_libraries(${name} PRIVATE cmsis-core gcc)
    target_include_directories(${name} PRIVATE
        ${L11_EX}
        ${L11_PLATFORM_DIR}/include
        ${CMAKE_SOURCE_DIR}/core/include
        ${CMAKE_SOURCE_DIR}/common/include)
    if(FW_DEFINES)
        target_compile_definitions(${name} PRIVATE ${FW_DEFINES})
    endif()

    target_link_options(${name} PRIVATE
        -T${L11_LINKER_SCRIPT}
        "-Wl,-Map=$<TARGET_FILE_DIR:${name}>/${name}.map")
    set_property(TARGET ${name} APPEND PROPERTY LINK_DEPENDS ${L11_LINKER_SCRIPT})

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${name}>
                $<TARGET_FILE_DIR:${name}>/${name}.bin
        COMMAND ${CMAKE_OBJDUMP} -d -S --demangle $<TARGET_FILE:${name}>
                > $<TARGET_FILE_DIR:${name}>/${name}.lst
        COMMAND ${CMAKE_SIZE_UTIL} $<TARGET_FILE:${name}>
        COMMENT "[${name}] bin + map + disassembly + size (ders-yerel startup)")

    if(QEMU_SYSTEM_ARM)
        add_custom_target(run-${name}
            COMMAND ${QEMU_SYSTEM_ARM} -M mps2-an505 -cpu cortex-m33
                    -nographic -monitor none -serial stdio
                    -kernel $<TARGET_FILE:${name}>
            DEPENDS ${name} USES_TERMINAL)
        add_custom_target(debug-${name}
            COMMAND ${QEMU_SYSTEM_ARM} -M mps2-an505 -cpu cortex-m33
                    -nographic -monitor none -serial stdio
                    -kernel $<TARGET_FILE:${name}> -S -gdb tcp::3333
            DEPENDS ${name} USES_TERMINAL)
    endif()
    add_custom_target(disasm-${name}
        COMMAND ${CMAKE_COMMAND} -E cat $<TARGET_FILE_DIR:${name}>/${name}.lst
        DEPENDS ${name} USES_TERMINAL)
endfunction()

set(L11_CUSTOM_COMMON
    ${L11_EX}/l11_vectors.c
    ${L11_EX}/l11_main.c
    ${L11_EX}/fixtures.c)

l11_add_custom_startup_firmware(11-startup-broken
    SOURCES ${L11_EX}/l11_reset.c ${L11_CUSTOM_COMMON}
    DEFINES L11_BROKEN_SKIP_BSS=1
            "L11_BANNER=\"LESSON 11: STARTUP (BROKEN: .bss sifirlanmiyor)\"")

l11_add_custom_startup_firmware(11-exercise-startup
    SOURCES exercises/01-reset-sequence/l11_reset.c ${L11_CUSTOM_COMMON}
    DEFINES "L11_BANNER=\"LESSON 11 EXERCISE 01\"")

l11_add_custom_startup_firmware(11-solution-startup
    SOURCES solutions/01-reset-sequence/l11_reset.c ${L11_CUSTOM_COMMON}
    DEFINES "L11_BANNER=\"LESSON 11 SOLUTION 01\"")
