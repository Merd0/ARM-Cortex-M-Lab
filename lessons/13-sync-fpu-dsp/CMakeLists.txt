set(L13_SYNC example/sync_main.c example/sync_ops.S example/dsp.c sync_support.c)
add_firmware(13-sync-fpu-dsp SOURCES ${L13_SYNC})
add_firmware(13-sync-broken SOURCES ${L13_SYNC} DEFINES L13_NO_RETRY=1)
add_firmware(13-exercise-critical SOURCES exercise_driver.c sync_support.c
    exercises/01-critical-update/critical.c)
add_firmware(13-solution-critical SOURCES exercise_driver.c sync_support.c
    solutions/01-critical-update/critical.c)
