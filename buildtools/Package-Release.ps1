[CmdletBinding()]
param(
	[switch]$Official,
	[switch]$CleanBuilds,
	[string]$OutputDirectory
)

$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$packageBuildRoot = Join-Path $repoRoot "build-package"
$vulkanBuildRoot = Join-Path $packageBuildRoot "vulkan"
$openGLBuildRoot = Join-Path $packageBuildRoot "opengl"

if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
	$distRoot = Join-Path $repoRoot "dist"
}
else {
	$distRoot = [System.IO.Path]::GetFullPath($OutputDirectory)
}

function Invoke-CheckedCommand {
	param(
		[Parameter(Mandatory = $true)][string]$Command,
		[Parameter(Mandatory = $true)][string[]]$Arguments
	)

	& $Command @Arguments
	if ($LASTEXITCODE -ne 0) {
		throw "Command failed with exit code ${LASTEXITCODE}: $Command $($Arguments -join ' ')"
	}
}

function Invoke-BuildPreset {
	param([Parameter(Mandatory = $true)][string]$Preset)

	try {
		Invoke-CheckedCommand -Command "cmake" -Arguments @("--build", "--preset", $Preset)
	}
	catch {
		# Fresh Visual Studio C++ module scans occasionally leave one incomplete dependency
		# file while still generating all other module metadata. One incremental retry lets
		# MSBuild rescan that file and has proven reliable for clean package build trees.
		Write-Warning "Build preset '$Preset' failed; retrying once to recover its module dependency scan."
		Invoke-CheckedCommand -Command "cmake" -Arguments @("--build", "--preset", $Preset)
	}
}

function Remove-DirectoryWithin {
	param(
		[Parameter(Mandatory = $true)][string]$Target,
		[Parameter(Mandatory = $true)][string]$AllowedParent
	)

	$targetPath = [System.IO.Path]::GetFullPath($Target)
	$parentPath = [System.IO.Path]::GetFullPath($AllowedParent).TrimEnd(
		[System.IO.Path]::DirectorySeparatorChar,
		[System.IO.Path]::AltDirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar

	if (!$targetPath.StartsWith($parentPath, [System.StringComparison]::OrdinalIgnoreCase)) {
		throw "Refusing to remove directory outside '$AllowedParent': $targetPath"
	}
	if (Test-Path -LiteralPath $targetPath) {
		Remove-Item -LiteralPath $targetPath -Recurse -Force
	}
}

function Copy-DirectoryContents {
	param(
		[Parameter(Mandatory = $true)][string]$Source,
		[Parameter(Mandatory = $true)][string]$Destination
	)

	if (!(Test-Path -LiteralPath $Source -PathType Container)) {
		throw "Required directory does not exist: $Source"
	}
	New-Item -ItemType Directory -Path $Destination -Force | Out-Null
	Get-ChildItem -LiteralPath $Source -Force | ForEach-Object {
		Copy-Item -LiteralPath $_.FullName -Destination $Destination -Recurse -Force
	}
}

function Get-RequiredFile {
	param([Parameter(Mandatory = $true)][string]$Path)

	if (!(Test-Path -LiteralPath $Path -PathType Leaf)) {
		throw "Required file does not exist: $Path"
	}
	$file = Get-Item -LiteralPath $Path
	if ($file.Length -le 0) {
		throw "Required file is empty: $Path"
	}
	return $file
}

if ($CleanBuilds) {
	Remove-DirectoryWithin -Target $vulkanBuildRoot -AllowedParent $packageBuildRoot
	Remove-DirectoryWithin -Target $openGLBuildRoot -AllowedParent $packageBuildRoot
}

Push-Location $repoRoot
try {
	Invoke-CheckedCommand -Command "cmake" -Arguments @("--preset", "package-windows-vulkan")
	Invoke-CheckedCommand -Command "cmake" -Arguments @("--preset", "package-windows-opengl")
}
finally {
	Pop-Location
}

$vulkanVersionPath = Join-Path $vulkanBuildRoot "SniffTheWayVersion.txt"
$openGLVersionPath = Join-Path $openGLBuildRoot "SniffTheWayVersion.txt"
$version = (Get-Content -LiteralPath $vulkanVersionPath -Raw).Trim()
$openGLVersion = (Get-Content -LiteralPath $openGLVersionPath -Raw).Trim()
if ([string]::IsNullOrWhiteSpace($version)) {
	throw "The generated package version is empty."
}
if ($version -ne $openGLVersion) {
	throw "Renderer build versions do not match: Vulkan '$version', OpenGL '$openGLVersion'."
}
if ($version -notmatch '^\d+\.\d+\.\d+$') {
	throw "Package version is not a numeric semantic version: $version"
}

$commit = (& git -C $repoRoot rev-parse --short=7 HEAD).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($commit)) {
	throw "Failed to determine the current Git commit."
}
$commitTimestampText = (& git -C $repoRoot show -s --format=%ct HEAD).Trim()
$commitTimestampSeconds = 0L
if ($LASTEXITCODE -ne 0 -or ![long]::TryParse($commitTimestampText, [ref]$commitTimestampSeconds)) {
	throw "Failed to determine the current Git commit timestamp."
}
$packageTimestamp = [System.DateTimeOffset]::FromUnixTimeSeconds($commitTimestampSeconds).UtcDateTime
$workingTreeStatus = (& git -C $repoRoot status --porcelain --ignore-submodules=none)
if ($LASTEXITCODE -ne 0) {
	throw "Failed to inspect the Git working tree."
}
$workingTreeIsDirty = [bool]$workingTreeStatus

