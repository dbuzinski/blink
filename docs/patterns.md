# Patterns and recipes

## JSON APIs

Blink passes the raw body as `req.Data` (a `string`). Decode and encode JSON with MATLAB’s built-ins:

```matlab
function resp = apiAdd(req, resp)
    data = jsondecode(req.Data);
    % ... use data ...
    resp.StatusCode = 201;
    resp.Data = jsonencode(struct("ok", true));
end
```

The todo example (`examples/todo/src/index.m` in the repository) uses `jsondecode` for POST bodies and returns HTML fragments for partial page updates.

## HTML with partial updates

A common pattern is:

1. `GET /` returns a full page (templates or inline HTML).
2. `POST` handlers return **only a fragment** of HTML that client-side JavaScript swaps into the DOM (for example with `fetch` and `innerHTML`).

See the todo app’s `getTodoList` and POST handlers in `examples/todo/src/index.m`.

## Templating (matstache)

Blink does not embed a template engine in `blink.*`. The repo includes the optional **matstache** toolbox (`+matstache`). Typical steps:

1. Load template files as strings (for example with `fileread`).
2. Build a `matstache.Renderer`.
3. Call [`resp.render`](response.md#render-templates) with a template string, context struct, and optional partials struct.

Example flow (abbreviated from the todo example):

```matlab
renderer = matstache.Renderer;
templates.todo = string(fileread(fullfile("templates", "todo.mustache")));
templates.todoList = string(fileread(fullfile("templates", "todoList.mustache")));

function resp = getTodos(~, resp, renderer, templates)
    ctx.todos = table2struct(sqlread(conn, "todos"));
    partials.todoList = templates.todoList;
    resp = resp.render(renderer, templates.todo, ctx, partials);
end
```

Matstache is documented alongside its sources under `include/matstache/toolbox/`; it is a **companion** package, not a core `blink` dependency for minimal apps.

## SQLite and databases

The todo example uses MATLAB’s `sqlite` interface. Database usage is **not** part of Blink—use whatever persistence fits your deployment.
