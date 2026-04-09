# Installation

## From a packaged toolbox

If you have a `.mltbx` built by this project’s MATLAB Build Tool, install it from the MATLAB **Home** tab: **Add-Ons** → **Install from File**, then select the toolbox file.

That puts `+blink` on the MATLAB path for that installation.

## From source (development)

1. Clone the repository and open MATLAB at the repo root.

2. Add the framework sources to your path:

   ```matlab
   addpath(fullfile(pwd, "src"));
   ```

3. Build native artifacts with the [MATLAB Build Tool](https://www.mathworks.com/help/matlab/matlab_prog/overview-of-matlab-build-tool.html):

   ```matlab
   buildtool
   ```

   The default plan compiles MEX files and packages the toolbox. See the repository `README` for Bazel and toolchain requirements.

## Verify

After `addpath` and a successful build:

```matlab
which blink.internal.serve   % should resolve to a MEX file
```

`blink.internal.serve` is used internally by [`blink.App`](application.md). You do not call it from application code; it is not part of the public API.

## Building this documentation

From the repository root, use a virtual environment (recommended on macOS/Homebrew Python):

```bash
python3 -m venv .venv
source .venv/bin/activate   # Windows: .venv\Scripts\activate
pip install -r requirements-docs.txt
mkdocs serve
```

Open the URL printed in the terminal to preview the site. Use `mkdocs build --strict` to produce static HTML under `site/` (ignored by git).
