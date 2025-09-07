function plan = buildfile
import matlab.buildtool.tasks.*;

plan = buildplan(localfunctions);

plan("clean") = CleanTask();
plan("installDeps").Inputs.PackageJson = plan.files("package.json");
plan("installDeps").Outputs.NodeModules = plan.files("node_modules");
plan("buildCss").Dependencies = "installDeps";
plan("buildCss").Inputs.Content = plan.files(["templates/**/*.html, templates/**/*.mtl"]);
plan("buildCss").Inputs.DirectivesFile = plan.files("styles/app.css");
plan("buildCss").Outputs.CssFile = plan.files("static/css/app.css");
plan("build").Inputs = ["src", "static", "templates"];
plan("build").Outputs = "dist";
plan("build").Dependencies = "buildCss";

plan.DefaultTasks = "build";
end

function installDepsTask(ctx)
!npm install
end

function buildCssTask(ctx)
directives = ctx.Task.Inputs.DirectivesFile.Path;
cssFile = ctx.Task.Outputs.CssFile.Path;
cmd = sprintf("npx tailwindcss -i %s -o %s --minify", directives, cssFile);
system(cmd);
end

function buildTask(ctx)
[~,~] = mkdir("dist");
!cp -r static dist/
!cp -r templates dist/
!cp -r src/ dist/
end
