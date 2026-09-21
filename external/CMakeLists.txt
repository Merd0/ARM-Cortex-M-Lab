# ---------------------------------------------------------------------------
# Ucuncu taraf kaynaklar. Bu dizindeki hicbir sey elle degistirilmez.
#
# CMSIS-Core (Arm, Apache-2.0) — repository'nin PRODUCTION core API'si.
# Kaynak: https://github.com/ARM-software/CMSIS_6  (CMSIS/Core/Include)
# Ayrintilar ve surum bilgisi: external/README.md
# ---------------------------------------------------------------------------

add_library(cmsis-core INTERFACE)

target_include_directories(cmsis-core SYSTEM INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Core/Include)
