# Unit test of cmake/ps5_content_version.cmake. Run: cmake -P tests/ps5/test_content_version.cmake
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/ps5_content_version.cmake)

set(failures 0)

function(expect_version tag expected)
    ps5_content_version("${tag}" actual)
    if(NOT actual STREQUAL expected)
        message(SEND_ERROR "ps5_content_version(${tag}) = ${actual}, expected ${expected}")
    endif()
endfunction()

expect_version("v2.0.1-port.5" "02.001.005")
expect_version("v2.0.1-port.12" "02.001.012")
expect_version("v2.1.0-port.1" "02.100.001")
expect_version("v3.0.0" "03.000.000")
expect_version("dev" "00.000.000")
expect_version("v2.0.1-rc.1" "00.000.000")

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
