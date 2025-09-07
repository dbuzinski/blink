function plan = buildfile
import matlab.buildtool.Task;
import matlab.buildtool.tasks.*;
import matlab.buildtool.io.FileCollection;
import matlab.addons.toolbox.ToolboxOptions;

plan = buildplan(localfunctions);

plan("clean") = CleanTask();
plan("lint") = CodeIssuesTask(["src/+blink" "tests"]);
plan("source") = Task(Description="Add m source to toolbox", ...
    Actions=@copySource);
plan("source").Inputs.Folders = FileCollection.fromPaths("src/+blink");
plan("source").Outputs.Folders = plan("source").Inputs.Folders.replace("src", fullfile("build", "toolbox"));
plan("mex") = MexTask(["src/mex/**/*.cpp", "src/cpp/bazel-bin/**/*.o"], ...
    "build/toolbox/+blink/+internal", ...
    Options=["CXXFLAGS=''$CXXFLAGS -std=c++20''", "-Isrc/cpp/include"], ...
    Filename="serve", ...
    Dependencies="cpp");
plan("cpp") = Task(Description="Compile C++ dependencies", ...
    Actions=@bazelBuild);
plan("test") = TestTask("tests");
plan("test").Dependencies = ["source", "mex"];
plan("package") = Task(Description="Package toolbox", ...
    Actions=@packageToolbox, ...
    Dependencies=["source", "mex"], ...
    Outputs="build/blink.mltbx");
plan("package").Inputs.ToolboxFolder = plan.files("build/toolbox");
plan("package").Inputs.ToolboxOptions = @()ToolboxOptions( ...
    "build/toolbox", ...
    "15D3F48E-0EA8-437E-B257-92FBFD1E1DD1", ...
    ToolboxName="Blink", ...
    ToolboxVersion="0.1.0", ...
    Description="Blink is a web framework for MATLAB", ...
    AuthorName="David Buzinski", ...
    AuthorEmail="davidbuzinski@gmail.com", ...
    OutputFile="build/blink.mltbx" ...
);

plan.DefaultTasks = ["package" "test"];
end

function bazelBuild(~)
cd src/cpp/
exitCode = system("bazel build //...");
assert(exitCode == 0, "Bazel build failure.")
end

function copySource(ctx)
folders = ctx.Task.Inputs.Folders.paths();
for folder = folders(:)'
    [~,folderName] = fileparts(folder);
    copyfile(folder, fullfile("build", "toolbox", folderName));
end
end

function packageToolbox(ctx)
toolboxOptions = ctx.Task.Inputs.ToolboxOptions();
matlab.addons.toolbox.packageToolbox(toolboxOptions);
end
