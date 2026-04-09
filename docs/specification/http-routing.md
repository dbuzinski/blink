# HTTP & routing

## Verbs

Blink registers **GET**, **POST**, **PUT**, **PATCH**, **DELETE**, and **OPTIONS**. `OPTIONS` supports CORS preflight and explicit `OPTIONS` routes.

## 404 and 405

After Blink registers your HTTP routes and any static mount, the native layer adds a **catch-all** route so unmatched requests get an explicit status instead of relying on library defaults. These responses are generated in C++ and **do not** run your MATLAB route handlers.

- **404 Not Found** — No dynamic route pattern matches the path (using `:param` segments), and the path is not under a configured static URL prefix.
- **405 Method Not Allowed** (default) — The path matches at least one registered route pattern or static prefix, but the request’s HTTP method is not among those registered (for example, `GET` when only `POST` exists for that path). The response includes an **`Allow`** header listing the methods that apply to that path. In the native `BlinkApp` API, `setTreatWrongMethodAs405(false)` makes these cases return **404** instead (optional, for deployments that prefer not to advertise allowed methods).

## Route registration order

Routes are matched in **registration order**. The **first** registered route that matches the request wins. Register more specific paths before general ones when patterns overlap.

## Path parameters

Segments of the form `:name` bind to `req.Parameters.name` (string values). See [Routing](../routing.md).

## Route groups and prefixes

Large applications use **route groups** or **mounted sub-applications** so shared URL prefixes and middleware stay in one place. The exact MATLAB API (`blink.Router`, `mount`, or equivalent) is part of the public surface documented alongside `blink.App`.

## Host-based routing

When multiple hostnames are served from one process, Blink may select routes or configuration using the request **`Host`** header. Single-host apps omit this.

## Static files

Static responses use **correct MIME types**, **cache-related headers** where appropriate, and **safe path resolution** so only files under the configured directory are served—no directory traversal beyond the mount.

## Error bodies

Default bodies for **404**, **405**, and **500** respect **Content-Type** and negotiation: API routes may return JSON error objects; browser-oriented routes may return HTML error pages. Exact defaults are consistent and documented per route type.
