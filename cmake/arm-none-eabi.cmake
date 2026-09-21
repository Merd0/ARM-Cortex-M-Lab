# ---------------------------------------------------------------------------
# Arm GNU Toolchain (arm-none-eabi) cross-compilation toolchain file.
#
# Mimari flag'leri TEK yerde tutulur. Hicbir lesson CMakeLists'i
# -mcpu / -mthumb yazmak zorunda degildir.
#
# Kullanim:
#   cmake --preset debug          (CMakePresets.json bu dosyayi secer)
# ---------------------------------------------------------------------------

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Bare-metal hedefte CMake'in "calistirilabilir uretip calistirma" testi
# basarisiz olur; static library testi yeterlidir.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Toolchain'i PATH'te ara; istenirse -DARM_TOOLCHAIN_BIN=... ile ezilebilir.
if(NOT DEFINED ARM_TOOLCHAIN_BIN)
    set(ARM_TOOLCHAIN_BIN "" CACHE PATH "arm-none-eabi-* araclarinin bulundugu dizin")
endif()

set(_tc_prefix arm-none-eabi-)

macro(_find_arm_tool var name)
    find_program(${var} ${_tc_prefix}${name} HINTS ${ARM_TOOLCHAIN_BIN})
endmacro()

_find_arm_tool(CMAKE_C_COMPILER   gcc)
_find_arm_tool(CMAKE_CXX_COMPILER g++)
_find_arm_tool(CMAKE_ASM_COMPILER gcc)
_find_arm_tool(CMAKE_OBJCOPY      objcopy)
_find_arm_tool(CMAKE_OBJDUMP      objdump)
_find_arm_tool(CMAKE_SIZE_UTIL    size)
_find_arm_tool(CMAKE_NM_UTIL      nm)
_find_arm_tool(CMAKE_READELF      readelf)

if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR
        "arm-none-eabi-gcc bulunamadi.\n"
        "Arm GNU Toolchain'i kurup PATH'e ekleyin, ya da\n"
        "  cmake --preset debug -DARM_TOOLCHAIN_BIN=C:/path/to/bin\n"
        "kullanin. Ayrintilar: docs/ENVIRONMENT.md")
endif()

# --------------------------------------------------------------- mimari ----
# Cortex-M33: Armv8-M Mainline + DSP + FPU.
# Baslangicta BILEREK soft-float kullaniyoruz:
#   - exception stack frame'i temel 8 word olarak kaliyor (ders 07/08 icin sade)
#   - FPU'yu acmak/lazy stacking'i gormek ayri bir derse birakiliyor (ders 13)
set(ARM_CPU_FLAGS "-mcpu=cortex-m33 -mthumb -mfloat-abi=soft")

set(CMAKE_C_FLAGS_INIT
    "${ARM_CPU_FLAGS} -ffreestanding -ffunction-sections -fdata-sections -fno-common")
set(CMAKE_CXX_FLAGS_INIT
    "${ARM_CPU_FLAGS} -ffreestanding -ffunction-sections -fdata-sections -fno-common -fno-exceptions -fno-rtti")
set(CMAKE_ASM_FLAGS_INIT "${ARM_CPU_FLAGS} -x assembler-with-cpp")

# -nostdlib: newlib yok. memcpy/memset gibi seyleri common/src/tiny_libc.c saglar.
#
# libgcc BURADA DEGIL: link satirinda kutuphaneler nesne dosyalarindan SONRA
# gelmeli, yoksa arsiv cozumlemesi derleyicinin urettigi yardimci rutinleri
# (ornegin __aeabi_uldivmod) bulamaz. Bu yuzden libgcc bir link KUTUPHANESI
# olarak platform/mps2-an505/CMakeLists.txt icinde bildirilir
# (bagimsiz denetim bulgusu I4).
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "${ARM_CPU_FLAGS} -nostdlib -Wl,--gc-sections -Wl,--print-memory-usage")

# --- optimizasyon seviyeleri (deterministik) -------------------------------
# DIKKAT: CMake'in Compiler/GNU.cmake modulu, *_INIT degiskenlerinin SONUNA
# kendi varsayilanlarini EKLER. Yani
#     set(CMAKE_C_FLAGS_RELEASE_INIT "-O2 -g")
# yazmak efektif olarak "-O2 -g -O3 -DNDEBUG" uretiyordu ve son -O3 kazaniyordu:
# "Release (-O2)" etiketi yalan oluyordu (bagimsiz denetim bulgusu I3).
#
# Cozum: son sozu soyleyen CACHE degiskenlerini burada sabitlemek. Boylece
# preset etiketi ile efektif optimizasyon seviyesi birebir ortusur.
set(CMAKE_C_FLAGS_DEBUG          "-Og -g3"           CACHE STRING "Debug C flags"          FORCE)
set(CMAKE_C_FLAGS_RELEASE        "-O2 -g -DNDEBUG"   CACHE STRING "Release C flags"        FORCE)
set(CMAKE_C_FLAGS_MINSIZEREL     "-Os -g -DNDEBUG"   CACHE STRING "MinSizeRel C flags"     FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g3 -DNDEBUG"  CACHE STRING "RelWithDebInfo C flags" FORCE)

set(CMAKE_ASM_FLAGS_DEBUG          "-g3"    CACHE STRING "Debug ASM flags"          FORCE)
set(CMAKE_ASM_FLAGS_RELEASE        "-g"     CACHE STRING "Release ASM flags"        FORCE)
set(CMAKE_ASM_FLAGS_MINSIZEREL     "-g"     CACHE STRING "MinSizeRel ASM flags"     FORCE)
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "-g3"    CACHE STRING "RelWithDebInfo ASM flags" FORCE)

set(CMAKE_EXECUTABLE_SUFFIX     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C   ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
