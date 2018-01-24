cmake_minimum_required(VERSION 3.9)

project(AsioDemo)

set(CMAKE_CXX_STANDARD 11)

link_libraries(boost_system)

option(PRODUCE "is PRODUCE build" OFF)
if (PRODUCE)

else ()

endif ()


include_directories(src)
include_directories(${PROJECT_BINARY_DIR}/src)


function(has_main_method filePath res)
    set (${res} OFF PARENT_SCOPE)
    file(STRINGS ${item} lines)
    foreach(line ${lines})
        string(REGEX MATCHALL ".*int +main *(.*).*" mainMaskFound "${line}")
        if ("${mainMaskFound} " STREQUAL " ")

        else()
            set (${res} ON PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

function(add_executable_with_main)
    file(GLOB_RECURSE srcs *.[c|h]pp)
    set(depends "")
    set(mains "")
    string(LENGTH "${PROJECT_BINARY_DIR}/" buildPathLen)
    foreach(item ${srcs})
        string(SUBSTRING "${item}" 0 ${buildPathLen} prefix)
        if("${prefix} " STREQUAL "${PROJECT_BINARY_DIR}/ ")

        else()
            has_main_method(${item} res)
            if (${res})
                list(APPEND mains "${item}")
            else()
                list(APPEND depends "${item}")
            endif()
        endif()
    endforeach()

    foreach(item ${mains})
        string(REGEX REPLACE ".*/([^.]+)\\.[ch]pp" "\\1" fileName "${item}")
        message("add_executable(${fileName} ${item} ${depends})")
        add_executable(${fileName} ${item} ${depends})
    endforeach()
endfunction()

add_executable_with_main()
