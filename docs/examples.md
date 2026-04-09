# Examples

Examples live under the repository `examples/` directory (paths below are relative to the repo root).

## `examples/hello`

Minimal single-file app: one `GET` route and a fixed HTML response. Good starting point after [Quick start](quick-start.md).

- Entry file: `examples/hello/index.m`

## `examples/todo`

Full-page HTML app with:

- SQLite persistence
- JSON POST bodies and HTML partial responses
- Route parameters (`:id`)
- matstache templates and `Response.render`
- Tailwind-built CSS under `static/`

Key files: `examples/todo/src/index.m`, templates under `examples/todo/templates/`.

## `examples/homepage`

Smaller matstache demo with static files—useful for seeing `StaticFiles` plus a single routed page.

- Entry file: `examples/homepage/src/index.m`

## Running

From the example directory (adjust for your MATLAB setup):

```bash
cd examples/hello
matlab -batch "index"
```

Then open `http://localhost:<port>/...` as described in each example’s `fprintf` or comments.
