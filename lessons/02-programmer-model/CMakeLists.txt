# Lesson 02 firmware hedefleri.
#
#   02-programmer-model          example/  - eksiksiz calisan ornek (README bunu anlatir)
#   02-programmer-model-broken   ayni kaynak, bilerek bozulmus ("Break It")
#   02-exercise-apsr             exercises/01-decode-apsr-flags/ - senin dolduracagin surum
#   02-solution-apsr             solutions/01-decode-apsr-flags/ - referans cozum

add_firmware(02-programmer-model
    SOURCES example/main.c)

add_firmware(02-programmer-model-broken
    SOURCES example/main.c
            example/break_trigger.S
    DEFINES LESSON02_BREAK_SPSEL=1)

add_firmware(02-exercise-apsr
    SOURCES exercises/01-decode-apsr-flags/main.c)

add_firmware(02-solution-apsr
    SOURCES solutions/01-decode-apsr-flags/main.c)
