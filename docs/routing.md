# Routing

## Paths and parameters

Routes are literal paths with optional **named segments** prefixed with `:`.

Example:

```matlab
app.get("/toggle/:id", @toggle);
```

For a request to `/toggle/42`, the handler receives a [`blink.Request`](request.md) whose `Parameters` struct contains a field `id` with value `'42'` (character data).

Rules:

- Segment names become field names on `req.Parameters`.
- Values are strings (even when they look numeric).

## Handler contract

A route handler must be invocable as:

```matlab
function resp = myHandler(req, resp)
    % read req, set fields on resp, return resp
end
```

- **`req`** — [`blink.Request`](request.md)
- **`resp`** — [`blink.Response`](response.md)

Assign `resp.Data`, `resp.StatusCode`, response headers, and return `resp` (or mutate `resp` in place when using handle semantics).

## Registration order

Routes are stored in registration order. If two patterns could match the same request, **the first registered route wins**. Prefer specific routes before parameterized ones when order matters.

## HTTP methods

`blink.App` registers **GET**, **POST**, **PUT**, **PATCH**, **DELETE**, and **OPTIONS** routes via the corresponding methods. See [Application](application.md).

## Unmatched paths and wrong methods

- **404 Not Found** — No route matches the request path (including static mounts when configured).
- **405 Method Not Allowed** — The path matches a route, but nothing is registered for that HTTP method. The response includes an **`Allow`** header listing allowed methods where applicable.

Details: [HTTP & routing](specification/http-routing.md).
