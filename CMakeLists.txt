cmake_minimum_required(VERSION 3.10)
project(CRP C CXX)

set(CRP_VERSION 3.1.0)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files
set(CRP_SOURCES
    src/crpCaudit.c
    src/crpCmain.c
    src/crpSaudit.c
    src/crpSmain.c
    src/crpXaudit.c
    src/crpXmain.c
    src/exact.c
    src/WrapCRP.c
)

# Headers
include_directories(src)

# Shared library
add_library(CRP SHARED ${CRP_SOURCES})

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
        set_target_properties(CRP PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/bin/${CONFIG_TYPE})
    endforeach()
endif()

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

# Solver libraries (user must set these appropriately)
# To enable open source SCIP solver support, uncomment and adjust the following lines:
# find_package(SCIP REQUIRED)
# target_link_libraries(CRP PRIVATE SCIP::scip)
# target_compile_definitions(CRP PRIVATE SCIPV)
#
# For XPRESS or CPLEX, do similarly if you have the libraries and CMake support.
# target_compile_definitions(CRP PRIVATE XPRESSV)
# target_compile_definitions(CRP PRIVATE CPLEXV)

# --- pkg-config support ---
configure_file(${CMAKE_SOURCE_DIR}/crp.pc.in ${CMAKE_BINARY_DIR}/crp.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/crp.pc DESTINATION lib/pkgconfig)

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
