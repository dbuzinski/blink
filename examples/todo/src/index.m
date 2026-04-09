app = blink.App;
app.StaticFiles = "static";
port = 8000;
initDb;
connection = sqlite("app.db");
closeConnection = onCleanup(@() close(connection));

renderer = matstache.Renderer;
templates.todo = string(fileread(fullfile("templates", "todo.mustache")));
templates.todoList = string(fileread(fullfile("templates", "todoList.mustache")));

app.get("/", @(req, resp) getTodos(connection, renderer, templates, req, resp));
app.post("/add", @(req, resp) addTodo(connection, renderer, templates, req, resp));
app.post("/toggle/:id", @(req, resp) toggleTodo(connection, renderer, templates, req, resp));
app.delete("/delete/:id", @(req, resp) deleteTodo(connection, req, resp));

fprintf("Listening on port: %d\n", port);
app.serve(Port=port);

function initDb
if ~isfile("app.db")
    c = sqlite("app.db", "create");
    execute(c, "CREATE TABLE todos(id integer primary key autoincrement, text VARCHAR(250), completed BOOLEAN DEFAULT 0);");
end
end

function todoList = getTodoList(connection, renderer, templates)
todos = sqlread(connection, "todos");
ctx.todos = table2struct(todos);
todoList = char(renderer.render(templates.todoList, ctx, struct()));
end

function resp = getTodos(connection, renderer, templates, req, resp)
todos = sqlread(connection, "todos");
ctx.todos = table2struct(todos);
partials.todoList = templates.todoList;
resp = resp.render(renderer, templates.todo, ctx, partials);
end

function resp = addTodo(connection, renderer, templates, req, resp)
data = jsondecode(req.Data);
sqlwrite(connection, "todos", table(string(data.todo), VariableNames="text"));
resp.Data = getTodoList(connection, renderer, templates);
end

function resp = toggleTodo(connection, renderer, templates, req, resp)
execute(connection, sprintf("UPDATE todos SET completed = 1 - completed WHERE id = %s;", req.Parameters.id));
resp.Data = getTodoList(connection, renderer, templates);
end

function resp = deleteTodo(connection, req, resp)
execute(connection, sprintf("DELETE FROM todos WHERE id = %s;", req.Parameters.id));
resp.Data = '';
end
