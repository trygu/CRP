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
find_package(Highs REQUIRED)
target_compile_definitions(CRP PRIVATE SCIPV)
target_link_libraries(CRP PRIVATE highs::highs)

# Option to build XXRounder (requires CPLEX)
option(BUILD_XXROUNDER "Build the XXRounder static library (requires CPLEX)" OFF)

# XXRounder (C++ static library)
if(BUILD_XXROUNDER)
    file(GLOB XXROUNDER_SOURCES XXRounder/*.cpp)
    add_library(XXRounder STATIC ${XXROUNDER_SOURCES})
    target_include_directories(XXRounder PUBLIC XXRounder)
    set(INSTALL_XXROUNDER XXRounder)
else()
    set(INSTALL_XXROUNDER)
endif()

# Link XXRounder if needed (example)
# target_link_libraries(CRP XXRounder)

# Set C standard
set_target_properties(CRP PROPERTIES C_STANDARD 99)
if(BUILD_XXROUNDER)
    set_target_properties(XXRounder PROPERTIES CXX_STANDARD 11)
endif()

# Installation
install(TARGETS CRP ${INSTALL_XXROUNDER}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
