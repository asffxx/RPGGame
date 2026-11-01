# RPGGame v3.2.1 prebuilt binaries

Standalone console builds. No installer, runtime, or DLL files are required.

| Platform | File |
| --- | --- |
| Windows 10/11 x64 | `RPGGame-v3.2.1-windows-x64.exe` |
| Linux x86_64 | `RPGGame-v3.2.1-linux-x86_64` |

## Run

Windows (PowerShell or cmd):

```powershell
.\RPGGame-v3.2.1-windows-x64.exe
```

Linux:

```sh
chmod +x RPGGame-v3.2.1-linux-x86_64
./RPGGame-v3.2.1-linux-x86_64
```

Run from a writable directory. Saves are written to `savegame.dat` in the
current working directory, not next to the executable.

The game needs an 80-column terminal. It uses plain text when output is
redirected or `TERM=dumb`; set `NO_COLOR=1` to disable colors in an interactive
terminal.

## Verify

```sh
sha256sum -c SHA256SUMS
```

On Windows:

```powershell
Get-FileHash .\RPGGame-v3.2.1-windows-x64.exe -Algorithm SHA256
```

## Build from source

See `README.md` in the repository root. The Linux build is statically linked.
The Windows build is a static MinGW-w64 x64 console executable.
