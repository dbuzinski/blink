function plan = buildfile
import matlab.buildtool.Task;
import matlab.buildtool.tasks.*;
import matlab.buildtool.io.FileCollection;
import matlab.addons.toolbox.ToolboxOptions;

plan = buildplan(localfunctions);

plan("clean") = CleanTask();

plan("lint") = CodeIssuesTask(["src/matlab/+blink" "tests"]);

plan("cpp") = Task(Description="Compile C++ dependencies", ...
    Actions=@bazelBuild);

plan("mex") = MexTask(["src/cpp/mexfunctions/**/*.cpp", "dist/bazel-bin/**/*.o"], ...
    "src/matlab/+blink/+internal", ...
    Options=["CXXFLAGS=''$CXXFLAGS -std=c++20''", "-Isrc/cpp/include"], ...
    Filename="serve", ...
    Dependencies="cpp");

plan("test") = TestTask("tests", Dependencies="mex");

plan("package") = PackageTask(Dependencies="mex");

plan.DefaultTasks = ["lint" "test" "package"];
end

function bazelBuild(~)
cd src/cpp;
exitCode = system("bazel build //...");
assert(exitCode == 0, "Bazel build failure.")
end
