# Installation guide

This page describes the supported MATLAB version(s), required dependencies, and the two supported installation paths: installing a packaged toolbox (`.mltbx`) or building from source.

## MATLAB Version

- Blink targets **modern MATLAB** releases.
- This documentation is the **spec** for the final user-facing API; exact minimum version requirements may change during development.

## Dependencies

Required dependencies for users (non-developer installs):

- **MATLAB** (supported versions described above)

If Blink ships as an `.mltbx`, the goal is that end users should not need a C++ toolchain or build system to use Blink.

## Optional dependencies

These are optional and unlock additional workflows:

- **MATLAB Compiler**: for packaging/deploying apps at scale.
- **Database Toolbox**: for database-backed apps (see Getting Started).
- **Parallel Computing Toolbox**: for throughput/concurrency patterns (see Getting Started).

## Install From MLTBX

Target experience:

1. Download `Blink.mltbx` (from a release artifact, internal registry, or distribution channel).
2. Install it by double-clicking the `.mltbx` file or using `matlab.addons.install`.
3. Verify installation in MATLAB:

```matlab
which blink.App
```

Expected outcome: MATLAB can resolve `blink.App` and you can run the hello-world app from the Getting Started guide.

## Build From Source

Target experience for contributors and advanced users:

1. Clone the repository.
2. Build native components (if applicable).
3. Ensure MATLAB can find the `+blink` package and any compiled artifacts.

This documentation intentionally does not lock in a specific build system or exact build commands yet; the developer build should be automated and reproducible (CI) once the API surface is stable.

