app = blink.App;
app.StaticFiles = "static";
port = 8000;
initDb;

app.get("/", @getTodos);
app.post("/add", @addTodo);
app.delete("/delete/:id", @deleteTodo);
app.post("/toggle/:id", @toggleTodo);

fprintf("Listening on port: %d\n", port);
app.serve(Port=port);

function initDb
if ~isfile("app.db")
    c = sqlite("app.db", "create");
    sqlquery = strcat("CREATE TABLE todos(id integer primary key autoincrement, text VARCHAR(250), completed BOOLEAN DEFAULT 0);");
    execute(c,sqlquery);
end
end

function todoList = getTodoList
forge = blink.internal.forge.Forge();
c = sqlite("app.db");
todos = sqlread(c, "todos");
close(c);
ctx.todos = table2struct(todos);
tpl = string(fileread("templates/todoList.mtl"));
todoList = forge.render(tpl, ctx);
end

function resp = addTodo(req, resp)
c = sqlite("app.db");
data = jsondecode(req.Data);
t = table(string(data.todo), VariableNames="text");
sqlwrite(c, "todos", t);
close(c);
resp.Data = getTodoList();
end

function resp = getTodos(~, resp)
ctx.todoList = getTodoList();
resp = resp.render("templates/todo.mtl", ctx);
end

function resp = toggleTodo(req, resp)
c = sqlite("app.db");
sqlquery = sprintf("UPDATE todos SET completed = ~completed WHERE id = %s;", req.Parameters.id);
execute(c,sqlquery);
close(c);
resp.Data = getTodoList();
end

function resp = deleteTodo(req, resp)
c = sqlite("app.db");
sqlquery = sprintf("DELETE FROM todos WHERE id = %s;", req.Parameters.id);
execute(c,sqlquery);
close(c);
end
