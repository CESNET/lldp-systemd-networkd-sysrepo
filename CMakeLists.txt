project(lldp-systemd-networkd-sysrepo LANGUAGES CXX)
cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)
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
prepare_git_version(LLDP-SYSTEMD-NETWORKD-SYSREPO_VERSION "0.0")

# Find dependencies
find_package(spdlog REQUIRED)
find_package(PkgConfig)
pkg_check_modules(SYSREPO REQUIRED libSysrepo-cpp>=0.7.3 IMPORTED_TARGET libsysrepo) # we're using both C++ and C APIs

# Build and install libraries and executables
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

add_library(log-utils STATIC
        src/utils/log.cpp
        src/utils/log.h
        )
target_link_libraries(log-utils PUBLIC spdlog::spdlog)

add_executable(lldp-systemd-networkd-sysrepod
        src/main.cpp
        )
target_link_libraries(lldp-systemd-networkd-sysrepod
        PRIVATE PkgConfig::SYSREPO
        )

install(TARGETS
        lldp-systemd-networkd-sysrepod
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/)

# Testing
include(CTest)
if(BUILD_TESTING)
    find_program(SYSREPOCTL sysrepoctl)
    find_program(SYSREPOCFG sysrepocfg)

    find_package(trompeloeil 33 REQUIRED)
    find_package(doctest 2.3.1 REQUIRED)

    add_library(DoctestIntegration STATIC
            tests/doctest_integration.cpp
            tests/pretty_printers.h
            tests/test_log_setup.h
            tests/trompeloeil_doctest.h
            )
    target_include_directories(DoctestIntegration PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tests/ ${CMAKE_CURRENT_SOURCE_DIR}/src/)
    target_link_libraries(DoctestIntegration doctest::doctest trompeloeil spdlog::spdlog)
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
                PUBLIC sysrepo-integration DoctestIntegration log-utils
                PRIVATE PkgConfig::SYSREPO
                )
        target_include_directories(test_${fname}
                PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tests/ ${CMAKE_CURRENT_SOURCE_DIR}/src/
                PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
                )

        if(NOT CMAKE_CROSSCOMPILING)
            add_test(test_${fname} test_${fname})
        endif()
    endfunction()
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
