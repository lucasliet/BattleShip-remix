# Writes a .torch-filelist in every directory under ROOT: the files below that directory, one per
# line. The PS5 title cannot list /app0, so Torch reads these instead (torch/src/utils/TorchUtils.cpp).
# Usage: cmake -DROOT=<dir> -P cmake/ps5_torch_filelists.cmake
if(NOT ROOT OR NOT IS_DIRECTORY "${ROOT}")
    message(FATAL_ERROR "ROOT must name an existing directory, got '${ROOT}'")
endif()

file(GLOB_RECURSE directories LIST_DIRECTORIES true "${ROOT}/*")
list(FILTER directories INCLUDE REGEX ".*")
set(listed_directories "${ROOT}")
foreach(path IN LISTS directories)
    if(IS_DIRECTORY "${path}")
        list(APPEND listed_directories "${path}")
    endif()
endforeach()

foreach(directory IN LISTS listed_directories)
    file(GLOB_RECURSE entries LIST_DIRECTORIES false RELATIVE "${directory}" "${directory}/*")
    list(FILTER entries EXCLUDE REGEX "(^|/)\\.torch-filelist$")
    list(SORT entries)
    list(JOIN entries "\n" manifest)
    file(WRITE "${directory}/.torch-filelist" "${manifest}\n")
endforeach()
