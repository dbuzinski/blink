# Installation

## Supported MATLAB Releases

Blink is tested on MATLAB **R2022b and later**; we recommend using the latest MATLAB release.

## Supported Platforms

Blink supports installation on:

- Linux (x86_64)
- Windows
- macOS (Apple Silicon)

[Building from source](#build-from-source) may work on additional platforms, but it isn’t officially supported.

## Dependencies

- **Matstache** is required to render templates.

## Optional dependencies

These are optional and unlock additional workflows:

- **Database Toolbox**: for database-backed apps (see [Using Database Toolbox](getting-started.md#using-database-toolbox)).
- **Parallel Computing Toolbox**: for throughput/concurrency patterns (see [Using Parallel Computing Toolbox](getting-started.md#using-parallel-computing-toolbox)).
- **MATLAB Compiler**: for packaging and deploying apps at scale (see [Deploying at Scale with MATLAB Compiler](getting-started.md#deploying-at-scale-with-matlab-compiler)).

## Install from MLTBX

Blink can be installed from the Add-On Explorer, or by downloading and installing `blink.mltbx` from File Exchange.

You can verify your installation in MATLAB by running:

```matlab
which blink.App
```

## Build from Source

Prerequisites:

- Git
- Bazel >=  9.0
- A C++20 compiler toolchain
- Python 3 (for building docs)

Commands:

1. Clone the repository.
2. In MATLAB, run `buildtool package` to create an installable `.mltbx`.
3. In MATLAB, run `buildtool` for a full build (including tests and documentation).
