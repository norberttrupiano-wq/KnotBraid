param(
    [ValidateSet("All", "LogiKnotting", "LogiBraiding")]
    [string]$App = "All",

    [string]$Configuration = "Release",
    [string]$QtPrefix = "C:/Qt/6.10.1/msvc2022_64",

    [switch]$BuildIfMissing,
    [switch]$Rebuild,
    [switch]$NoLaunch
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$qtBin = Join-Path $QtPrefix "bin"

if (Test-Path -LiteralPath $qtBin) {
    $env:Path = "$qtBin;$env:Path"
}
else {
    Write-Warning "Qt bin folder not found: $qtBin"
}

$apps = @(
    @{
        Name = "LogiKnotting"
        ProjectDir = "LogiKnotting"
        ExeName = "LogiKnotting.exe"
    },
    @{
        Name = "LogiBraiding"
        ProjectDir = "LogiBraiding"
        ExeName = "LogiBraiding.exe"
    }
)

if ($App -ne "All") {
    $apps = $apps | Where-Object { $_.Name -eq $App }
}

function Invoke-CMakeBuild {
    param(
        [hashtable]$AppItem
    )

    $projectPath = Join-Path $repoRoot $AppItem.ProjectDir
    $buildPath = Join-Path $projectPath "build"

    Write-Host "[build] Configuring $($AppItem.Name)..."
    & cmake -S $projectPath -B $buildPath "-DCMAKE_PREFIX_PATH=$QtPrefix"
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configure failed for $($AppItem.Name)."
    }

    Write-Host "[build] Building $($AppItem.Name) ($Configuration)..."
    & cmake --build $buildPath --config $Configuration
    if ($LASTEXITCODE -ne 0) {
        throw "CMake build failed for $($AppItem.Name)."
    }
}

foreach ($item in $apps) {
    $projectPath = Join-Path $repoRoot $item.ProjectDir
    $buildPath = Join-Path $projectPath "build"
    $exePath = Join-Path $buildPath "$Configuration/$($item.ExeName)"

    $needsBuild = $Rebuild -or ($BuildIfMissing -and -not (Test-Path -LiteralPath $exePath))
    if ($needsBuild) {
        Invoke-CMakeBuild -AppItem $item
    }

    if (-not (Test-Path -LiteralPath $exePath)) {
        throw "Executable not found: $exePath`nUse -BuildIfMissing to build automatically."
    }

    if ($NoLaunch) {
        Write-Host "[ok] Ready: $exePath"
        continue
    }

    Write-Host "[run] Starting $($item.Name)..."
    Start-Process -FilePath $exePath -WorkingDirectory (Split-Path -Parent $exePath)
}

Write-Host "Done."
