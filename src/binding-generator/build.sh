#!/bin/sh
set -xe

INCDIR="/home/segfault/Programming/srcj/vendor/llvm-lldb-clang-lld-20.1.7-Release-Dynamic/include"
LIBDIR="/home/segfault/Programming/srcj/vendor/llvm-lldb-clang-lld-20.1.7-Release-Dynamic/lib"
OUT="main"
SRC="binding.cpp"
RPATH="$LIBDIR"

clang++ \
  -std=c++20 \
  -Wall \
  -Wextra \
  -ggdb \
  -I"$INCDIR" \
  -o "$OUT" "$SRC" \
  -L"$LIBDIR" \
  -l:libclang.so \
  -Wl,-rpath,"$RPATH"

cat > compile_flags.txt <<EOF
-std=c++20
-Wall
-Wextra
-I$INCDIR
