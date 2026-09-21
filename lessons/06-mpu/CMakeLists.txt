# Lesson 06 firmware hedefleri.
#
#   06-mpu            example/   - MPU politikasi + SVC uzerinden yasal erisim
#   06-mpu-broken     ayni       - korunan veriye DOGRUDAN yazma (MemManage)
#   06-exercise-mpu   exercises/ - MPU politikasini sen yazacaksin
#   06-solution-mpu   solutions/ - referans cozum

add_firmware(06-mpu
    SOURCES example/main.c
            example/svc_handler.S
            example/psp_start.S
            example/mpu_probe.S)

add_firmware(06-mpu-broken
    SOURCES example/main.c
            example/svc_handler.S
            example/psp_start.S
            example/mpu_probe.S
    DEFINES LESSON06_BREAK_DIRECT_ACCESS=1)

add_firmware(06-mpu-permissive
    SOURCES example/main.c
            example/svc_handler.S
            example/psp_start.S
            example/mpu_probe.S
    DEFINES LESSON06_BREAK_DIRECT_ACCESS=1 LESSON06_EXPERIMENT_ALLOW_COUNTER=1)

add_firmware(06-exercise-mpu
    SOURCES exercises/01-thread-mpu-policy/main.c
            exercises/01-thread-mpu-policy/svc_handler.S
            exercises/01-thread-mpu-policy/psp_start.S
            exercises/01-thread-mpu-policy/mpu_probe.S)

add_firmware(06-solution-mpu
    SOURCES solutions/01-thread-mpu-policy/main.c
            solutions/01-thread-mpu-policy/svc_handler.S
            solutions/01-thread-mpu-policy/psp_start.S
            solutions/01-thread-mpu-policy/mpu_probe.S)
