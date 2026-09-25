# Lesson 09 firmware hedefleri - her biri TEK bir ogretim amacina hizmet eder.
#
#   09-compiler-aapcs          sozlesmeyi OLC: arguman yerlesimi, donus,
#                              cagri sinirinda SP, callee-saved harness
#   09-compiler-aapcs-broken   dogru sonuc, BOZUK sozlesme (r4)
#   09-compiler-optimization   ayni C kaynagi, uc preset: .lst karsilastirmasi
#   09-exercise-callee-save    ogrenci ABI'yi bozuk bir asm fonksiyonu onarir
#   09-solution-callee-save    referans cozum
#   09-harness-sp-regression   OLCUM ALETI regresyonu (ogrenciye yonelik degil):
#                              harness, SP'yi bozan bir callee'den sag cikip
#                              bunu raporlayabiliyor mu?
#
# Olcum aracinin (abi_harness.S) ve C yardimcilarinin (helpers.c) TEK bir
# kaynagi var: hedefler onlari example/ altindan paylasir. Bir olcum
# aletinin iki kopyasi zamanla birbirinden ayrisabilirdi.

set(L09_EX ${CMAKE_CURRENT_SOURCE_DIR}/example)

set(L09_COMMON
    ${L09_EX}/abi_harness.S
    ${L09_EX}/good_callee.S
    ${L09_EX}/helpers.c)

add_firmware(09-compiler-aapcs
    SOURCES ${L09_EX}/main.c ${L09_EX}/abi_probes.S ${L09_COMMON})

add_firmware(09-compiler-aapcs-broken
    SOURCES ${L09_EX}/main.c ${L09_EX}/abi_probes.S ${L09_COMMON}
            ${L09_EX}/bad_callee.S
    DEFINES L09_BROKEN=1)

add_firmware(09-compiler-optimization
    SOURCES ${L09_EX}/optimization.c ${L09_EX}/helpers.c
    DEFINES L09_EXPECTED_CHECKSUM=0x79DC404Cu)

add_firmware(09-exercise-callee-save
    SOURCES exercises/01-callee-save/main.c
            exercises/01-callee-save/scale_and_sum.S ${L09_COMMON})

add_firmware(09-solution-callee-save
    SOURCES solutions/01-callee-save/main.c
            solutions/01-callee-save/scale_and_sum.S ${L09_COMMON})

add_firmware(09-harness-sp-regression
    SOURCES regression/main.c regression/sp_corruption.S ${L09_COMMON})

foreach(t 09-compiler-aapcs 09-compiler-aapcs-broken 09-compiler-optimization
          09-exercise-callee-save 09-solution-callee-save
          09-harness-sp-regression)
    target_include_directories(${t} PRIVATE ${L09_EX})
endforeach()
