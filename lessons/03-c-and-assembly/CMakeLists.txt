# Lesson 03 firmware hedefleri.
#
#   03-c-and-assembly          example/        - handwritten asm + C karsilastirmasi
#   03-c-and-assembly-broken   ayni kaynak     - callee-saved register ihlali
#   03-exercise-sub            exercises/01-   - senin dolduracagin sub_numbers
#   03-solution-sub            solutions/01-   - referans cozum
#   03-solution-mla            solutions/02-   - experiment'in referans cozumu

add_firmware(03-c-and-assembly
    SOURCES example/main.c
            example/add_numbers.S
            example/accumulate.S)

add_firmware(03-c-and-assembly-broken
    SOURCES example/main.c
            example/add_numbers.S
            example/accumulate.S
    DEFINES LESSON03_BREAK_CALLEE_SAVED=1)

add_firmware(03-exercise-sub
    SOURCES exercises/01-sub-numbers/main.c
            exercises/01-sub-numbers/sub_numbers.S)

add_firmware(03-solution-sub
    SOURCES solutions/01-sub-numbers/main.c
            solutions/01-sub-numbers/sub_numbers.S)

add_firmware(03-solution-mla
    SOURCES solutions/02-multiply-add/main.c
            solutions/02-multiply-add/multiply_add.S)
