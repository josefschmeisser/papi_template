include(ExternalProject)
find_package(Git REQUIRED)

set(PAPI_PREFIX "papi")

ExternalProject_Add(
    papi_src
    PREFIX ${PAPI_PREFIX}
    GIT_REPOSITORY "https://bitbucket.org/icl/papi.git"
    GIT_TAG "papi-5-7-0-t"
    TIMEOUT 10
    SOURCE_SUBDIR "src"
    BUILD_IN_SOURCE True
    CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND     ${CMAKE_MAKE_PROGRAM}
    INSTALL_COMMAND   ${CMAKE_MAKE_PROGRAM} install
    UPDATE_COMMAND ""
)

# Prepare PAPI
ExternalProject_Get_Property(papi_src install_dir)
set(PAPI_INCLUDE_DIR ${install_dir}/include)
set(PAPI_LIBRARY_PATH ${install_dir}/lib/libpapi.a)
file(MAKE_DIRECTORY ${PAPI_INCLUDE_DIR})
add_library(papi STATIC IMPORTED)
set_property(TARGET papi PROPERTY IMPORTED_LOCATION ${PAPI_LIBRARY_PATH})
set_property(TARGET papi APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${PAPI_INCLUDE_DIR})

# Dependencies
add_dependencies(papi papi_src)
