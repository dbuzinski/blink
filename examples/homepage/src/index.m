app = blink.App();
renderer = matstache.Renderer;
srcDir = fileparts(mfilename("fullpath"));
templates = struct("home", string(fileread(fullfile(srcDir, "..", "templates", "home.mtl"))));

app.get("/", @(req, resp) home(req, resp, renderer, templates));
app.StaticFiles = "static";
port = 8000;
fprintf("Listening on port: %d\n", port);
app.serve(Port=port);

function resp = home(~, resp, renderer, templates)
ctx.todos = [];
resp = resp.render(renderer, templates.home, ctx);
end
