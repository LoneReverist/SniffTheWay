## Sniff the Way — A Tail to Guide You Home
A short story about a lost duo trying to find their way home.

![](cover.png)

## Versioning

The game version is defined by the top-level CMake `project(... VERSION ...)` declaration. Official releases use matching Git tags such as `v0.1.0`.

## Packaging a Windows release

Run the packaging script from PowerShell:

```powershell
.\buildtools\Package-Release.ps1
```

It builds isolated Vulkan and OpenGL Release configurations, combines both executables with their shared resources, and writes a versioned ZIP and SHA-256 checksum under `dist/`.

Use `-Official` for a release build. Official packaging requires a clean working tree and a Git tag matching the CMake version.
