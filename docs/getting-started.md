# Getting Started

This guide introduces Blink’s user-facing API by building small, focused examples. Treat this as a spec: names, shapes, and behavior described here are the intended final developer experience.

## A Hello World Application

Minimal app that responds on `/hello`.

```matlab
app = blink.App();

app.get("/hello", @hello);
app.serve(Port=8000);

function resp = hello(~, resp)
    resp.Data = "<h1>Hello, Blink!</h1>";
    resp.ContentType = "text/html";
end
```

## Routing

Blink routes map an HTTP method + path pattern to a MATLAB handler:

- `GET /health`
- `GET /users/:id`
- `POST /users`

Target routing features:

- Named path parameters (e.g. `:id`)
- Query string parsing
- Deterministic route matching (first match wins, or most-specific wins—final rule to be specified)

## Static Files

Target behavior: serve a directory of static assets (CSS, JS, images) under a URL prefix such as `/static`.

```matlab
app = blink.App();
app.StaticFiles = "public";
app.StaticRoute = "/static";
app.serve(Port=8000);
```

## Handling Requests

Handlers receive a request object (incoming data) and a response object (outgoing data). Blink should make it easy to access:

- Route parameters
- Query parameters
- Headers
- Body (raw + parsed when applicable)

## Sending Responses

Target response capabilities:

- Set status code (e.g. 200, 404, 500)
- Set headers
- Send text/HTML
- Send JSON
- Send files/streams (where supported)

## Rendering Templates

Target behavior: render templates (e.g. Mustache-style) with a context struct to produce HTML.

```matlab
function resp = homepage(~, resp)
    resp = resp.render("templates/home.mustache", struct("Title","Blink"));
end
```

## Redirects and Errors

Target behavior:

- Redirect helpers (301/302/307/308)
- Standard error responses (404 for missing route, 500 for unhandled exception)
- A consistent way to customize error pages/JSON error envelopes

## Middleware

Middleware is a chain that runs before/around route handlers, used for:

- Logging
- Authentication/authorization
- Request IDs / correlation IDs
- Parsing bodies
- Error handling

Target middleware design goals:

- Composable, predictable order
- Easy to write as plain MATLAB functions

## Sessions

Target behavior: optional session support for server-side or signed-cookie sessions.

This section will specify:

- How a session is created/read/written
- Cookie settings (secure, httpOnly, sameSite)
- Storage backends (in-memory, file, database, custom)

## Using Database Toolbox

Target behavior: clean patterns for DB-backed request handling:

- Create and manage connections/pools
- Per-request transaction boundaries
- Safe parameter binding
- Avoid leaking connections on errors

This section will eventually include a Blink-recommended approach and an end-to-end example.

## Using Parallel Computing Toolbox

Target behavior: scale throughput using parallelism where appropriate:

- Background work (queue + worker pool)
- Offloading CPU-heavy computation
- Coordinating request lifecycle vs asynchronous work

This section will eventually define what “safe” parallel usage looks like in Blink apps (state isolation, cancellation, timeouts).

## Deploying at Scale with MATLAB Complier

Target behavior: compile and run Blink apps in production:

- Building a deployable artifact
- Environment-based configuration
- Logging/metrics hooks
- Multi-process / multi-instance scaling

This section will eventually define the supported deployment topologies and recommended defaults.

