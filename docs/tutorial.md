# Tutorial: Build an end-to-end Blink app

This tutorial walks through building a small, production-shaped Blink application: a tiny “notes” app with HTML pages, static assets, tests, and a deployable configuration.

## Project Setup

Target project layout (suggested):

```
my-blink-app/
  app/
    main.m
    handlers/
    middleware/
  templates/
  public/
  tests/
```

Goals:

- One clear entrypoint (`main.m`)
- A place for handlers and middleware
- Templates and static assets kept separate
- Tests that can run non-interactively

## Define Endpoint

We’ll define endpoints for:

- `GET /` (homepage)
- `GET /notes` (list notes)
- `POST /notes` (create note)
- `GET /notes/:id` (show note)

This section will specify:

- Handler signatures
- How to read route params and body fields
- How validation errors are represented

## Add Templates

We’ll render HTML pages from templates:

- A base layout
- A list page
- A detail page

This section will specify:

- How templates are loaded (path, caching)
- How context structs map to template variables
- How partials/layouts work

## Add Static Files

We’ll add CSS/JS assets under a static route:

- A stylesheet for layout
- A small script for progressive enhancement

This section will specify:

- Default cache headers and how to override
- Content type detection
- Directory traversal protection

## Testing

We’ll test the application at two levels:

- Unit tests for handlers (pure functions where possible)
- Integration tests that run the server and make HTTP calls

This section will specify:

- A stable way to start/stop Blink for tests
- How to choose ports (random available port recommended)
- How errors are asserted

## Deploy to Production

We’ll cover a production deployment shape:

- Configuration via environment variables (port, host, logging, secrets)
- Running multiple instances behind a load balancer
- A compiled deployment path (MATLAB Compiler) where supported

This section will specify:

- Supported deployment models
- Recommended defaults (timeouts, limits)
- How to rotate secrets and session keys

