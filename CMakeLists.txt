cmake_minimum_required(VERSION 3.10)
# Honor LANGUAGE CXX for legacy .c files without compiler suffix warnings.
if(POLICY CMP0119)
    cmake_policy(SET CMP0119 NEW)
endif()
project(CRP C CXX)

set(CRP_VERSION 3.1.0)

# Source files
# The "S" files are now HiGHS-backed (the SCIP name is kept as the
# compile definition to minimise churn; they contain no SCIP code).
set(CRP_SOURCES
    src/crpSaudit.c
    src/crpSmain.c
    src/exact.c
    src/WrapCRP.c
)

# The two files that include the HiGHS C API must be compiled as C++.
# In C, `const` file-scope constants in a header have EXTERNAL linkage, so
# including highs_c_api.h in two translation units produces duplicate-symbol
# link errors. In C++ those `const` constants have internal linkage (one per
# TU), so the link succeeds. This mirrors how native/csp is built.
set_source_files_properties(src/crpSmain.c src/crpSaudit.c PROPERTIES LANGUAGE CXX)

# Shared library
option(BUILD_SHARED_LIBS "Build shared native libraries" ON)
add_library(CRP ${CRP_SOURCES})
add_library(tauargus::crp ALIAS CRP)
set(TAUARGUS_MAX_THREADS "" CACHE STRING "Limit HiGHS workers (empty uses its default)")
if(NOT TAUARGUS_MAX_THREADS STREQUAL "")
    if(NOT TAUARGUS_MAX_THREADS MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR "TAUARGUS_MAX_THREADS must be a positive integer")
    endif()
    target_compile_definitions(CRP PUBLIC TAUARGUS_MAX_THREADS=${TAUARGUS_MAX_THREADS})
endif()
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(CRP PUBLIC TAUARGUS_STATIC)
endif()

# Headers (PUBLIC so downstream targets, e.g. native/rounder, can #include
# "WrapCRP.h" from CRP's src/ directory).
target_include_directories(CRP
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

# CMake chooses the correct shared or static suffix for each platform.
if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(CRP PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    foreach(CONFIG_TYPE Debug Release RelWithDebInfo MinSizeRel)
        set_target_properties(CRP PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/bin/${CONFIG_TYPE})
    endforeach()
endif()

# --- HiGHS solver (open source; replaces the old SCIP backend) ---
# HiGHS version pin. The superbuild exports TAUARGUS_HIGHS_VERSION (read from
# engine/HiGHS-VERSION, the single source of truth). A standalone build from
# the parent checkout reads the same file.
if(NOT DEFINED TAUARGUS_HIGHS_VERSION AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../HiGHS-VERSION")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../../HiGHS-VERSION" _TA_HIGHS_TAG)
    string(STRIP "${_TA_HIGHS_TAG}" _TA_HIGHS_TAG)
    string(REGEX REPLACE "^v" "" TAUARGUS_HIGHS_VERSION "${_TA_HIGHS_TAG}")
endif()
find_package(Highs ${TAUARGUS_HIGHS_VERSION} REQUIRED)
target_compile_definitions(CRP PRIVATE SCIPV)
target_link_libraries(CRP PRIVATE highs::highs)

# Set C standard
set_target_properties(CRP PROPERTIES C_STANDARD 99)

# crpglob.h defines globals in a header shared by C and C++ sources; GCC >= 10
# defaults to -fno-common and would report multiple definitions at link time.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(CRP PRIVATE $<$<COMPILE_LANGUAGE:C>:-fcommon>)
endif()

# The DYNAMIC/EXPORTFUNC macros are not defined in this build, so export every
# symbol from the Windows DLL (rounder links against CRP's import library).
# The property covers MSVC only: CMake silently ignores it on MinGW, where
# the DLL would export nothing. Pass the equivalent GNU ld flag explicitly.
if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(CRP PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # Keep statically linked dependencies (notably winpthreads) private.
        # Exporting them makes a consumer's own C++ runtime conflict at link time.
        target_link_options(CRP PRIVATE "LINKER:--export-all-symbols"
                                        "LINKER:--exclude-libs,libpthread.a:libwinpthread.a")
    endif()
endif()

# Loader tests (run by the parent repo's dev loop,
# scripts/dev-test.sh). The test hook (CRPtest_load0old) is compiled
# into the library only when tests are enabled.
option(TAUARGUS_CRP_BUILD_TESTS "Build crp loader tests" OFF)
if(TAUARGUS_CRP_BUILD_TESTS)
    enable_testing()
    target_compile_definitions(CRP PRIVATE TAUARGUS_CRP_UNIT_TEST)
    add_executable(crp_maxsize_tests tests/maxsize_test.cpp)
    target_link_libraries(crp_maxsize_tests PRIVATE CRP)
    add_test(NAME maxsize COMMAND crp_maxsize_tests)
endif()

# Installation
install(TARGETS CRP
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(DIRECTORY src/
    DESTINATION include/crp
    FILES_MATCHING PATTERN "*.h"
)
