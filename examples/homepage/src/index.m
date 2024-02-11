app = blink.App();

app.get("/", @home);
app.StaticFiles = "static";
port = 8000;
fprintf("Listening on port: %d\n", port);
app.serve(Port=port);

function resp = home(~, resp)
ctx.todos = [];
resp = resp.render("templates/home.mtl",ctx);
end
