#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

if [[ -z "${PREFIX:-}" || -z "${HOME:-}" ]]; then
    printf '%s\n' 'Run this script inside Termux.' >&2
    exit 1
fi
main_dir=$(cd -- "$(dirname -- "$0")" && pwd -P)
private_dir=$(cd -- "$PREFIX/.." && pwd -P)
home_dir=$(cd -- "$HOME" && pwd -P)
case "$main_dir/" in
    "$private_dir/"*) ;;
    *) printf '%s\n' 'Clone the project inside Termux private storage, not shared storage.' >&2; exit 1 ;;
esac
case "$home_dir/" in
    "$private_dir/"*) ;;
    *) printf '%s\n' 'HOME must be inside Termux private storage.' >&2; exit 1 ;;
esac
output_dir="${JEB_OUTPUT_DIR:-/sdcard/Jeb_Protector}"

for tool in python3 clang clang++ llvm-objcopy llvm-readelf patchelf; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        printf 'Missing required tool: %s\n' "$tool" >&2
        exit 1
    fi
done

ndk_lib="$PREFIX/aarch64-linux-android/lib"
if [[ ! -f "$ndk_lib/libc++_static.a" || ! -f "$ndk_lib/libc++abi.a" ]]; then
    printf '%s\n' 'Missing NDK libraries. Install them with: pkg install ndk-multilib-native-static' >&2
    exit 1
fi

if (( $# > 1 )); then
    printf 'Usage: bash %s [path/to/libil2cpp.so]\n' "$0" >&2
    exit 1
fi
if (( $# == 1 )); then
    input=$1
else
    read -r -p 'Path to libil2cpp.so: ' input
fi
input="${input#"${input%%[![:space:]]*}"}"
input="${input%"${input##*[![:space:]]}"}"
if [[ -z "$input" ]]; then
    printf '%s\n' 'No input path provided.' >&2
    exit 1
fi
if [[ ${#input} -ge 2 && ( "$input" == \"*\" || "$input" == \'*\' ) ]]; then
    input=${input:1:${#input}-2}
fi
if [[ "$input" == '~/'* ]]; then
    input="$HOME/${input:2}"
fi
if [[ -e "$input" && -e "$output_dir/libil2cpp.so" && "$input" -ef "$output_dir/libil2cpp.so" ]]; then
    printf '%s\n' 'The input library is also the output file; move the input elsewhere first.' >&2
    exit 1
fi

work=$(mktemp -d "$home_dir/.jeb-build.XXXXXXXX")
trap 'rm -rf -- "$work"' EXIT
cp -R -- "$main_dir/jni" "$work/jni"
cp -- "$main_dir/encode.py" "$work/encode.py"
mkdir -p "$work/tmp"
export TMPDIR="$work/tmp"
jni="$work/jni"
python3 "$work/encode.py" "$input"

includes=(
    -I"$jni/include"
    -I"$jni/include/xdl"
    -I"$jni/include/Hook/Dobby"
    -I"$jni/include/Hook/KittyMemory"
    -I"$jni/include/Utils"
    -I"$jni/include/Utils/Unity"
    -I"$jni/include/Utils/il2cpp"
)
flags=(-w -O2 -fPIC -fvisibility=hidden -ffunction-sections -fdata-sections)
shopt -s nullglob
sources=("$jni"/*.cpp "$jni"/src/xdl/*.c "$jni"/src/Hook/KittyMemory/*.cpp "$jni"/src/Utils/*.cpp)
objects=()
for source in "${sources[@]}"; do
    object="$work/${#objects[@]}.o"
    printf 'Compiling %s\n' "${source#"$jni"/}"
    if [[ "$source" == *.c ]]; then
        clang "${flags[@]}" "${includes[@]}" -c "$source" -o "$object"
    else
        clang++ "${flags[@]}" -std=c++17 -fexceptions "${includes[@]}" -c "$source" -o "$object"
    fi
    objects+=("$object")
done

printf '%s\n' 'Linking libil2cpp.so'
clang++ -shared -nostdlib++ -o "$work/libil2cpp.so" \
    "${objects[@]}" \
    "$jni/lib/arm64-v8a/libhack_so_enc.o" \
    "$jni/lib/arm64-v8a/libdobby.a" \
    "$ndk_lib/libc++_static.a" "$ndk_lib/libc++abi.a" \
    -Wl,--gc-sections,--strip-all,--build-id=none,--no-undefined,-soname,libil2cpp.so \
    -llog -landroid -lEGL -lGLESv3 -ldl

patchelf --remove-rpath "$work/libil2cpp.so"
llvm-readelf -h "$work/libil2cpp.so" | grep -q 'AArch64'
mkdir -p "$output_dir"
target_tmp="$output_dir/.libil2cpp.so.$$.tmp"
trap 'rm -f -- "$target_tmp"; rm -rf -- "$work"' EXIT
cp -- "$work/libil2cpp.so" "$target_tmp"
mv -f -- "$target_tmp" "$output_dir/libil2cpp.so"
printf '\nBuild complete: %s/libil2cpp.so\n' "$output_dir"
