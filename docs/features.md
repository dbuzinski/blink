# Features

Blink is a full HTTP stack for MATLAB applications. The [User guide](application.md) covers day-to-day use; the [Specification](specification/index.md) spells out detailed behavior.

## HTTP and routing

- **Verbs:** `GET`, `POST`, `PUT`, `PATCH`, `DELETE`, and `OPTIONS` (including CORS preflight and explicit `OPTIONS` routes).
- **Path parameters:** Named segments such as `/item/:id` surface on `req.Parameters`.
- **Semantics:** **404** when no route matches the path; **405** when the path matches but the method does not, with an **`Allow`** header where applicable.
- **Order:** Routes match in **registration order**; the first matching route wins.
- **Composition:** Route groups and URL prefixes (for example `blink.Router` or mounting sub-applications) keep large apps modular.
- **Optional:** Host-based routing when multiple virtual hosts share a process.

See [HTTP & routing](specification/http-routing.md).

## Request and response

- **Query string** and **headers** on `blink.Request`, with predictable mapping to MATLAB types and case-insensitive header lookup.
- **Response** status, body, **Content-Type**, and arbitrary **headers**; helpers for redirects and JSON where provided.
- **Cookies:** Parsed request cookies and response `Set-Cookie` support.

See [Request & response](specification/request-response.md).

## Bodies and uploads

- Parsed **`application/x-www-form-urlencoded`** and **`multipart/form-data`** bodies, with documented limits and **413** when exceeded.
- File uploads with configurable storage, cleanup, and size limits.

See [Body & uploads](specification/body-uploads.md).

## Middleware, errors, and CORS

- **Middleware** pipeline for cross-cutting logic (logging, auth, CORS).
- **Global error handlers** mapping MATLAB exceptions to HTTP responses, with configurable development vs production detail.
- **CORS** configuration aligned with browser preflight.

See [Middleware & errors](specification/middleware-errors.md).

## Configuration and lifecycle

- **`blink.Config`** for defaults: host, port, static paths, environment, and parser limits.
- **`serve`** options for listen behavior; **graceful shutdown** stops accepting connections and drains work according to documented policy.
- Optional **access logging** hooks.

See [Config & lifecycle](specification/config-lifecycle.md).

## Security

- Session patterns, CSRF considerations for cookie-based apps, and CORS rules for browser vs non-browser clients.

See [Security](specification/security.md).

## Real-time and TLS

- **WebSocket** routes and message callbacks alongside HTTP.
- **HTTPS** configuration from MATLAB-facing certificate and key options.

See [Real-time & HTTPS](specification/realtime-https.md).

## Execution model

- Clear rules for **which context runs handlers**, **blocking** vs background work, and **request/response lifetime** after the handler returns.

See [Execution model](specification/execution-model.md).

## Optional toolboxes

- **matstache** (Mustache templates) integrates via `blink.Response.render` but lives in a separate toolbox; core routing and HTTP behavior do not require it.

See [Patterns](patterns.md).
