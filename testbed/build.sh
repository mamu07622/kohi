#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir"

if [[ ! -d "src" ]]; then
  echo "Missing source directory: src"
  exit 1
fi

cpp_files=()
while IFS= read -r -d '' file; do
  cpp_files+=("$file")
done < <(find "src" -type f -name '*.cpp' -print0 | sort -z)

if (( ${#cpp_files[@]} == 0 )); then
  echo "No C++ source files found under src."
  exit 1
fi

mkdir -p "../bin"

compiler="${CXX:-clang++}"
assembly="testbed"
output="../bin/${assembly}"

compiler_flags=(-g -Wall -Wextra)
defines=(-D_DEBUG -DKIMPORT)
include_flags=(-Isrc -I../engine/src)
linker_flags=(-L../bin -lengine)

case "$(uname -s)" in
  Darwin)
    linker_flags+=(-Wl,-rpath,@executable_path)
    ;;
  Linux)
    linker_flags+=(-Wl,-rpath,'$ORIGIN')
    ;;
  *)
    echo "Unsupported OS for this script: $(uname -s)"
    exit 1
    ;;
esac

echo "Building ${assembly}..."
"$compiler" "${cpp_files[@]}" "${compiler_flags[@]}" -o "$output" "${defines[@]}" "${include_flags[@]}" "${linker_flags[@]}"
echo "Build succeeded: $output"
