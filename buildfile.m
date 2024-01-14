function plan = buildfile
import matlab.buildtool.tasks.*;

plan = buildplan(localfunctions);

plan("clean") = CleanTask();
plan("lint") = CodeIssuesTask(["toolbox" "tests"]);
plan("buildDeps").Inputs = ["mex/include/uWebSockets/src" "mex/include/uWebSockets/uSockets/src"];
plan("buildDeps").Outputs = "mex/include/uWebSockets/uSockets/*.o";
plan("mex") = MexTask([plan("buildDeps").Outputs.paths, "mex/internal/serve.cpp"], ...
    "toolbox/+blaze/+internal", ...
    Filename="serve", ...
    Options=["CXXFLAGS=''$CXXFLAGS -std=c++20 -Wall''", "-Imex/include/uWebSockets/src", "-Imex/include/uWebSockets/uSockets/src", "-lz"], ...
    Dependencies="buildDeps");
plan("test") = TestTask("tests");

plan.DefaultTasks = "mex";
end

function buildDepsTask(~)
% Build uWebSockets dependency
!git submodule update --init --recursive
cd mex/include/uWebSockets/uSockets
!CFLAGS="$CFLAGS -mmacosx-version-min=11.0" make
cd ..
!CFLAGS="$CFLAGS -mmacosx-version-min=11.0" make default
end