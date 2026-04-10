# Common Patterns

This section is a collection of “how we build apps with Blink” patterns. It should stay practical and focused on user-facing API usage, not internal implementation.

## Project structure (proposed)

- One entrypoint that wires routes and middleware
- Separate folders for handlers, middleware, templates, static assets, and tests

## Request validation (proposed)

Topics to specify:

- How to validate JSON/form bodies
- How to return consistent validation errors
- How to share validation logic across handlers

## Error handling (proposed)

Topics to specify:

- Central error middleware
- Mapping exceptions to status codes
- HTML vs JSON error responses

## Dependency injection / shared resources (proposed)

Topics to specify:

- How to share DB connections, caches, and config safely
- Per-request context vs global state

## Background work (proposed)

Topics to specify:

- Fire-and-forget vs queued work
- Timeouts, cancellation, and retries
- Parallel Computing Toolbox integration patterns

