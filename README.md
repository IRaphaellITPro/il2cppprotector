# Jeb Protector

Build an ARM64 `libil2cpp.so` wrapper in Termux. This project currently targets `com.herogame.gplay.lastdayrulessurvival` for Android user 0; it is not a general-purpose Unity protector.

## Setup and build

Install a current 64-bit ARM version of Termux, then run:

```sh
pkg install git
git clone REPOSITORY_URL "$HOME/Jeb_Protector"
cd "$HOME/Jeb_Protector"
bash setup.sh
```

Replace `REPOSITORY_URL` with the URL of your GitHub repository. Keep the checkout in Termux private storage, not `/sdcard`. The script installs missing build packages, requests shared-storage access if needed, and asks for the full path to the original ARM64 `libil2cpp.so`. Grant the Android storage permission when prompted; if setup reports that storage is unavailable, grant it and run `bash setup.sh` again.

You can also pass the input path directly:

```sh
bash setup.sh "/full/path/to/original/libil2cpp.so"
```

The finished library is written to `/sdcard/Jeb_Protector/libil2cpp.so`. A successful build replaces any previous output at that path. Keep the original input somewhere else; it is not modified.

Source copies, the generated key, encrypted payload, and compiler intermediates are created inside Termux private storage and removed after the build. The shared-storage output contains only the finished library. The project does not upload the input or output.

## Limitations

- Only 64-bit ARM libraries are supported. The existing runtime loader is tied to the game package above and writes the decrypted payload to that app's private `files/libil2cpp.so` path. Back up any existing file there before using the wrapper.
- Building does not install the library or guarantee compatibility with a particular game version. Test with an app and library you are authorized to modify.
- The encoding is obfuscation, not strong cryptographic protection: the key is recoverable from the built library.
- The repository intentionally excludes original game libraries, generated payloads, and keys. Review redistribution rights for the included third-party sources and `libdobby.a` before publishing; complete license information was not present in the source project.
