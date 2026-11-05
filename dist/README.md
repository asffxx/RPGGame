# RPGGame v3.7.0 packages

Console builds with statically linked C++ runtimes. Linux requires system glibc;
Windows uses standard system DLLs. Both require the matching `content/` folder.

| Platform | Executable |
| --- | --- |
| Linux x86_64 | `releases/linux-x86_64/RPGGame-v3.7.0-linux-x86_64` |
| Windows 10/11 x64 | `releases/windows-x64/RPGGame-v3.7.0-windows-x64.exe` |

Copy the complete platform folder, including its `content/` directory, license,
instructions, and checksums. Downloadable archives are provided on
[GitHub Releases](https://github.com/asffxx/RPGGame/releases).

## Run

Linux:

```sh
cd releases/linux-x86_64
chmod +x RPGGame-v3.7.0-linux-x86_64
./RPGGame-v3.7.0-linux-x86_64
```

Windows (PowerShell or cmd):

```powershell
cd releases\windows-x64
.\RPGGame-v3.7.0-windows-x64.exe
```

Run from a writable directory alongside the `content/` folder.
Saves are written to `savegame.dat` in the current working directory.

The game needs an 80-column terminal. Use `NO_COLOR=1` to disable colors,
or set `TERM=dumb` for plain text output. Set `RPG_CONTENT_DIR=/path/to/content`
to override the default content directory location.

## Verify

Each platform folder contains `SHA256SUMS` for its complete payload. From that
folder, run `sha256sum -c SHA256SUMS` on Linux. The checksum file beside this
document covers both platform executables and is checked from `dist/`.

On Windows:

```powershell
Get-FileHash .\RPGGame-v3.7.0-windows-x64.exe -Algorithm SHA256
```

## Build from source

See the [source build instructions](../README.md#build-from-source).
The Linux build links libgcc and libstdc++ statically.
The Windows build uses LLVM-MinGW targeting x64 and UCRT.
