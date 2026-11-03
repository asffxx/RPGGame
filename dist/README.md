# RPGGame v3.6.0 prebuilt binaries

Console builds with statically linked C++ runtimes. Linux requires system glibc;
Windows uses standard system DLLs. Both require the matching `content/` folder.

| Platform | File |
| --- | --- |
| Linux x86_64 | `RPGGame-v3.6.0-linux-x86_64` |
| Windows 10/11 x64 | `RPGGame-v3.6.0-windows-x64.exe` |

## Run

Linux:

```sh
chmod +x RPGGame-v3.6.0-linux-x86_64
./RPGGame-v3.6.0-linux-x86_64
```

Windows (PowerShell or cmd):

```powershell
.\RPGGame-v3.6.0-windows-x64.exe
```

Run from a writable directory alongside the `content/` folder.
Saves are written to `savegame.dat` in the current working directory.

The game needs an 80-column terminal. Use `NO_COLOR=1` to disable colors,
or set `TERM=dumb` for plain text output. Set `RPG_CONTENT_DIR=/path/to/content`
to override the default content directory location.

## Verify

```sh
sha256sum -c SHA256SUMS
```

On Windows:

```powershell
Get-FileHash .\RPGGame-v3.6.0-windows-x64.exe -Algorithm SHA256
```

## Build from source

See `README.md` in the repository root for full build instructions.
The Linux build links libgcc and libstdc++ statically.
The Windows build uses LLVM-MinGW targeting x64 and UCRT.
