# Lesson 05 firmware hedefleri.
#
#   05-privilege-svc          example/   - ayricalik dusurme + SVC zinciri
#   05-privilege-svc-broken   ayni       - tanimsiz SVC #99 (Break It)
#   05-exercise-svc2          exercises/ - SVC #2'yi sen ekleyeceksin
#   05-solution-svc2          solutions/ - referans cozum

add_firmware(05-privilege-svc
    SOURCES example/main.c
            example/svc_handler.S
            example/psp_start.S)

add_firmware(05-privilege-svc-broken
    SOURCES example/main.c
            example/svc_handler.S
            example/psp_start.S
    DEFINES LESSON05_BREAK_UNKNOWN_SVC=1)

add_firmware(05-exercise-svc2
    SOURCES exercises/01-add-svc-service/main.c
            exercises/01-add-svc-service/svc_handler.S
            exercises/01-add-svc-service/psp_start.S
    DEFINES LESSON05_EXERCISE_SVC2=1)

add_firmware(05-solution-svc2
    SOURCES solutions/01-add-svc-service/main.c
            solutions/01-add-svc-service/svc_handler.S
            solutions/01-add-svc-service/psp_start.S
    DEFINES LESSON05_SOLUTION_SVC2=1)
