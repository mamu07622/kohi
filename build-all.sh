#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir"

echo "Building everything..."

if [[ ! -f "engine/build.sh" ]]; then
  echo "Missing engine/build.sh"
  exit 1
fi

if [[ ! -f "testbed/build.sh" ]]; then
  echo "Missing testbed/build.sh"
  exit 1
fi

bash "engine/build.sh"
bash "testbed/build.sh"

echo "All assemblies built successfully."
