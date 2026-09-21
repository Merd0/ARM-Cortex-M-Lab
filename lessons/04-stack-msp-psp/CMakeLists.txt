# Lesson 04 firmware hedefleri.
#
#   04-stack-msp-psp          example/    - uc asamali MSP/PSP gecisi + push/pop
#   04-stack-msp-psp-broken   ayni kaynak - PSPLIM + kontrollu stack tasmasi
#   04-exercise-psp           exercises/  - PSP'yi sen aktif edeceksin
#   04-solution-psp           solutions/  - referans cozum

add_firmware(04-stack-msp-psp
    SOURCES example/main.c
            example/push_pop_probe.S
            example/psp_start.S)

add_firmware(04-stack-msp-psp-broken
    SOURCES example/main.c
            example/push_pop_probe.S
            example/psp_start.S
    DEFINES LESSON04_BREAK_STACK_OVERFLOW=1)

add_firmware(04-exercise-psp
    SOURCES exercises/01-activate-psp/main.c
            exercises/01-activate-psp/psp_start.S)

add_firmware(04-solution-psp
    SOURCES solutions/01-activate-psp/main.c
            solutions/01-activate-psp/psp_start.S)