if ($Official) {
	if ($workingTreeIsDirty) {
		throw "Official packages require a clean Git working tree."
	}

	$currentTag = (& git -C $repoRoot describe --tags --exact-match 2>$null)
	if ($LASTEXITCODE -ne 0 -or $currentTag.Trim() -ne "v$version") {
		throw "Official package version $version requires the current commit to be tagged v$version."
	}
	$packageVersion = $version
}
else {
	$dirtySuffix = if ($workingTreeIsDirty) { "-dirty" } else { "" }
	$packageVersion = "$version-dev-$commit$dirtySuffix"
}

$packageName = "SniffTheWay-$packageVersion-windows-x64"
$packageRoot = Join-Path $distRoot $packageName
$zipPath = Join-Path $distRoot "$packageName.zip"
$checksumPath = "$zipPath.sha256"

if ($Official -and (Test-Path -LiteralPath $zipPath)) {
	throw "Official package already exists: $zipPath"
}

Push-Location $repoRoot
try {
	Invoke-BuildPreset -Preset "package-windows-vulkan-release"
	Invoke-BuildPreset -Preset "package-windows-opengl-release"
}
finally {
	Pop-Location
}

New-Item -ItemType Directory -Path $distRoot -Force | Out-Null
Remove-DirectoryWithin -Target $packageRoot -AllowedParent $distRoot
if (Test-Path -LiteralPath $zipPath) {
	Remove-Item -LiteralPath $zipPath -Force
}
if (Test-Path -LiteralPath $checksumPath) {
	Remove-Item -LiteralPath $checksumPath -Force
}
New-Item -ItemType Directory -Path $packageRoot -Force | Out-Null

$vulkanExecutable = Get-RequiredFile (Join-Path $vulkanBuildRoot "Release/SniffTheWay.exe")
$openGLExecutable = Get-RequiredFile (Join-Path $openGLBuildRoot "Release/SniffTheWay.exe")
Copy-Item -LiteralPath $vulkanExecutable.FullName -Destination (Join-Path $packageRoot "SniffTheWay-Vulkan.exe")
Copy-Item -LiteralPath $openGLExecutable.FullName -Destination (Join-Path $packageRoot "SniffTheWay-OpenGL.exe")

$stagedResources = Join-Path $packageRoot "resources"
Copy-DirectoryContents -Source (Join-Path $repoRoot "resources") -Destination $stagedResources

$stagedShaders = Join-Path $stagedResources "shaders"
Copy-DirectoryContents -Source (Join-Path $repoRoot "src/shaders") -Destination $stagedShaders

$compiledShaderRoot = Join-Path $vulkanBuildRoot "shaders"
Get-ChildItem -LiteralPath $compiledShaderRoot -File -Filter "*.spv" | ForEach-Object {
	Copy-Item -LiteralPath $_.FullName -Destination $stagedShaders -Force
}

$sourceShaders = Get-ChildItem -LiteralPath (Join-Path $repoRoot "src/shaders") -File
foreach ($shader in $sourceShaders) {
	Get-RequiredFile (Join-Path $stagedShaders $shader.Name) | Out-Null
	Get-RequiredFile (Join-Path $stagedShaders "$($shader.Name).spv") | Out-Null
}

foreach ($requiredResourceDirectory in @("fonts", "gameplay", "music", "sfx", "story", "textures")) {
	$requiredPath = Join-Path $stagedResources $requiredResourceDirectory
	if (!(Test-Path -LiteralPath $requiredPath -PathType Container)) {
		throw "Required resource directory is missing from the package: $requiredResourceDirectory"
	}
}

$utf8WithoutBom = [System.Text.UTF8Encoding]::new($false)
[System.IO.File]::WriteAllText((Join-Path $packageRoot "VERSION.txt"), "$packageVersion`r`n", $utf8WithoutBom)
Copy-Item -LiteralPath (Join-Path $PSScriptRoot "Package-README.txt") -Destination (Join-Path $packageRoot "README.txt")

# Normalize staged timestamps so identical inputs produce stable ZIP metadata.
Get-ChildItem -LiteralPath $packageRoot -Recurse -Force | ForEach-Object {
	$_.LastWriteTimeUtc = $packageTimestamp
}
(Get-Item -LiteralPath $packageRoot).LastWriteTimeUtc = $packageTimestamp

Compress-Archive -LiteralPath $packageRoot -DestinationPath $zipPath -CompressionLevel Optimal
Get-RequiredFile $zipPath | Out-Null

Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
try {
	$entryNames = $archive.Entries | ForEach-Object { $_.FullName.Replace('\', '/') }
	foreach ($requiredEntry in @(
		"$packageName/SniffTheWay-Vulkan.exe",
		"$packageName/SniffTheWay-OpenGL.exe",
		"$packageName/VERSION.txt",
		"$packageName/README.txt")) {
		if ($entryNames -notcontains $requiredEntry) {
			throw "ZIP verification failed; missing entry: $requiredEntry"
		}
	}
}
finally {
	$archive.Dispose()
}

$zipHash = (Get-FileHash -LiteralPath $zipPath -Algorithm SHA256).Hash.ToLowerInvariant()
[System.IO.File]::WriteAllText(
	$checksumPath,
	"$zipHash  $([System.IO.Path]::GetFileName($zipPath))`r`n",
	$utf8WithoutBom)

Write-Host "Created release package: $zipPath"
Write-Host "SHA-256: $zipHash"
