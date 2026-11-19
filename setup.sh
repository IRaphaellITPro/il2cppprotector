#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

if [[ -z "${PREFIX:-}" || -z "${HOME:-}" ]] || ! command -v pkg >/dev/null 2>&1; then
    printf '%s\n' 'Run this script inside Termux.' >&2
    exit 1
fi
project_dir=$(cd -- "$(dirname -- "$0")" && pwd -P)
private_dir=$(cd -- "$PREFIX/.." && pwd -P)
case "$project_dir/" in
    "$private_dir/"*) ;;
    *) printf '%s\n' 'Clone the project inside Termux private storage, not shared storage.' >&2; exit 1 ;;
esac
if [[ "$(dpkg --print-architecture)" != aarch64 ]]; then
    printf '%s\n' 'A 64-bit ARM Termux installation is required.' >&2
    exit 1
fi

missing=()
for package in python clang llvm patchelf ndk-multilib-native-static; do
    if ! dpkg-query -W -f='${Status}' "$package" 2>/dev/null | grep -q '^install ok installed$'; then
        missing+=("$package")
    fi
done
if (( ${#missing[@]} )); then
    printf 'Installing missing Termux packages: %s\n' "${missing[*]}"
    pkg install -y "${missing[@]}"
fi

output_dir="${JEB_OUTPUT_DIR:-/sdcard/Jeb_Protector}"
case "$output_dir/" in
    /sdcard/*|/storage/emulated/*|/storage/self/primary/*)
        if [[ ! -d "$HOME/storage/shared" ]]; then
            printf '%s\n' 'Grant Termux access to shared storage when prompted.'
            termux-setup-storage
        fi
        ;;
esac
if ! mkdir -p -- "$output_dir" || [[ ! -w "$output_dir" ]]; then
    printf '%s\n' 'Cannot write to the output directory. Grant storage access and run setup.sh again.' >&2
    exit 1
fi

exec bash "$project_dir/app/src/main/build-termux.sh" "$@"
