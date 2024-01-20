app = cinch.App();

app.get("/hello/:name", @hello);
app.get("/greet", @greet);
app.post("/data", @data);

port = 8000;
fprintf("Listening on port: %d\n", port);
app.serve(Port=port)

function resp = hello(req, resp)
arguments
    req cinch.Request
    resp cinch.Response
end
resp.Data = sprintf("<h1>Hello %s!</h1>", req.Parameters.name);
end

function resp = greet(req, resp)
arguments
    req cinch.Request
    resp cinch.Response
end
resp.Data = sprintf("<h1>Greetings %s!</h1>", req.Query.name);
end


function resp = data(req, resp)
arguments
    req cinch.Request
    resp cinch.Response
end
data = jsondecode(req.Data);
if any(~isfield(data, ["id", "username", "name"]))
    resp.StatusCode = 400;
    resp.Data = "Error: Bad request.";
    return
end
user.id = data.id;
user.username = data.username;
user.name = data.name;
resp.Data = jsonencode(user);
disp(resp);
end