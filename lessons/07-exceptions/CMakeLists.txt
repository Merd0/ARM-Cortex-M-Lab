# Lesson 07 firmware hedefleri.
#
#   07-exceptions             PSP kaynakli SVC: giris, yigma, donus
#   07-exceptions-msp         ayni olcum, Thread MSP uzerindeyken
#   07-exceptions-memmanage   MPU ihlali, MemManage ETKIN
#   07-exceptions-escalation  ayni ihlal, MemManage DEVRE DISI
#   07-exercise-decode        frame cozumleyiciyi sen yazacaksin
#   07-solution-decode        referans cozum

add_firmware(07-exceptions
    SOURCES example/main.c
            example/svc_handler.S
            example/psp_start.S)

add_firmware(07-exceptions-msp
    SOURCES example/main.c
            example/svc_handler.S
            example/psp_start.S
    DEFINES LESSON07_MSP_ORIGIN=1)

add_firmware(07-exceptions-memmanage
    SOURCES example/main.c
            example/svc_handler.S
            example/mem_handlers.S
            example/psp_start.S
            example/mpu_probe.S
    DEFINES LESSON07_BREAK_MEMMANAGE=1)

add_firmware(07-exceptions-escalation
    SOURCES example/main.c
            example/svc_handler.S
            example/mem_handlers.S
            example/psp_start.S
            example/mpu_probe.S
    DEFINES LESSON07_BREAK_ESCALATION=1)

add_firmware(07-exercise-decode
    SOURCES exercises/01-decode-exception-entry/main.c
            exercises/01-decode-exception-entry/svc_handler.S
            exercises/01-decode-exception-entry/psp_start.S)

add_firmware(07-solution-decode
    SOURCES solutions/01-decode-exception-entry/main.c
            solutions/01-decode-exception-entry/svc_handler.S
            solutions/01-decode-exception-entry/psp_start.S)
