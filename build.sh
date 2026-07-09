#!/usr/bin/env bash
# Flexible build script for RPGGame.
#
# Auto-discovers every .cpp under src/ (except enemy_data.cpp, which is
# intentionally #included inside src/game/enemy.cpp) so newly added source
# files are compiled without editing this script.
#
# Usage:
#   ./build.sh            # build rpg (or rpg.exe on Windows) with -O2
#   ./build.sh debug      # build with -g -O0 and no optimization
#   ./build.sh clean      # remove build artifacts
#   ./build.sh run        # build then execute

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$ROOT/src"
OUT="$ROOT/rpg"

CXX="${CXX:-g++}"
STD="-std=c++17"
INC="-I $SRC_DIR"

# Collect sources, excluding the intentionally-included enemy_data.cpp.
mapfile -t SOURCES < <(find "$SRC_DIR" -name '*.cpp' ! -name 'enemy_data.cpp' | sort)

if [[ "${1:-}" == "clean" ]]; then
    echo "Cleaning build artifacts..."
    rm -f "$OUT" "$OUT.exe"
    find "$SRC_DIR" -name '*.o' -delete
    echo "Done."
    exit 0
fi

if [[ "${1:-}" == "debug" ]]; then
    CXXFLAGS="-g -O0"
    echo "Building in DEBUG mode..."
else
    CXXFLAGS="-O2"
    echo "Building in RELEASE mode..."
fi

# Use a Windows-style executable name when on Windows.
if [[ "$(uname -s 2>/dev/null || echo MINGW)" == *MINGW* || "$(uname -s 2>/dev/null || echo MSYS)" == *MSYS* ]]; then
    OUT="$ROOT/rpg.exe"
fi

echo "Compiling ${#SOURCES[@]} source file(s)..."
"$CXX" $STD $CXXFLAGS $INC "${SOURCES[@]}" -o "$OUT"

echo "Build succeeded: $OUT"

if [[ "${1:-}" == "run" ]]; then
    echo "Running..."
    "$OUT"
fi
