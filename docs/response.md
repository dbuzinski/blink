# Response (`blink.Response`)

Handlers receive a `blink.Response` and set fields (or helpers) to control what is sent to the client.

## `StatusCode`

`int16` — HTTP status code. Default **200**.

## `Data`

`char` row vector (or documented binary type) — response body: HTML, plain text, JSON text, bytes, etc.

## Content type and headers

Set **`ContentType`** and **`Headers`** so clients receive the correct `Content-Type` and any additional headers (`Location` for redirects, `Set-Cookie`, cache control, and so on). Defaults apply when you omit them.

## Helpers

The reference documents helpers for **redirects**, **JSON** bodies, and similar shortcuts where provided.

## `render` (templates)

If you use the optional **matstache** toolbox, fill the body from a Mustache template:

```matlab
function resp = page(~, resp, renderer, template, ctx)
    resp = resp.render(renderer, template, ctx);
end
```

Arguments:

- `renderer` — `matstache.Renderer`
- `template` — `string` template source
- `context` — `struct` (optional, default empty)
- `partials` — `struct` of named partial templates (optional, default empty)

`render` sets `resp.Data` from the rendered template string.

## Errors in handlers

Uncaught errors in a handler are passed to **global error handlers** when registered; otherwise Blink returns an appropriate **500** response per [Middleware & errors](specification/middleware-errors.md).
