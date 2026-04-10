# Configuration

This section defines how Blink applications are configured across development, test, and production. Subsections are intentionally provisional; as the API solidifies, this page becomes the single source of truth.

## Configuration sources (proposed)

Proposed precedence (highest wins):

1. Per-call overrides (e.g. `app.serve(Port=...)`)
2. App-level configuration (e.g. `app.Config = ...`)
3. Environment variables
4. Built-in defaults

## Server settings (proposed)

Topics to specify:

- Host / port binding
- Timeouts (request, idle, keep-alive)
- Max header size / max body size
- Connection backlog and concurrency model

## Routing settings (proposed)

Topics to specify:

- Case sensitivity
- Trailing slash normalization
- URL decoding rules
- Route conflict behavior

## Static files settings (proposed)

Topics to specify:

- URL prefix
- Caching headers
- Content type mapping
- Allow/deny rules

## Templates settings (proposed)

Topics to specify:

- Template root(s)
- Caching strategy
- Partial lookup rules

## Sessions settings (proposed)

Topics to specify:

- Cookie settings (secure/httpOnly/sameSite)
- Session storage backend selection
- Key rotation strategy

## Logging & observability (proposed)

Topics to specify:

- Structured logging fields
- Request IDs
- Metrics hooks (latency, status codes)

