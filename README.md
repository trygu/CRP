# Argus Open Source

**Version:** 3.1.0 (set centrally in CMakeLists.txt as CRP_VERSION)

Software to apply Statistical Disclosure Control techniques

Copyright jjsalaza@ull.es

This program is free software; you can redistribute it and/or 
modify it under the termcomboBoxOutputDimensions of the European Union Public Licence 
(EUPL) version 1.1, as published by the European Commission.

You can find the text of the EUPL v1.1 on
https://joinup.ec.europa.eu/software/page/eupl/licence-eupl

This software is distributed on an "AS IS" basis without 
warranties or conditions of any kind, either express or implied.

---

# Cross-Platform Build and Usage (2025 Update)

## Major Updates
- **CMake-based build system**: Now supports Windows, macOS, and Linux out of the box.
- **Optional commercial solver (CPLEX)**: The XXRounder static library and CPLEX dependency are now optional. By default, you do NOT need CPLEX to build or use the core library.
- **Cloud-native ready**: The project is suitable for CI/CD, containers, and modern development environments.

## Building the Core Library

1. **Install CMake** (https://cmake.org/download/)
2. Open a terminal in the project root.
3. Run:
   ```sh
   cmake -S . -B build
   cmake --build build
   ```
   This will build the core CRP shared library (e.g., `libCRP.dylib` on macOS, `libCRP.so` on Linux, `libCRP.dll` on Windows).

> **Note:** CMake is required to build this project on all platforms (Windows, macOS, Linux). Download it from https://cmake.org/download/ if you do not already have it installed. On Windows, you can use the official installer or package managers like Chocolatey or Scoop.

## Optional: Building XXRounder (CPLEX-dependent)
- By default, the XXRounder static library (and CPLEX dependency) is **not** built.
- If you have CPLEX and want to build XXRounder:
   ```sh
   cmake -S . -B build -DBUILD_XXROUNDER=ON
   cmake --build build
   ```
- If you do not have CPLEX, leave `BUILD_XXROUNDER=OFF` (the default).

## Supported Solvers

This project supports multiple mathematical optimization solvers:

- **CPLEX**: A commercial, high-performance solver from IBM for linear, integer, and quadratic programming. CPLEX is optional and only required if you want to build the XXRounder static library or use CPLEX-specific features.
- **SCIP**: An open source solver for mixed integer programming and related problems. You can use SCIP as a free alternative to CPLEX.
- **XPRESS**: Another commercial solver, supported if you have the appropriate libraries and headers.

> **Note:** For open source usage, SCIP is recommended. Ensure the relevant solver libraries and headers are available on your system and update the CMake configuration as needed if you want to enable or link against these solvers.
>
> **How to enable SCIP (open source solver):**
> 1. Install SCIP and its development files for your platform.
> 2. In `CMakeLists.txt`, uncomment the lines for SCIP support (see the Solver libraries section).
> 3. Run CMake and build as usual. The build will link against SCIP and enable its features.

## Notes
- The build system will automatically detect your OS and compiler.
- You can install the built libraries with:
   ```sh
   cmake --install build
   ```
- For solver support (CPLEX, SCIP, XPRESS), ensure the relevant libraries and headers are available and update the CMake configuration as needed.

## Using with pkg-config

This library provides a pkg-config file (`crp.pc`) for easy integration with other projects.

After installation, you can use pkg-config to get the necessary compiler and linker flags:

```sh
pkg-config --cflags crp   # Shows the include path
pkg-config --libs crp     # Shows the linker flags
```

If you install to a non-standard prefix, set the PKG_CONFIG_PATH environment variable:

```sh
export PKG_CONFIG_PATH=/your/install/prefix/lib/pkgconfig:$PKG_CONFIG_PATH
```

This makes it easy to use CRP in your own C/C++ projects and build systems.

## Legacy Makefiles
- Old Makefiles (including `Makefile` and NetBeans `NBMakefile`) are deprecated and no longer maintained. They may not reflect the current build process and should not be used. Use CMake for all new builds.

---

# Questions?
If you have issues or want to contribute, open an issue or pull request.
