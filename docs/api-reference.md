# API reference

Summary of the public MATLAB types in `+blink`. For tutorials, see the [User guide](application.md); for precise behavior, see the [Specification](specification/index.md).

## `blink.App` (handle)

| Member | Description |
|--------|-------------|
| `Routes` | Array of `blink.Route` (managed via verb methods). |
| `StaticFiles` | Scalar `string` or empty — directory for static assets. |
| `StaticRoute` | Scalar `string` — URL prefix for static files (default `"/static"`). |
| `get(path, handler)` | Register a `GET` route. |
| `post(path, handler)` | Register a `POST` route. |
| `put(path, handler)` | Register a `PUT` route. |
| `patch(path, handler)` | Register a `PATCH` route. |
| `delete(path, handler)` | Register a `DELETE` route. |
| `options(path, handler)` | Register an `OPTIONS` route. |
| `use(...)` | Register middleware (signature per release documentation). |
| `serve(...)` | Start the server: `Port`, host, config, and other name-value options (blocks until shutdown). |

## `blink.Route`

Normally constructed internally.

| Property | Description |
|----------|-------------|
| `HttpMethod` | `'GET'`, `'POST'`, `'PUT'`, `'PATCH'`, `'DELETE'`, or `'OPTIONS'`. |
| `Path` | Route pattern including optional `:` segments. |
| `Handler` | Function handle `@(req, resp) ...`. |

## `blink.Request`

| Property | Type | Description |
|----------|------|-------------|
| `Parameters` | `struct` | Route parameters. |
| `Query` | `struct` | Parsed query string. |
| `Headers` | `struct` | Request headers (with documented case-insensitive access). |
| `Data` | `string` | Raw body. |
| *(parsed bodies)* | — | Additional fields for form and multipart when content type matches; see [Body & uploads](specification/body-uploads.md). |

Constructor (framework): `blink.Request(params, query, headers, data, ...)`.

## `blink.Response`

| Property | Type | Description |
|----------|------|-------------|
| `Data` | `char` (or documented binary type) | Response body. |
| `StatusCode` | `int16` | HTTP status (default 200). |
| `ContentType` | `string` or compatible | Sets `Content-Type` when set. |
| `Headers` | `struct` or documented type | Extra response headers. |

| Method | Description |
|--------|-------------|
| `render(renderer, template, context, partials)` | Render Mustache via matstache; sets `Data`. Optional `context` and `partials` default to empty structs. |
| *(helpers)* | Redirect, JSON, etc., per release documentation. |

## `blink.Config`

| Property / use | Description |
|----------------|-------------|
| Host, port, backlog, reuse | Listen defaults. |
| Static paths, environment | App-wide defaults. |
| Body and upload limits | Parser configuration. |

Passed to `serve` or set on the app per the [Config & lifecycle](specification/config-lifecycle.md) page.
