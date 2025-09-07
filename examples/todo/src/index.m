app = blink.App;
app.StaticFiles = "static";
port = 8000;
initDb;
connection = sqlite("app.db");
closeConnection = onCleanup(@() close(connection));

app.get("/", @(req, resp) getTodos(connection, req, resp));
app.post("/add", @(req, resp) addTodo(connection, req, resp));
app.delete("/delete/:id", @(req, resp) deleteTodo(connection, req, resp));
app.post("/toggle/:id", @(req, resp) toggleTodo(connection, req, resp));

fprintf("Listening on port: %d\n", port);
app.serve(Port=port);

function initDb
if ~isfile("app.db")
    c = sqlite("app.db", "create");
    sqlquery = strcat("CREATE TABLE todos(id integer primary key autoincrement, text VARCHAR(250), completed BOOLEAN DEFAULT 0);");
    execute(c,sqlquery);
end
end

function todoList = getTodoList(connection)
forge = blink.internal.Forge();
todos = sqlread(connection, "todos");
ctx.todos = table2struct(todos);
tpl = string(fileread("templates/todoList.mtl"));
todoList = forge.render(tpl, ctx);
end

function resp = addTodo(connection, req, resp)
data = jsondecode(req.Data);
t = table(string(data.todo), VariableNames="text");
sqlwrite(connection, "todos", t);
resp.Data = getTodoList(connection);
end

function resp = getTodos(connection, req, resp)
ctx.todoList = getTodoList(connection);
resp = resp.render("templates/todo.mtl", ctx);
end

function resp = toggleTodo(connection, req, resp)
sqlquery = sprintf("UPDATE todos SET completed = ~completed WHERE id = %s;", req.Parameters.id);
execute(connection,sqlquery);
resp.Data = getTodoList(connection);
end

function resp = deleteTodo(connection, req, resp)
sqlquery = sprintf("DELETE FROM todos WHERE id = %s;", req.Parameters.id);
execute(connection, sqlquery);
end
