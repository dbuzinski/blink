function plan = buildfile
import matlab.buildtool.Task;
import matlab.buildtool.tasks.*;
import matlab.buildtool.io.FileCollection;
import matlab.addons.toolbox.ToolboxOptions;

plan = buildplan(localfunctions);

plan("clean") = CleanTask();

plan("lint") = CodeIssuesTask(["toolbox/+blink" "tests"]);

plan("cpp") = Task(Description="Compile C++ dependencies", ...
    Actions=@bazelBuild);

plan("mex") = MexTask(["cpp/mexfunctions/**/*.cpp", "dist/bazel-bin/**/*.o"], ...
    "toolbox/+blink/+internal", ...
    Options=["CXXFLAGS=''$CXXFLAGS -std=c++20''", "-Icpp/include"], ...
    Filename="serve", ...
    Dependencies="cpp");

plan("test") = TestTask("tests");
plan("test").Dependencies = "mex";

plan("package") = Task(Description="Package toolbox", ...
    Actions=@packageToolbox, ...
    Dependencies="mex", ...
    Outputs="dist/blink.mltbx");
plan("package").Inputs.ToolboxFolder = plan.files("toolbox");
plan("package").Inputs.ToolboxOptions = @()ToolboxOptions( ...
    "toolbox", ...
    "15D3F48E-0EA8-437E-B257-92FBFD1E1DD1", ...
    ToolboxName="Blink", ...
    ToolboxVersion="0.1.0", ...
    Description="Blink is a web framework for MATLAB", ...
    AuthorName="David Buzinski", ...
    AuthorEmail="davidbuzinski@gmail.com", ...
    OutputFile="dist/blink.mltbx" ...
);

plan("pythonVenv") = Task(Description="Setup Python environment for mkdocs", ...
    Actions=@pythonVenv, ...
    Outputs=".venv");

plan("mkdocs") = Task(Description="Build documentation", ...
    Actions=@buildMkdocs, ...
    Dependencies="pythonVenv", ...
    Inputs=["docs", "mkdocs.yml"], ...
    Outputs="site");

plan("build") = Task(Description = "Run full build", ...
    Dependencies=["lint", "test", "package", "mkdocs"]);

plan.DefaultTasks = "build";
end

function bazelBuild(~)
cd cpp;
exitCode = system("bazel test //...");
assert(exitCode == 0, "Bazel build failure.")
end

function pythonVenv(~)
py = fullfile(".venv", "bin", "python3");
if ~isfile(py)
    exitCode = system("python3 -m venv .venv");
    assert(exitCode == 0, "Failed to create Python venv in .venv")
end
exitCode = system(sprintf("%s -m pip install -r requirements-docs.txt", py));
assert(exitCode == 0, "pip install failed for requirements-docs.txt")
end

function buildMkdocs(~)
py = fullfile(".venv", "bin", "python3");
[status, cmdout] = system(sprintf("%s -m mkdocs build --strict", py));
assert(status == 0, "MkDocs build failed (exit code %d). %s", status, cmdout);
end

function packageToolbox(ctx)
toolboxOptions = ctx.Task.Inputs.ToolboxOptions();
matlab.addons.toolbox.packageToolbox(toolboxOptions);
end
