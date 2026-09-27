[CmdletBinding()]
param(
    [string]$InstalledRoot = 'C:/vcpkg/installed',
    [string]$Triplet = 'x64-windows-static',
    [switch]$Check
)

$ErrorActionPreference = 'Stop'
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$outputPath = Join-Path $repoRoot 'THIRD_PARTY_NOTICES.txt'
$status = Get-Content -LiteralPath (Join-Path $InstalledRoot 'vcpkg/status') -Raw
$paragraphs = ($status -replace "`r`n", "`n") -split "`n`n+"

# Runtime and header dependencies of both game renderers, including glog's
# gflags dependency and the Khronos platform header used by glad.
# Assimp/demo libraries, validation layers and build tools are not packaged.
$packages = @('glfw3', 'glm', 'glog', 'gflags', 'nlohmann-json', 'stb',
    'miniaudio', 'glad', 'egl-registry', 'vulkan-headers', 'vulkan-loader')
$sections = [System.Collections.Generic.List[string]]::new()
$sections.Add(@'
Sniff the Way - Third-party notices

The components below retain their respective copyrights and license terms.
The proprietary game license does not replace or restrict these terms.
This file covers the combined Windows Vulkan/OpenGL distribution.
The Vulkan loader section includes the full Apache License 2.0 referenced
by the Khronos components. Alice's font license is also distributed at
resources/fonts/OFL.txt.

Generated from the installed dependency notices by
buildtools/Update-ThirdPartyNotices.ps1. Regenerate and review this file
whenever dependencies change. Dependency versions include vcpkg port revisions.
'@)

foreach ($package in $packages) {
    $records = @($paragraphs | Where-Object {
        $_ -match "(?m)^Package: $([regex]::Escape($package))$" -and
        $_ -match "(?m)^Architecture: $([regex]::Escape($Triplet))$" -and
        $_ -match '(?m)^Status: install ok installed$' -and
        $_ -notmatch '(?m)^Feature:'
    })
    if ($records.Count -ne 1) { throw "Expected one installed record for ${package}:${Triplet}." }
    if ($records[0] -notmatch '(?m)^Version: (.+)$') { throw "Missing version for $package." }
    $version = $Matches[1]
    if ($records[0] -match '(?m)^Port-Version: (.+)$') { $version += '#' + $Matches[1] }
    $noticePath = Join-Path $InstalledRoot "$Triplet/share/$package/copyright"
    $notice = Get-Content -LiteralPath $noticePath -Raw
    if ([string]::IsNullOrWhiteSpace($notice)) { throw "Empty notice: $noticePath" }
    $sections.Add("$package $version`nSource: share/$package/copyright`n`n$($notice.Trim())")
}

$headerNotices = @{
    'vulkan/vulkan.hpp' = '(?s)\A// Copyright.*?(?=\r?\n\r?\n)'
    'vulkan/vulkan_core.h' = '(?s)/\*\s*\*\* Copyright.*?\*/'
    'KHR/khrplatform.h' = '(?s)/\*\s*\*\* Copyright.*?\*/'
}
foreach ($header in ($headerNotices.Keys | Sort-Object)) {
    $source = Get-Content -LiteralPath (Join-Path $InstalledRoot "$Triplet/include/$header") -Raw
    if ($source -notmatch $headerNotices[$header]) {
        throw "Cannot locate the copyright header in $header; review its updated license."
    }
    $sections.Add("Header notice: $header`nSource: include/$header`n`n$($Matches[0])")
}

$fontNotice = Get-Content -LiteralPath (Join-Path $repoRoot 'resources/fonts/OFL.txt') -Raw
if ([string]::IsNullOrWhiteSpace($fontNotice)) { throw 'Missing Alice font notice.' }
$sections.Add("Alice font`nSource: resources/fonts/OFL.txt`n`n$($fontNotice.Trim())")
$text = (($sections -join "`n`n========================================================================`n`n") -replace "`r`n", "`n") + "`n"
if ($Check) {
    $existing = Get-Content -LiteralPath $outputPath -Raw
    if (($existing -replace "`r`n", "`n") -cne $text) {
        throw 'Third-party notices are stale. Run buildtools/Update-ThirdPartyNotices.ps1 with the build dependency root/triplet, review, and commit the result.'
    }
    Write-Host 'Third-party notices match installed dependencies.'
}
else {
    [System.IO.File]::WriteAllText($outputPath, $text, [System.Text.UTF8Encoding]::new($false))
    Write-Host "Updated $outputPath"
}
