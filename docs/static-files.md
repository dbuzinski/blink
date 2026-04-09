# Static files

Mount a directory of assets (CSS, JavaScript, images) at a URL prefix.

## Properties on `blink.App`

| Property | Type | Description |
|----------|------|-------------|
| `StaticFiles` | scalar `string` or empty | Filesystem path to the folder to expose. Empty means **no** static mount. |
| `StaticRoute` | scalar `string` | URL prefix (default `"/static"`). |

Example:

```matlab
app = blink.App();
app.StaticFiles = "static";
app.StaticRoute = "/static";
app.get("/", @home);
app.serve(Port=8000);
```

A file at `static/css/app.css` is available at `http://localhost:<port>/static/css/app.css` (exact URL depends on port and `StaticRoute`).

## Behavior

Static responses use **appropriate MIME types**, **cache headers** when configured, and **safe path resolution** so only files under the mounted directory are served. See [HTTP & routing](specification/http-routing.md).

## Security

Only place files that are safe to publish under the served directory. Do not point `StaticFiles` at directories containing secrets, private keys, or MATLAB source you do not intend to expose.
