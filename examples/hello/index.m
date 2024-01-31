app = cinch.App();

app.get("/hello", @hello);
port = 8000;
fprintf("Listening on port: %d\n", port);
app.serve(Port=port)

function resp = hello(req, resp)
resp.Data = "<h1>Hello world!</h1>";
end