## Sniff the Way — A Tail to Guide You Home
A curious baby. A devoted dog. A long way home. Sniff the Way is a storybook adventure about two small companions lost in the woods, where a little courage and a loyal nose guide the journey back to family.

Play now on Itch.io: https://lonereverist.itch.io/sniff-the-way

![](cover.png)

## License

Copyright (c) 2026 Jonathan Kraber. All rights reserved.

Sniff the Way is proprietary software. Players may install and play lawfully obtained releases for personal, non-commercial use under [the game license](LICENSE.txt). The source code and original game assets are not licensed for reuse, modification, or redistribution. Separately licensed components remain under their own terms.

## Versioning

The game version is defined by the top-level CMake `project(... VERSION ...)` declaration. Official releases use matching Git tags such as `v1.0.0`.

## Packaging a Windows release

Run the packaging script from PowerShell:

```powershell
.\buildtools\Package-Release.ps1
```

It builds isolated Vulkan and OpenGL Release configurations, combines both executables with their shared resources, and writes a versioned ZIP and SHA-256 checksum under `dist/`.

Use `-Official` for a release build. Official packaging requires a clean working tree and a Git tag matching the CMake version.

The ZIP includes [third-party notices](THIRD_PARTY_NOTICES.txt) for both renderers
and the Alice font. Packaging checks these notices against the installed vcpkg
dependencies and rejects missing or stale notices.

After upgrading dependencies, run `./buildtools/Update-ThirdPartyNotices.ps1`,
review the updated notices, and commit them with the dependency change. Use
`-InstalledRoot` and `-Triplet` for a different vcpkg installation. When adding
or removing dependencies, update the package list in that script, including
transitive libraries. Demo-only dependencies and development tools are excluded
because they are not shipped. Art, music, and sound-effect provenance must be
reviewed separately; this generated inventory covers libraries and the font.
