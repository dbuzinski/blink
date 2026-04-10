#!/usr/bin/env bash
set -euo pipefail

# bazel_clang_tidy runs clang-tidy inside Bazel's sandbox. On macOS this can
# fail to locate SDK headers unless we pass an explicit sysroot.
sdk_path="$(xcrun --sdk macosx --show-sdk-path)"

exec /usr/local/bin/clang-tidy \
  --extra-arg-before="-isysroot" \
  --extra-arg-before="${sdk_path}" \
  --extra-arg-before="-isystem" \
  --extra-arg-before="${sdk_path}/usr/include/c++/v1" \
  --extra-arg="-isystem" \
  --extra-arg="${sdk_path}/usr/include" \
  "$@"

