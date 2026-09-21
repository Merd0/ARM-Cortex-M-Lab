# Sadece icinde CMakeLists.txt olan lesson'lar build'e girer.
# Boylece bos scaffold'lar build'i bozmaz.
file(GLOB lesson_dirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(d ${lesson_dirs})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${d}
       AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${d}/CMakeLists.txt)
        add_subdirectory(${d})
    endif()
endforeach()
