add_firmware(14-systick-pendsv SOURCES example/main.c example/priorities.c)
add_firmware(14-pendsv-broken SOURCES example/main.c example/priorities.c
    DEFINES L14_BAD_PRIORITY=1)
add_firmware(14-exercise-priority SOURCES example/main.c exercises/01-defer-work/priorities.c)
add_firmware(14-solution-priority SOURCES example/main.c solutions/01-defer-work/priorities.c)
