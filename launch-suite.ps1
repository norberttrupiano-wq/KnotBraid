param(
    [ValidateSet("All", "KnotBraid", "KnotBraidLauncher", "LogiKnotting", "LogiBraiding")]
    [string]$App = "All",

    [string]$Configuration = "Release",
    [string]$QtPrefix = "",

    [switch]$BuildIfMissing,
    [switch]$Rebuild,
    [switch]$NoLaunch
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-QtPrefix {
    param(
        [string]$RequestedPrefix
    )

    if (-not [string]::IsNullOrWhiteSpace($RequestedPrefix)) {
        return $RequestedPrefix.Trim()
    }

    if (-not [string]::IsNullOrWhiteSpace($env:KNOTBRAID_QT_PREFIX)) {
        return $env:KNOTBRAID_QT_PREFIX.Trim()
    }

    $qtRoot = "C:/Qt"
    if (Test-Path -LiteralPath $qtRoot) {
        $installedVersions = Get-ChildItem -LiteralPath $qtRoot -Directory -ErrorAction SilentlyContinue |
            ForEach-Object {
                try {
                    $version = [version]$_.Name
                }
                catch {
                    return
                }

                if ($version.Major -lt 6) {
                    return
                }

                $candidate = Join-Path $_.FullName "msvc2022_64"
                if (-not (Test-Path -LiteralPath $candidate)) {
                    return
                }

                [PSCustomObject]@{
                    Version = $version
                    Path = $candidate
                }
            } |
            Sort-Object Version -Descending

        if ($installedVersions) {
            return ($installedVersions | Select-Object -First 1).Path
        }
    }

    return "C:/Qt/6.10.2/msvc2022_64"
}

function Get-WindeployQtPath {
    $candidate = Join-Path $QtPrefix "bin/windeployqt.exe"
    if (Test-Path -LiteralPath $candidate) {
        return $candidate
    }

    return $null
}

function Test-QtRuntimeDeploymentNeeded {
    param(
        [string]$ExePath
    )

    $referenceDir = Join-Path $QtPrefix "bin"
    $exeDir = Split-Path -Parent $ExePath
    $runtimeDlls = @("Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll")

    foreach ($dllName in $runtimeDlls) {
        $referencePath = Join-Path $referenceDir $dllName
        if (-not (Test-Path -LiteralPath $referencePath)) {
            continue
        }

        $deployedPath = Join-Path $exeDir $dllName
        if (-not (Test-Path -LiteralPath $deployedPath)) {
            return $true
        }

        $referenceVersion = (Get-Item -LiteralPath $referencePath).VersionInfo.FileVersion
        $deployedVersion = (Get-Item -LiteralPath $deployedPath).VersionInfo.FileVersion
        if ($referenceVersion -ne $deployedVersion) {
            return $true
        }
    }

    return $false
}

function Invoke-QtDeploy {
    param(
        [string]$ExePath,
        [string]$AppName
    )

    $windeployqt = Get-WindeployQtPath
    if (-not $windeployqt) {
        return
    }

    Write-Host "[deploy] Updating Qt runtime for $AppName..."
    & $windeployqt --release --force --compiler-runtime $ExePath
    if ($LASTEXITCODE -ne 0) {
        throw "windeployqt failed for $AppName."
    }
}

$QtPrefix = Resolve-QtPrefix -RequestedPrefix $QtPrefix

Write-Host "[qt] Using Qt prefix: $QtPrefix"

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
        Name = "KnotBraid"
        ProjectDir = "KnotBraidLauncher"
        ExeName = "KnotBraid.exe"
        TargetName = "KnotBraid"
    },
    @{
        Name = "KnotBraidLauncher"
        ProjectDir = "KnotBraidLauncher"
        ExeName = "KnotBraidLauncher.exe"
        TargetName = "KnotBraidLauncher"
    },
    @{
        Name = "LogiKnotting"
        ProjectDir = "LogiKnotting"
        ExeName = "LogiKnotting.exe"
        TargetName = "LogiKnotting"
    },
    @{
        Name = "LogiBraiding"
        ProjectDir = "LogiBraiding"
        ExeName = "LogiBraiding.exe"
        TargetName = "LogiBraiding"
    }
)

if ($App -eq "All") {
    # Keep legacy behavior: All launches the two existing apps.
    $apps = $apps | Where-Object { $_.Name -notin @("KnotBraid", "KnotBraidLauncher") }
}
else {
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
    $targetName = $AppItem.TargetName
    if ([string]::IsNullOrWhiteSpace($targetName)) {
        $targetName = [System.IO.Path]::GetFileNameWithoutExtension($AppItem.ExeName)
    }

    & cmake --build $buildPath --config $Configuration --target $targetName
    if ($LASTEXITCODE -ne 0) {
        throw "CMake build failed for $($AppItem.Name)."
    }

    $exePath = Join-Path $buildPath "$Configuration/$($AppItem.ExeName)"
    if (-not (Test-Path -LiteralPath $exePath)) {
        throw "Executable not found after build: $exePath"
    }

    Invoke-QtDeploy -ExePath $exePath -AppName $AppItem.Name
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

    if (Test-QtRuntimeDeploymentNeeded -ExePath $exePath) {
        Invoke-QtDeploy -ExePath $exePath -AppName $item.Name
    }

    if ($NoLaunch) {
        Write-Host "[ok] Ready: $exePath"
        continue
    }

    Write-Host "[run] Starting $($item.Name)..."
    Start-Process -FilePath $exePath -WorkingDirectory (Split-Path -Parent $exePath)
}

Write-Host "Done."
