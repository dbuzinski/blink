function plan = buildfile
import matlab.buildtool.Task;
import matlab.buildtool.tasks.*;
import matlab.buildtool.io.FileCollection;
import matlab.addons.toolbox.ToolboxOptions;

plan = buildplan(localfunctions);

plan("clean") = CleanTask();
plan("lint") = CodeIssuesTask(["src/+blink" "tests"]);
plan("mex") = MexTask(["src/mex/**/*.cpp", "src/cpp/bazel-bin/**/*.o"], ...
    "src/+blink/+internal", ...
    Options=["CXXFLAGS=''$CXXFLAGS -std=c++20''", "-Isrc/cpp/include"], ...
    Filename="serve", ...
    Dependencies="cpp");
plan("cpp") = Task(Description="Compile C++ dependencies", ...
    Actions=@bazelBuild);
plan("test") = TestTask("tests");
plan("test").Dependencies = "mex";
plan("mkdocs") = Task(Description="Build documentation", ...
    Actions=@buildMkdocs);
% plan("package") = Task(Description="Package toolbox", ...
%     Actions=@packageToolbox, ...
%     Dependencies=["matstache", "mex"], ...
%     Outputs="blink.mltbx");
% plan("package").Inputs.ToolboxFolder = plan.files("build/toolbox");
% plan("package").Inputs.ToolboxOptions = @()ToolboxOptions( ...
%     "build/toolbox", ...
%     "15D3F48E-0EA8-437E-B257-92FBFD1E1DD1", ...
%     ToolboxName="Blink", ...
%     ToolboxVersion="0.1.0", ...
%     Description="Blink is a web framework for MATLAB", ...
%     AuthorName="David Buzinski", ...
%     AuthorEmail="davidbuzinski@gmail.com", ...
%     OutputFile="build/blink.mltbx" ...
% );

plan.DefaultTasks = ["lint" "mex" "test"];
end

function bazelBuild(~)
cd src/cpp/
exitCode = system("bazel build //...");
assert(exitCode == 0, "Bazel build failure.")
end

function buildMkdocs(~)
projectRoot = fileparts(mfilename("fullpath"));
oldDir = cd(projectRoot);
cleanupObj = onCleanup(@() cd(oldDir));
pythonExe = resolvePythonForMkdocs(projectRoot);
cmd = sprintf("%s -m mkdocs build --strict", pythonExe);
[status, cmdout] = system(cmd);
if status ~= 0
    fprintf("%s", cmdout);
    error("MkDocs build failed (exit code %d). Install docs deps: pip install -r requirements-docs.txt", status);
end
end

function pythonExe = resolvePythonForMkdocs(projectRoot)
if ispc
    venvPy = fullfile(projectRoot, ".venv", "Scripts", "python.exe");
else
    venvPy = fullfile(projectRoot, ".venv", "bin", "python3");
end
if isfile(venvPy)
    pythonExe = quoteIfNeeded(venvPy);
else
    pythonExe = "python3";
end
end

function s = quoteIfNeeded(pathStr)
if contains(pathStr, " ")
    s = ['"' pathStr '"'];
else
    s = pathStr;
end
end

function copyFiles(ctx)
srcFolders = ctx.Task.Inputs.Folders.paths();
dest = ctx.Task.Outputs.DestinationFolder.Path;
for folder = srcFolders
    copyfile(folder, dest);
end
end

function packageToolbox(ctx)
toolboxOptions = ctx.Task.Inputs.ToolboxOptions();
matlab.addons.toolbox.packageToolbox(toolboxOptions);
end
