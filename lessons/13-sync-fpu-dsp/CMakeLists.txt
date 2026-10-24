set(L13_SYNC example/sync_main.c example/sync_ops.S example/dsp.c sync_support.c)
add_firmware(13-sync-fpu-dsp SOURCES ${L13_SYNC})
add_firmware(13-sync-broken SOURCES ${L13_SYNC} DEFINES L13_NO_RETRY=1)
add_firmware(13-exercise-critical SOURCES exercise_driver.c sync_support.c
    exercises/01-critical-update/critical.c)
add_firmware(13-solution-critical SOURCES exercise_driver.c sync_support.c
    solutions/01-critical-update/critical.c)

# FP arithmetic is isolated from integer-only setup/reporting. Every source
# linked into a hard-float executable uses that ABI, including platform sources.
foreach(abi IN ITEMS softfp hard)
    add_library(l13-abi-${abi} OBJECT example/abi_add.c)
    target_compile_options(l13-abi-${abi} PRIVATE -mfpu=fpv5-sp-d16 -mfloat-abi=${abi})
endforeach()
foreach(mode IN ITEMS basic eager lazy denied hard)
    set(abi softfp)
    if(mode STREQUAL "hard")
        set(abi hard)
    endif()
    add_firmware(13-fpu-${mode}
        SOURCES example/fp_main.c example/fp_ops.S $<TARGET_OBJECTS:l13-abi-${abi}>
        OPTIONS -mfpu=fpv5-sp-d16 -mfloat-abi=${abi} -mgeneral-regs-only
        DEFINES L13_FP_${mode}=1)
    # Link driver must choose a libgcc variant with matching FP ABI as well.
    target_link_options(13-fpu-${mode} PRIVATE -mfpu=fpv5-sp-d16 -mfloat-abi=${abi})
endforeach()
