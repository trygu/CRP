# Argus Open Source

**Version:** 3.1.0 (set centrally in CMakeLists.txt as CRP_VERSION)

Software to apply Statistical Disclosure Control techniques

Copyright jjsalaza@ull.es

This program is free software; you can redistribute it and/or 
modify it under the terms of the European Union Public Licence 
(EUPL) version 1.1, as published by the European Commission.

You can find the text of the EUPL v1.1 on
https://joinup.ec.europa.eu/software/page/eupl/licence-eupl

This software is distributed on an "AS IS" basis without 
warranties or conditions of any kind, either express or implied.

---

# Cross-Platform Build and Usage (2025 Update)

## Major Features
- **Modern CMake build system**: Supports Windows, macOS, and Linux.
- **Optional solver support**: CPLEX, SCIP (recommended, open source), and XPRESS.
- **CI/CD ready**: Suitable for automated builds and releases.

## Building the Core Library

1. **Install CMake** ([Download](https://cmake.org/download/))
2. Open a terminal in the project root.
3. Run:
   ```sh
   cmake -S . -B build
   cmake --build build
   cmake --install build
   ```
   This builds and installs the core CRP shared library (e.g., `libCRP.so`, `libCRP.dylib`, or `libCRP.dll`).

> **Note:** CMake is required on all platforms. On Windows, use the official installer or a package manager like Chocolatey.

## Solver Support

By default, the core library is built without solver support for fast CI and development builds. Solver support can be enabled as needed:

- **CPLEX**: Commercial solver, optional. Enable with `-DBUILD_XXROUNDER=ON` if you have CPLEX installed.
- **SCIP**: Open source solver, recommended for most users and used for official releases.
- **XPRESS**: Commercial solver, optional.

### How Automatic SCIP Inclusion Works

- **Automatic Download and Build:**
  When SCIP support is enabled (`-DUSE_SCIP=ON`), CMake will automatically download the SCIP source code from the official GitHub repository using FetchContent, build it from source, and link it to the CRP library. No manual installation of SCIP is needed.
- **Where is SCIP built?**
  The SCIP source is cloned into `build/_deps/scip-src/` and built in `build/_deps/scip-build/` inside your build directory. This is all handled automatically by CMake.
- **No Prebuilt Binaries:**
  The build always uses the latest stable SCIP source and does not use prebuilt binaries or system packages.

### Recommended Workflow

- **Normal development/CI builds:**
  Use `-DUSE_SCIP=OFF` (default in CI) for fast builds and to avoid long dependency downloads.
- **Release builds:**
  Use `-DUSE_SCIP=ON` to include full open source solver support. This is enabled automatically in the release workflow.

#### Example: Enabling SCIP for a Release

```sh
cmake -S . -B build -DUSE_SCIP=ON
cmake --build build
cmake --install build
```

> **Note:** The first build with SCIP enabled will take longer, as it downloads and compiles SCIP from source.

## Using with pkg-config

After installation, you can use pkg-config to get compiler and linker flags:

```sh
pkg-config --cflags crp   # Shows the include path
pkg-config --libs crp     # Shows the linker flags
```
If you install to a non-standard prefix, set the PKG_CONFIG_PATH environment variable:
```sh
export PKG_CONFIG_PATH=/your/install/prefix/lib/pkgconfig:$PKG_CONFIG_PATH
```

## Legacy Makefiles
Old Makefiles (including `Makefile` and NetBeans `NBMakefile`) are deprecated. Use CMake for all builds.

---

# Questions?
If you have issues or want to contribute, open an issue or pull request.
