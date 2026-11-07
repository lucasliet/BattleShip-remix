# PS5 contentVersion (NN.NNN.NNN in sce_sys/param.json) for the console and the homebrew stores, which treat a
# larger value as a newer release. It is the number of v* tags reachable from the built commit, written as an
# eight-digit number: the release tag v2.0.1-port.5, the 42nd tag, gives 00.000.042. It grows by one with each
# release whatever the version format; a build without git gives 00.000.000.

# ps5_count_release_tags(<out_var>)
function(ps5_count_release_tags out_var)
    find_package(Git QUIET)
    set(count 0)
    if(Git_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} tag --merged HEAD --list "v*"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE tags
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET)
        if(NOT tags STREQUAL "")
            string(REPLACE "\n" ";" tags "${tags}")
            list(LENGTH tags count)
        endif()
    endif()
    set(${out_var} "${count}" PARENT_SCOPE)
endfunction()

# ps5_content_version(<release_count> <out_var>)
function(ps5_content_version release_count out_var)
    if(NOT release_count MATCHES "^[0-9]+$" OR release_count GREATER 99999999)
        message(FATAL_ERROR "PS5 contentVersion: '${release_count}' is not a count of 0..99999999")
    endif()
    set(digits "0000000${release_count}")
    string(LENGTH "${digits}" length)
    math(EXPR start "${length} - 8")
    string(SUBSTRING "${digits}" ${start} 8 digits)
    string(SUBSTRING "${digits}" 0 2 high)
    string(SUBSTRING "${digits}" 2 3 middle)
    string(SUBSTRING "${digits}" 5 3 low)
    set(${out_var} "${high}.${middle}.${low}" PARENT_SCOPE)
endfunction()

# ps5_write_param_json(<template> <output> <content_version>)
# Copies param.json with its contentVersion replaced; the file is only rewritten when it changes.
function(ps5_write_param_json template output content_version)
    file(READ "${template}" param_json)
    string(REGEX REPLACE "\"contentVersion\": \"[0-9.]+\"" "\"contentVersion\": \"${content_version}\""
           param_json "${param_json}")
    set(previous "")
    if(EXISTS "${output}")
        file(READ "${output}" previous)
    endif()
    if(NOT previous STREQUAL param_json)
        file(WRITE "${output}" "${param_json}")
    endif()
endfunction()
