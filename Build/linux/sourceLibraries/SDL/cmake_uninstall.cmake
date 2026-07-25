if (NOT EXISTS "/repo/build/linux/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: \"/repo/build/linux/install_manifest.txt\"")
endif(NOT EXISTS "/repo/build/linux/install_manifest.txt")

file(READ "/repo/build/linux/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach (file ${files})
    message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
    execute_process(
        COMMAND /cache/cmake/bin/cmake -E remove "$ENV{DESTDIR}${file}"
        OUTPUT_VARIABLE rm_out
        RESULT_VARIABLE rm_retval
    )
    if(NOT ${rm_retval} EQUAL 0)
        message(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
    endif (NOT ${rm_retval} EQUAL 0)
endforeach(file)

