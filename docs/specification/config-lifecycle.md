# Config & app lifecycle

## `blink.Config`

`blink.Config` holds defaults and environment for an application:

- **Listen:** host, port, backlog, address reuse
- **Static files:** default folder and URL prefix
- **Environment** name or mode (development vs production)
- **Parsers:** maximum body size, upload directory, multipart limits

Apps pass a `blink.Config` into `serve`, merge it with name-value arguments, or set properties on `blink.App`—per the reference for `blink.App` and `blink.Config`.

## Serving and shutdown

- **`serve`** starts the HTTP server. Name-value arguments override `blink.Config` where both apply.
- **Graceful shutdown** (for example `stop` or closing a returned server handle) stops accepting new connections and completes or terminates in-flight requests according to the documented drain policy.

## Observability

Optional hooks provide **access logs** and extension points for metrics without exposing native implementation details.
