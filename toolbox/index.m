app = cinch.App();

app.get("/hello", @hello);

port = 8000;
fprintf("Listening on port: %d\n", port);
app.serve(Port=port)

function resp = hello(~)
resp = '<h1>Hello World!</h1>';
end

% function resp = greet(req)
% resp = sprintf('<h1>Hi %s!</h1>', req.params.name);
% end
% 
% 
% function resp = data(~)
% user.id = 1;
% user.username = "davidb";
% user.name = "David Buzinski";
% resp = jsonencode(user);
% end