[CmdletBinding()]
param([string]$BuildDirectory = '.codex-temp/unicode-path-tests')
$ErrorActionPreference = 'Stop'
$executable = (Resolve-Path (Join-Path $BuildDirectory 'Release/UnicodePathTests.exe')).Path
$root = Join-Path ([System.IO.Path]::GetFullPath($BuildDirectory)) ('runs/' + [guid]::NewGuid().ToString('N'))
$unicodeName = ([string][char]0x65e5) + [char]0x672c + [char]0x8a9e + '-' + [char]0x00e9 + '-' + [char]::ConvertFromUtf32(0x1f43e)
$unicodeDirectory = Join-Path $root $unicodeName
New-Item -ItemType Directory -Path $unicodeDirectory -Force | Out-Null
Copy-Item -LiteralPath $executable -Destination $unicodeDirectory
& (Join-Path $unicodeDirectory 'UnicodePathTests.exe')
if ($LASTEXITCODE -ne 0) { throw 'Unicode-path regression failed.' }

# Extended path syntax allows testing a path longer than MAX_PATH regardless
# of the host's long-path policy. Each individual directory stays below 255.
$longDirectory = $unicodeDirectory
while ($longDirectory.Length -lt 300) { $longDirectory = Join-Path $longDirectory ('nested-' + ('x' * 40)) }
$longDirectory = '\\?\' + $longDirectory.Replace('/', '\')
[System.IO.Directory]::CreateDirectory($longDirectory) | Out-Null
$longExecutable = Join-Path $longDirectory 'UnicodePathTests.exe'
[System.IO.File]::Copy($executable, $longExecutable)
& $executable --launch-long $longExecutable
if ($LASTEXITCODE -ne 0) { throw 'Long Unicode-path regression failed.' }
