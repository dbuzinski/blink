# Blink

A lightweight web framework for MATLAB that makes it easy to build web applications and APIs.

## Features

- Lightweight API
- Built on high-performance uWebSockets
- Route parameters and query string parsing
- Static file serving
- Easy integration with MATLAB functions

## Quick Start

### Hello World Example

```matlab
app = blink.App();

app.get("/hello", @hello);
port = 8000;
fprintf("Listening on port: %d\n", port);
app.serve(Port=port)

function resp = hello(req, resp)
    resp.Data = "<h1>Hello world!</h1>";
end
```

Run this example:
```bash
cd examples/hello
matlab -batch "index"
```

Then visit `http://localhost:8000/hello` in your browser.

## Build Instructions

**Requirements:** MATLAB R2024b or later with MATLAB Compiler Toolbox, and Bazel.

Blink uses the [MATLAB Build Tool](https://www.mathworks.com/help/matlab/matlab_prog/overview-of-matlab-build-tool.html) to lint, test, and package the toolbox.

To qualify changes, run the build with the default tasks:
```bash
matlab -batch "buildtool"
```
Run `buildtool -tasks` to see the list of all available tasks.