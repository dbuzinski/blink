function plan = buildfile
import matlab.buildtool.tasks.*;

plan = buildplan(localfunctions);

plan("clean") = CleanTask();
plan("lint") = CodeIssuesTask(["toolbox" "tests"]);

plan("uWebSockets").Inputs = ["include/uWebSockets/uSockets/src/**/*.c" "include/uWebSockets/src"];
plan("uWebSockets").Outputs = plan("uWebSockets").Inputs(1).replace("src/","").replace(".c", ".o");
plan("forge").Inputs = "include/forge/toolbox/Forge.m";
plan("forge").Outputs = "toolbox/+blink/+internal/Forge.m";
plan("mex") = MexTask([plan("uWebSockets").Outputs.paths, "mex/internal/*.cpp"], ...
    "toolbox/+blink/+internal", ...
    Filename="serve", ...
    Options=["CXXFLAGS=''$CXXFLAGS -std=c++20''", "-Iinclude/uWebSockets/src", "-Iinclude/uWebSockets/uSockets/src", "-lz"], ...
    Dependencies="uWebSockets");
plan("test") = TestTask("tests");
plan("test").Dependencies = ["mex" "forge"];
plan("package").Inputs = ["BlinkToolbox.prj" plan("mex").MexFile, plan("forge").Outputs];
plan("package").Outputs = "Blink.mltbx";

plan.DefaultTasks = ["lint" "test" "package"];
end

function uWebSocketsTask(~)
% Build uWebSockets dependency
cd include/uWebSockets/uSockets
!CFLAGS="$CFLAGS -mmacosx-version-min=11.0" make
cd ..
!CFLAGS="$CFLAGS -mmacosx-version-min=11.0" make default
end

function forgeTask(ctx)
% Include files from Forge dependency
copyfile(ctx.Task.Inputs.Path, ctx.Task.Outputs.Path);
end

function packageTask(ctx)
% Package toolbox
matlab.addons.toolbox.packageToolbox("BlinkToolbox.prj", ctx.Task.Outputs.Path);
end
