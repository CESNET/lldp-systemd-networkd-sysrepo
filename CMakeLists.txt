project(lldp-systemd-networkd-sysrepo LANGUAGES CXX)
cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# Set a default build type if none was specified. This was shamelessly stolen
# from VTK's cmake setup because these guys produce both CMake and a project that
# manipulates this variable, and the web is full of posts where people say that
# it is apparently evil to just set the build type in a way an earlier version of
# this patch did. Oh, and the location of this check/update matters, apparently.
#
# Yes, this is just plain crazy.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# -Werror is not a default for sanity reasons (one cannot know what warnings a future compiler
# might bring along), but it's a default in debug mode. The idea is that developers should care
# about a warning-free build, and that this is easier than messing with yet another configure option.
set(CMAKE_CXX_FLAGS_DEBUG "-Werror ${CMAKE_CXX_FLAGS_DEBUG}")

# I don't want to duplicate the compiler's optimizations
set(CMAKE_CXX_FLAGS "-O2 ${CMAKE_CXX_FLAGS}")

# Build warnings are useful tools (and this project should be warning-free anyway), enable them on all
# configurations. They are warnings, not errors.
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Woverloaded-virtual ${CMAKE_CXX_FLAGS}")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-Wsuggest-override ${CMAKE_CXX_FLAGS}")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_custom_target(lldp-systemd-networkd-sysrepo-version-cmake-ide
        cmake/ProjectGitVersion.cmake
        cmake/ProjectGitVersionRunner.cmake
        )
include(cmake/ProjectGitVersion.cmake)
prepare_git_version(LLDP_SYSTEMD_NETWORKD_SYSREPO_VERSION "0.1")

# Find dependencies
find_package(spdlog REQUIRED)
find_package(sdbus-c++ REQUIRED)
find_package(docopt REQUIRED)
find_package(PkgConfig)
pkg_check_modules(SYSREPO REQUIRED sysrepo-cpp>=1.4.135 IMPORTED_TARGET sysrepo)
pkg_check_modules(LIBYANG REQUIRED libyang-cpp>=1.0.233 IMPORTED_TARGET libyang)
pkg_check_modules(SYSTEMD REQUIRED IMPORTED_TARGET libsystemd)

find_path(SD_LLDP_H systemd/sd-lldp.h PATHS ${SYSTEMD_INCLUDEDIR} NO_DEFAULT_PATH)
if(NOT SD_LLDP_H)
    message(FATAL_ERROR "Cannot find required header <systemd/sd-lldp.h>, use https://github.com/systemd/systemd/pull/16744")
