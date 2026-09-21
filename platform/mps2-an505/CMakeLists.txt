# Platform katmani INTERFACE library olarak tanimlanir.
#
# Neden INTERFACE (onceden derlenmis static library degil)?
# Cunku bazi dersler farkli derleyici flag'leri kullanir (ornegin ders 13
# FPU'yu hard-float ile acar). Ayni onceden derlenmis kutuphaneyi farkli
# ABI ile link etmek sessiz hatalara yol acar. INTERFACE ile her firmware
# platform kaynaklarini KENDI flag'leriyle derler.

set(PLATFORM_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LINKER_SCRIPT ${PLATFORM_DIR}/linker/mps2-an505-secure.ld)

add_library(platform-mps2-an505 INTERFACE)

# Core API: resmi CMSIS-Core. Elle yazilmis core/cm33_teaching.h yalnizca
# lesson firmware'lerinin istege bagli olarak kullandigi egitim basligidir.
target_link_libraries(platform-mps2-an505 INTERFACE cmsis-core)

target_sources(platform-mps2-an505 INTERFACE
    ${PLATFORM_DIR}/src/startup_mps2_an505.c
    ${PLATFORM_DIR}/src/system_init.c
    ${PLATFORM_DIR}/src/uart_cmsdk.c
    ${CMAKE_SOURCE_DIR}/common/src/console.c
    ${CMAKE_SOURCE_DIR}/common/src/tiny_libc.c)

target_include_directories(platform-mps2-an505 INTERFACE
    ${PLATFORM_DIR}/include
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/common/include)

target_link_options(platform-mps2-an505 INTERFACE
    -T${LINKER_SCRIPT})

# libgcc: derleyicinin urettigi yardimci rutinler (64-bit bolme, bazi kaydirma
# ve donusumler). KUTUPHANE olarak bildiriliyor, link SECENEGI olarak degil:
# secenek olarak verilince komut satirinda nesne dosyalarindan ONCE yer aliyor
# ve GNU ld'nin tek gecisli arsiv cozumlemesi henuz talep edilmemis sembolleri
# cekmiyordu (bagimsiz denetim bulgusu I4: __aeabi_uldivmod cozumlenmiyordu).
# target_link_libraries onu nesne dosyalarindan SONRAYA koyar.
target_link_libraries(platform-mps2-an505 INTERFACE gcc)

# Linker script degisirse yeniden link et.
set_property(TARGET platform-mps2-an505 APPEND PROPERTY
    INTERFACE_LINK_DEPENDS ${LINKER_SCRIPT})
