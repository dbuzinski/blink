# Troubleshooting

## Port already in use

If `serve` fails to bind, another process may be using the port. Choose a different `Port` in `app.serve(Port=...)` or stop the other server.

## 404 in the browser

- Check the URL path matches a registered route (including leading slash and HTTP method).
- For static files, confirm `StaticFiles` is set and the URL starts with `StaticRoute` (default `/static`).

## 405 Method Not Allowed

The path matched something, but not for this verb. Register the method you need (`post`, `patch`, etc.) or check the **`Allow`** header on the response.

## Handler never runs

- Verify the HTTP method matches the registration (`get` vs `post`, etc.).
- Ensure the function handle has the form `@(req, resp) ...` and that you passed the correct handle to the right `app.<verb>` method.

## `resp` changes not visible

Handlers should assign to the **`blink.Response`** instance passed in (`resp.Data`, `resp.StatusCode`, headers). If you create a new `blink.Response` inside the handler without returning it, the framework may not use it—follow [Quick start](quick-start.md) and [Examples](examples.md).

## JSON errors

`jsondecode` throws if the body is empty or not valid JSON. Guard with `strlength(req.Data)` or `try`/`catch` for robust APIs.

## Query or headers look wrong

Confirm query keys are valid MATLAB identifiers after normalization (see [Request](request.md)). For headers, use the documented case-insensitive access pattern.
