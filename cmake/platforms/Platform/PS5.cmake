# Platform rules for CMAKE_SYSTEM_NAME PS5 (see cmake/toolchains/PS5.cmake): a FreeBSD-like
# static target where an "executable" is one relocatable object for native-app-stage.sh.
set(CMAKE_EXECUTABLE_SUFFIX ".o")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

foreach(language IN ITEMS C CXX ASM)
    set(CMAKE_${language}_LINK_EXECUTABLE
        "${PS5_RELOCATABLE_LINKER} -r <OBJECTS> --start-group <LINK_LIBRARIES> --end-group -o <TARGET>")
endforeach()
