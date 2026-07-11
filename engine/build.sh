#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir"

if [[ -z "${VULKAN_SDK:-}" ]]; then
  sdk_root="${HOME}/VulkanSDK"
  if [[ -d "$sdk_root" ]]; then
    latest_sdk_dir="$(find "$sdk_root" -mindepth 1 -maxdepth 1 -type d -name '1.*' | sort -V | tail -n 1 || true)"
    if [[ -n "$latest_sdk_dir" && -d "$latest_sdk_dir/macOS" ]]; then
      VULKAN_SDK="$latest_sdk_dir/macOS"
      export VULKAN_SDK
    fi
  fi
fi

if [[ -z "${VULKAN_SDK:-}" ]]; then
  echo "VULKAN_SDK is not configured. Set VULKAN_SDK before building."
  exit 1
fi

if [[ ! -d "${VULKAN_SDK}/include" || ! -d "${VULKAN_SDK}/lib" ]]; then
  echo "VULKAN_SDK is set but appears invalid: ${VULKAN_SDK}"
  exit 1
fi

if [[ ! -d "src" ]]; then
  echo "Missing source directory: src"
  exit 1
fi

source_files=()
while IFS= read -r -d '' file; do
  source_files+=("$file")
done < <(find "src" -type f -name '*.cpp' -print0 | sort -z)

if (( ${#source_files[@]} == 0 )); then
  echo "No C++ source files found under src."
  exit 1
fi

mkdir -p "../bin"

compiler="${CXX:-clang++}"
assembly="engine"
output="../bin/lib${assembly}.so"

compiler_flags=(-g -shared -Wvarargs -Wall -Werror)
defines=(-D_DEBUG -DKEXPORT)
include_flags=(-Isrc "-I${VULKAN_SDK}/include")
linker_flags=("-L${VULKAN_SDK}/lib" -lvulkan)

case "$(uname -s)" in
  Darwin)
    output="../bin/lib${assembly}.dylib"
    linker_flags+=(-Wl,-install_name,@rpath/lib${assembly}.dylib)
    linker_flags+=(-Wl,-rpath,"${VULKAN_SDK}/lib")
    linker_flags+=(-framework Cocoa)
    ;;
  Linux)
    output="../bin/lib${assembly}.so"
    ;;
  *)
    echo "Unsupported OS for this script: $(uname -s)"
    exit 1
    ;;
esac

echo "Building ${assembly}..."
"$compiler" "${source_files[@]}" "${compiler_flags[@]}" -o "$output" "${defines[@]}" "${include_flags[@]}" "${linker_flags[@]}"
echo "Build succeeded: $output"