endif()
if(CMAKE_CXX_FLAGS MATCHES "-stdlib=libc\\+\\+" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    SET(STD_FILESYSTEM_LIBRARY c++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
    SET(STD_FILESYSTEM_LIBRARY stdc++fs)
endif()

# Build and install libraries and executables
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

add_library(sysrepo-lldp STATIC
        src/callback.cpp
        src/callback.h
        src/LLDP.cpp
        src/LLDP.h
        src/logging.cpp
        src/logging.h
        )
target_link_libraries(sysrepo-lldp
        PUBLIC spdlog::spdlog SDBusCpp::sdbus-c++ PkgConfig::SYSREPO PkgConfig::LIBYANG PkgConfig::SYSTEMD ${STD_FILESYSTEM_LIBRARY}
        )

add_executable(lldp-systemd-networkd-sysrepod
        src/main.cpp
        )
target_link_libraries(lldp-systemd-networkd-sysrepod
        PUBLIC sysrepo-lldp docopt
        )
add_dependencies(lldp-systemd-networkd-sysrepod target-LLDP_SYSTEMD_NETWORKD_SYSREPO_VERSION)
target_include_directories(lldp-systemd-networkd-sysrepod PRIVATE ${PROJECT_BINARY_DIR})

install(TARGETS
        lldp-systemd-networkd-sysrepod
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/)

install(FILES
        yang/czechlight-lldp@2020-11-04.yang
        yang/iana-afn-safi@2013-07-04.yang
        yang/ietf-inet-types@2013-07-15.yang
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/lldp-systemd-networkd-sysrepo/yang
        )

# Testing
include(CTest)
if(BUILD_TESTING)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/configure.cmake.h.in
                   ${CMAKE_CURRENT_BINARY_DIR}/configure.cmake.h)

    find_program(SYSREPOCTL sysrepoctl)
    find_program(SYSREPOCFG sysrepocfg)

    find_package(doctest 2.3.1 REQUIRED)

    add_library(DbusMock STATIC
            tests/dbus-helpers/dbus_server.cpp
            tests/dbus-helpers/dbus_server.h
            )
    target_link_libraries(DbusMock PUBLIC SDBusCpp::sdbus-c++)

    add_library(DoctestIntegration STATIC
            tests/doctest_integration.cpp
            tests/doctest_integration.h
            tests/pretty_printers.h
            tests/test_log_setup.h
            )
    target_include_directories(DoctestIntegration PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tests/ ${CMAKE_CURRENT_SOURCE_DIR}/src/)
    target_link_libraries(DoctestIntegration doctest::doctest spdlog::spdlog)
    target_compile_definitions(DoctestIntegration PUBLIC DOCTEST_CONFIG_SUPER_FAST_ASSERTS)

    set(sysrepo_previous_fixture_name UNDEFINED)
    function(sysrepo_fixture_env name)
        set(test_name_init sysrepo:prep:${name})
        set(test_name_cleanup sysrepo:clean:${name})
        set(fixture_name sysrepo:env:${name})

        add_test(NAME ${test_name_init} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/sysrepoctl-manage-module.sh ${SYSREPOCTL} ${SYSREPOCFG} prepare ${ARGN})
        set_tests_properties(${test_name_init} PROPERTIES FIXTURES_SETUP ${fixture_name} RESOURCE_LOCK sysrepo)
        add_test(NAME ${test_name_cleanup} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/sysrepoctl-manage-module.sh ${SYSREPOCTL} ${SYSREPOCFG} uninstall ${ARGN})
        set_tests_properties(${test_name_cleanup} PROPERTIES FIXTURES_CLEANUP ${fixture_name} RESOURCE_LOCK sysrepo)

        if(sysrepo_previous_fixture_name)
            set_tests_properties(${test_name_init} PROPERTIES DEPENDS sysrepo:clean:${sysrepo_previous_fixture_name})
        endif()

        set(sysrepo_previous_fixture_name ${name} PARENT_SCOPE)
    endfunction()

    function(lldp_test fname)
        add_executable(test_${fname} tests/${fname}.cpp)
        target_link_libraries(test_${fname}
                PUBLIC sysrepo-lldp DoctestIntegration DbusMock
                )
        target_include_directories(test_${fname}
                PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tests/ ${CMAKE_CURRENT_SOURCE_DIR}/src/ ${CMAKE_CURRENT_BINARY_DIR}
                PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
                )

        if(NOT CMAKE_CROSSCOMPILING)
            add_test(test_${fname} test_${fname})
        endif()
    endfunction()

    lldp_test(sysrepo-opsdata)
    sysrepo_fixture_env(sysrepo YANG ${CMAKE_CURRENT_SOURCE_DIR}/yang/czechlight-lldp@2020-11-04.yang)
    set_tests_properties(
            test_sysrepo-opsdata
            PROPERTIES FIXTURES_REQUIRED sysrepo:env:sysrepo
            RESOURCE_LOCK sysrepo
    )

    lldp_test(lldp-systemd)
endif()

find_package(Doxygen)
option(WITH_DOCS "Create and install internal documentation (needs Doxygen)" ${DOXYGEN_FOUND})
if(WITH_DOCS)
    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${doxyfile_in} ${doxyfile} @ONLY)
    add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
            SOURCES ${doxyfile_in}
            )
endif()
