# Blink

Blink is a lightweight web framework for MATLAB. You register routes on a [`blink.App`](application.md), handle requests with ordinary MATLAB functions, and read or set data on [`blink.Request`](request.md) and [`blink.Response`](response.md).

This documentation describes the **user-facing** API—`blink.*` types and how to build apps.

## Requirements

- MATLAB R2024b or later, with **MATLAB Compiler** (for the packaged MEX stack—see [Installation](installation.md)).
- Bazel (only if you build Blink from source).

## Where to go next

- [Quick start](quick-start.md) — minimal “hello world” server
- [Features](features.md) — what Blink provides
- [Application](application.md) — `blink.App`, `serve`, static files
- [Specification](specification/index.md) — HTTP semantics and detailed behavior
