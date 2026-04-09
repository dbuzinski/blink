# Application (`blink.App`)

Create an app with the default constructor:

```matlab
app = blink.App();
```

## Registering routes

Use one method per HTTP verb. Each call appends a route to the app’s route list (order matters; see [Routing](routing.md)).

| Method | Purpose |
|--------|---------|
| `app.get(path, handler)` | `GET` |
| `app.post(path, handler)` | `POST` |
| `app.put(path, handler)` | `PUT` |
| `app.patch(path, handler)` | `PATCH` |
| `app.delete(path, handler)` | `DELETE` |
| `app.options(path, handler)` | `OPTIONS` |

`path` is a character vector (for example `"/api/items"`). `handler` is a function handle with the **[handler contract](routing.md#handler-contract)**.

## Serving

```matlab
app.serve(Port=5000)
```

`Port` defaults to **5000** if omitted. Listen **host**, **backlog**, and other options are given as name-value arguments or via [`blink.Config`](api-reference.md#blinkconfig)—see the API reference and [Config & lifecycle](specification/config-lifecycle.md).

`serve` starts the HTTP server and **blocks** until the server stops (for example after **graceful shutdown**).

## Static files

Optional properties:

| Property | Meaning |
|----------|---------|
| `StaticFiles` | Path to a directory of files to serve (scalar string or empty). |
| `StaticRoute` | URL prefix for those files (default `"/static"`). |

When `StaticFiles` is non-empty, Blink registers a static file handler under `StaticRoute`. See [Static files](static-files.md).

## Middleware

Register **middleware** with the public API on `blink.App` (see [Middleware & errors](specification/middleware-errors.md)).

## Public vs internal API

Application code uses **`blink.App`** and documented `blink.*` types only. Lower-level entry points are not part of the public API.
