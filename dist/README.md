# RPGGame v3.4.2 prebuilt binaries

Standalone console builds. No installer, runtime, or DLL files are required.

| Platform | File |
| --- | --- |
| Linux x86_64 | `RPGGame-v3.4.2-linux-x86_64` |
| Windows 10/11 x64 | `RPGGame-v3.4.2-windows-x64.exe` |

## Run

Linux:

```sh
chmod +x RPGGame-v3.4.2-linux-x86_64
./RPGGame-v3.4.2-linux-x86_64
```

Windows (PowerShell or cmd):

```powershell
.\RPGGame-v3.4.2-windows-x64.exe
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
Get-FileHash .\RPGGame-v3.4.2-windows-x64.exe -Algorithm SHA256
```

## Build from source

See `README.md` in the repository root for build instructions.
The Linux build links libgcc and libstdc++ statically; no runtime required.
The Windows build is a fully static MinGW-w64 x64 console executable.
