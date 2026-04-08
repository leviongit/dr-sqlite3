#! /bin/bash

set -ex

CC=${CC:-clang}
CFLAGS=("-Wall" "-Wextra" "-fpic" "-Iinclude" "-Iinclude/dr" "-std=c23")
declare FORCE
declare -a COMPILED

function compile {
  test $# -lt 0 && return 1
  local file=$1; shift
  local source="src/${file}.c"
  local out="build/${file}.o"
  test "${source}" -nt "${out}" -o \( -n "${FORCE}" -a "${file}" != "sqlite3" \) -o "${FORCE:=0}" -gt 1 &&
    "${CC}" -c "${CFLAGS[@]}" "${source}" -o "${out}" 
  COMPILED+=("${out}")
}

while test $# -gt 0; do
  if test $1 = "-f" -a "${FORCE:=0}" -lt 1; then
    FORCE=1
  elif test $1 = "-ff"; then
    FORCE=2
  fi
  shift
done

compile sqliteconnection
compile sqlitequery
compile bridge
compile sqlite3

"${CC}" "${COMPILED[@]}" "${CFLAGS[@]}" -shared -o "$(pwd)/../mygame/native/linux-amd64/libsqlite3.so"
