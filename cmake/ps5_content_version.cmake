# PS5 contentVersion (NN.NNN.NNN in sce_sys/param.json) from the release tag, so the console and the homebrew
# stores see each release as newer than the last:
#   v<major>.<minor>.<patch>[-port.<n>]  ->  <major>.<minor*100+patch>.<n>, zero-padded
#   v2.0.1-port.5                        ->  02.001.005
# Anything else (dev builds, untagged trees) gives 00.000.000.

function(_ps5_zero_pad value width out_var)
    string(LENGTH "${value}" length)
    while(length LESS width)
        set(value "0${value}")
        math(EXPR length "${length} + 1")
    endwhile()
    set(${out_var} "${value}" PARENT_SCOPE)
endfunction()

# ps5_content_version(<version> <out_var>)
function(ps5_content_version version out_var)
    if(NOT version MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)(-port\\.([0-9]+))?$")
        set(${out_var} "00.000.000" PARENT_SCOPE)
        return()
    endif()
    set(major "${CMAKE_MATCH_1}")
    math(EXPR minor "${CMAKE_MATCH_2} * 100 + ${CMAKE_MATCH_3}")
    set(port "${CMAKE_MATCH_5}")
    if(port STREQUAL "")
        set(port 0)
    endif()
    if(major GREATER 99 OR minor GREATER 999 OR port GREATER 999)
        message(FATAL_ERROR "PS5 contentVersion: '${version}' does not fit NN.NNN.NNN")
    endif()
    _ps5_zero_pad("${major}" 2 major)
    _ps5_zero_pad("${minor}" 3 minor)
    _ps5_zero_pad("${port}" 3 port)
    set(${out_var} "${major}.${minor}.${port}" PARENT_SCOPE)
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
