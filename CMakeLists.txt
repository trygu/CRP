cmake_minimum_required(VERSION 3.10)
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
add_library(CRP SHARED ${CRP_SOURCES})
add_library(tauargus::crp ALIAS CRP)

# Headers (PUBLIC so downstream targets, e.g. native/rounder, can #include
# "WrapCRP.h" from CRP's src/ directory).
target_include_directories(CRP
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

# Platform-specific settings
if(WIN32)
    set_target_properties(CRP PROPERTIES SUFFIX ".dll")
    set_target_properties(CRP PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
elseif(APPLE)
    set_target_properties(CRP PROPERTIES SUFFIX ".dylib")
else()
    set_target_properties(CRP PROPERTIES SUFFIX ".so")
endif()

# Ensure the DLL is placed in the correct directory for all configurations on Windows
if(WIN32)
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
# symbol from the Windows DLL (rounder links against CRP.lib).
if(WIN32)
    set_target_properties(CRP PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Installation
install(TARGETS CRP
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
