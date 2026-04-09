# Quick start

Minimal HTTP server that responds on `/hello`.

```matlab
app = blink.App();

app.get("/hello", @hello);
port = 8000;
fprintf("Listening on port: %d\n", port);
app.serve(Port=port);

function resp = hello(~, resp)
    resp.Data = "<h1>Hello world!</h1>";
end
```

Save as `index.m`, `cd` to that folder, then run:

```bash
matlab -batch "index"
```

In a browser, open `http://localhost:8000/hello`.

!!! note "`serve` blocks"

    `app.serve` runs the embedded HTTP server and **blocks** the MATLAB session until the process exits (for example when you stop MATLAB). Plan your workflow accordingly.

## Next steps

- [Application](application.md) — ports, static files, all HTTP verbs
- [Routing](routing.md) — path parameters such as `/item/:id`
- [Examples](examples.md) — hello, todo, and homepage samples in the repo
