cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

###############################################################################
#### DEFINITIONS ##############################################################
###############################################################################
find_package(GLib REQUIRED)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_search_module(UUID REQUIRED uuid)
pkg_search_module(blkid REQUIRED blkid)
pkg_search_module(mount REQUIRED mount)

# glib dependencies
set(GLIB_DEPS
    pcre
    gio-2.0
    glib-2.0
    gmodule-2.0
    gobject-2.0
    blkid
    ffi
    mount
    selinux
    uuid
    z)

OPTION(INSTALL_UNIT_TEST  "install unit test" OFF)

###############################################################################
#### INCLUDES #################################################################
###############################################################################

include_directories(SYSTEM)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

###############################################################################
#### LINKER ###################################################################
###############################################################################

if (INSTALL_UNIT_TEST)
    add_library(gtest STATIC IMPORTED)
    set_target_properties(gtest PROPERTIES IMPORTED_LOCATION "${GOOGLETEST_STAGING_DIR}/usr/lib/libgtest.a")

    add_library(gtest_main STATIC IMPORTED)
    set_target_properties(gtest_main PROPERTIES IMPORTED_LOCATION "${GOOGLETEST_STAGING_DIR}/usr/lib/libgtest_main.a")
endif()

###############################################################################
#### SOURCES ##################################################################
###############################################################################

file(GLOB UPDI_PROGRAMMER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

###############################################################################
#### GENERATE OUTPUT ##########################################################
###############################################################################

add_executable(updi_programmer ${UPDI_PROGRAMMER_SOURCE})

target_link_libraries(updi_programmer LINK_PUBLIC
    ${GLIB_DEPS}
)

# install(TARGETS
#         updi_programmer
#         RUNTIME DESTINATION usr/bin
#         LIBRARY DESTINATION usr/lib)

if (INSTALL_UNIT_TEST)
    add_executable(intel_hexfile_unit_test
        "${CMAKE_CURRENT_SOURCE_DIR}/unit_test/intel_hexfile_unit_test.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/intel_hexfile.cpp")

    add_dependencies(intel_hexfile_unit_test googletest)

    target_link_libraries(intel_hexfile_unit_test LINK_PUBLIC
        ${GTEST_DEPS}
        ${GLIB_DEPS}
    )

    target_include_directories(intel_hexfile_unit_test PRIVATE ${GOOGLETEST_SOURCE_DIR}/googletest/include)
    target_include_directories(intel_hexfile_unit_test PRIVATE ${GOOGLETEST_SOURCE_DIR}/googlemock/include)

    install(TARGETS
        intel_hexfile_unit_test
        RUNTIME DESTINATION usr/bin
        LIBRARY DESTINATION usr/lib)

    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/fake_attiny.hex
        ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/wrong_checksum.hex
        DESTINATION usr/bin)
endif()
