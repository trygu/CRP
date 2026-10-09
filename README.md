# tauargus — crp

The **crp** module implements the **Controlled Rounding Problem (CRP)** —
the mixed-integer programming (MIP) backend for iterative rounding with
auditing. It rounds the response-table cells to integer values while keeping
totals and margin constraints satisfied, and (optionally) audits the result.

The MIP is solved by **HiGHS** through the HiGHS C API. The two source files
that include the HiGHS header (`crpSmain.c`, `crpSaudit.c`) are compiled as
C++ so the `const` constants in the header get internal linkage and don't
produce duplicate-symbol link errors.

The historical CPLEX and XPRESS solver variants (`crpC*`, `crpX*`), the
`XXRounder` CPLEX-only helper, and the pkg-config / SCIP-FetchContent
machinery have all been removed; **HiGHS is the only solver**.

## What it provides

The public API (`src/WrapCRP.h`) exposes:

- `do_round(...)` — the main entry point the `rounder` module calls.
- `CRPsetfunctions`, `CRPSetDoubleConstant`, `CRPGetDoubleConstant` —
  callback / constant plumbing.

It is consumed by the `rounder` module (`../rounder`), a thin C++ dispatch
layer that drives `CRP` through pybind11.

## Build

Part of the superbuild in the top-level repo:

```bash
cmake -S native -B native/build \
      -DHighs_DIR=$(brew --prefix)/lib/cmake/Highs
cmake --build native/build
```

Standalone (needs HiGHS):

```bash
cmake -S native/crp -B native/crp/build \
      -DHighs_DIR=$(brew --prefix)/lib/cmake/Highs
cmake --build native/crp/build
```

Produces `libCRP.dylib` / `libCRP.so` / `CRP.dll` (shared). The `src/`
directory is exposed as a PUBLIC include dir so `rounder` can `#include
"WrapCRP.h"`.

## License

© jjsalaza@ull.es. Distributed under the EUPL v1.1 / EUPL-1.2. See the
top-level repository for the full notice.
