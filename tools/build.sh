#!/usr/bin/env bash
set -e

echo "[MiniNet] Configure..."
cmake -S . -B build

echo "[MiniNet] Build..."
cmake --build build -j
