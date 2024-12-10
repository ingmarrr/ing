CC      := "gcc"

SOURCES     := `find . -name "*.c"`
BUILD_DIR   := "build"
PROGRAM     := "py"

# -D_FORTIFY_SOURCE=2
std_flags   := "-Wall -Wextra -Wpedantic -Werror -Wno-gnu-zero-variadic-macro-arguments"
opt_flags   := "-O3 -march=native -flto -ffast-math"
dbg_flags   := "-O0 -g3 -fno-omit-frame-pointer"
wrn_flags   := "-Wformat=2 -Wcast-align -Wcast-qual -Wconversion -Wshadow -Wpointer-arith -Wstrict-aliasing=2"
sec_flags   := "-fstack-protector-strong -fPIE"
# vet_flags   := "-fsanitize=address,undefined"
vet_flags   := "-fsanitize=undefined"

bin_includes := "-I."

default :
    @just --list

release :
    {{CC}} {{std_flags}} {{opt_flags}} {{wrn_flags}} {{sec_flags}} {{SOURCES}} "-o" {{BUILD_DIR}}/{{PROGRAM}}
debug   :
    {{CC}} {{std_flags}} {{dbg_flags}} {{wrn_flags}} {{sec_flags}} {{SOURCES}} "-o" {{BUILD_DIR}}/{{PROGRAM}}_dbg
quick   :
    {{CC}} {{std_flags}} "-O2" {{SOURCES}} "-o" {{BUILD_DIR}}/{{PROGRAM}}
vet     :
    {{CC}} {{std_flags}} {{dbg_flags}} {{vet_flags}} {{SOURCES}} "-o" {{BUILD_DIR}}/{{PROGRAM}}

build FLAGS:
    {{CC}} {{FLAGS}} {{SOURCES}} "-o" {{BUILD_DIR}}/{{PROGRAM}}

clean   :
    @rm -rf {{BUILD_DIR}}
    @mkdir {{BUILD_DIR}}

show-flags:
    @echo "FLAGS:"
    @echo "[base]           {{std_flags}}"
    @echo "[optimisation]   {{opt_flags}}"
    @echo "[debug]          {{dbg_flags}}"
    @echo "[warning]        {{wrn_flags}}"
    @echo "[security]       {{sec_flags}}"

example ARG:
    @{{CC}} {{std_flags}} {{vet_flags}} {{bin_includes}} examples/e{{ARG}}.c "-o" {{BUILD_DIR}}/e{{ARG}}
    @./{{BUILD_DIR}}/e{{ARG}}

build-bin ARG:
    @{{CC}} {{std_flags}} {{bin_includes}} bins/{{ARG}}/main.c "-o" {{BUILD_DIR}}/{{ARG}}

run-bin ARG:
    @{{CC}} {{std_flags}} {{bin_includes}} bins/{{ARG}}/main.c "-o" {{BUILD_DIR}}/{{ARG}}
    @./{{BUILD_DIR}}/{{ARG}}

run:
    ./{{BUILD_DIR}}/{{PROGRAM}}
