# Unit test of cmake/ps5_content_version.cmake. Run: cmake -P tests/ps5/test_content_version.cmake
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/ps5_content_version.cmake)

function(expect_version count expected)
    ps5_content_version("${count}" actual)
    if(NOT actual STREQUAL expected)
        message(SEND_ERROR "ps5_content_version(${count}) = ${actual}, expected ${expected}")
    endif()
endfunction()

expect_version("0" "00.000.000")
expect_version("42" "00.000.042")
expect_version("999" "00.000.999")
expect_version("1000" "00.001.000")
expect_version("12345678" "12.345.678")

set(template "${CMAKE_CURRENT_LIST_DIR}/../../assets/ps5/param.json")
set(output "$ENV{TMPDIR}/ps5_param_test.json")
file(REMOVE "${output}")
ps5_write_param_json("${template}" "${output}" "02.001.005")
file(READ "${output}" written)
if(NOT written MATCHES "\"contentVersion\": \"02.001.005\"")
    message(SEND_ERROR "ps5_write_param_json did not set contentVersion")
endif()
file(REMOVE "${output}")

message(STATUS "ps5 content version tests done")
