# Lesson 10 firmware hedefleri - her biri TEK bir ogretim amacina hizmet eder.
#
#   10-linker-elf-memory   ayri derlenmis parcalar -> tek ELF; yerlesim,
#                          semboller, relocation'lar, VMA/LMA, map
#   10-linker-broken       BILEREK link EDILEMEZ: asm fonksiyonu .global degil
#                          (ALL'a dahil DEGIL; tests/build-probes.sh dogrular)
#   10-exercise-linker     "Locate Your Data" - ogrenci locate.c'yi yazar
#   10-solution-linker     referans cozum
#
# Bu ders paylasilan linker script'i (platform/.../mps2-an505-secure.ld)
# DEGISTIRMEZ; onu inceler.

set(L10_EX ${CMAKE_CURRENT_SOURCE_DIR}/example)

set(L10_OBJECTS
    ${L10_EX}/objects.c
    ${L10_EX}/placement.c)

add_firmware(10-linker-elf-memory
    SOURCES ${L10_EX}/main.c ${L10_OBJECTS} ${L10_EX}/l10_asm.S)

add_firmware(10-linker-broken
    SOURCES ${L10_EX}/main.c ${L10_OBJECTS} ${L10_EX}/l10_asm_broken.S)
set_target_properties(10-linker-broken PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_firmware(10-exercise-linker
    SOURCES ${L10_EX}/locate_main.c ${L10_OBJECTS}
            exercises/01-locate-your-data/locate.c)

add_firmware(10-solution-linker
    SOURCES ${L10_EX}/locate_main.c ${L10_OBJECTS}
            solutions/01-locate-your-data/locate.c
    DEFINES L10_SOLUTION=1)

foreach(t 10-linker-elf-memory 10-linker-broken 10-exercise-linker 10-solution-linker)
    target_include_directories(${t} PRIVATE ${L10_EX})
endforeach()
