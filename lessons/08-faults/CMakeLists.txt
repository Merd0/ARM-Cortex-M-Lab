# Lesson 08 firmware hedefleri - her biri TEK bir ogretim noktasini izole eder.
#
#   08-faults-memmanage     ayricaliksiz STR -> korunan veri   (temiz fault)
#   08-faults-usage         UDF, UsageFault ETKIN
#   08-faults-bus           eslenmemis adrese LDR, BusFault ETKIN
#   08-faults-escalation    ayni UDF, UsageFault DEVRE DISI -> HardFault
#   08-exercise-fault-report  rapor cozumleyiciyi sen yazacaksin
#   08-solution-fault-report  referans cozum

set(L08_SRC example/main.c example/fault_handlers.S
            example/fault_triggers.S example/psp_start.S)

add_firmware(08-faults-memmanage  SOURCES ${L08_SRC} DEFINES LESSON08_CASE_MEMMANAGE=1)
add_firmware(08-faults-usage      SOURCES ${L08_SRC} DEFINES LESSON08_CASE_USAGE=1)
add_firmware(08-faults-bus        SOURCES ${L08_SRC} DEFINES LESSON08_CASE_BUS=1)
add_firmware(08-faults-escalation SOURCES ${L08_SRC} DEFINES LESSON08_CASE_ESCALATION=1)

add_firmware(08-exercise-fault-report
    SOURCES ${L08_SRC} exercises/01-fault-report/decode.c
    DEFINES LESSON08_CASE_MEMMANAGE=1 LESSON08_HAS_DECODE=1)
target_include_directories(08-exercise-fault-report PRIVATE example)

add_firmware(08-solution-fault-report
    SOURCES ${L08_SRC} solutions/01-fault-report/decode.c
    DEFINES LESSON08_CASE_MEMMANAGE=1 LESSON08_HAS_DECODE=1)
target_include_directories(08-solution-fault-report PRIVATE example)
