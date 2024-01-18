app = cinch.App();

app.get("/hello/:name", @hello);
app.get("/greet", @greet);
app.post("/data", @data);

port = 8000;
fprintf("Listening on port: %d\n", port);
app.serve(Port=port)

function resp = hello(req)
disp(req.Data);
resp = sprintf('<h1>Hello %s!</h1>', req.Parameters.name);
end

function resp = greet(req)
disp(req.Data);
resp = sprintf('<h1>Greetings %s!</h1>', req.Query.name);
end


function resp = data(req)
disp(jsondecode(req.Data));
user.id = 1;
user.username = "davidb";
user.name = "David Buzinski";
resp = jsonencode(user);
end