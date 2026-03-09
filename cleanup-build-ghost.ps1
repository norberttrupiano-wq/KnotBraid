param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectPath,

    [string]$BuildDir = "build",
    [switch]$Reconfigure,
    [switch]$Build,
    [string]$QtPrefix = "C:/Qt/6.10.1/msvc2022_64"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ($Build -and -not $Reconfigure) {
    $Reconfigure = $true
}

$resolvedProject = (Resolve-Path -LiteralPath $ProjectPath).Path
$buildPath = Join-Path $resolvedProject $BuildDir

Write-Host "Project  : $resolvedProject"
Write-Host "Build dir: $buildPath"

if (Test-Path -LiteralPath $buildPath) {
    # Detect each .qtc/package-manager folder under build and remove malformed "."
    $packageDirs = Get-ChildItem -LiteralPath $buildPath -Directory -Recurse -Force -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -eq "package-manager" -and $_.FullName -like "*\.qtc\package-manager" }

    foreach ($pkg in $packageDirs) {
        $rawBase = "\\?\$($pkg.FullName)"
        $ghostDot = "$rawBase\."

        try {
            if (Test-Path -LiteralPath $ghostDot) {
                Remove-Item -LiteralPath $ghostDot -Recurse -Force
                Write-Host "Removed ghost entry: $ghostDot"
            }
        }
        catch {
            Write-Warning "Could not remove $ghostDot : $($_.Exception.Message)"
        }
    }

    # Use cmd rmdir for stubborn build trees.
    cmd /c "rmdir /s /q `"$buildPath`""
    if (Test-Path -LiteralPath $buildPath) {
        throw "Build folder still exists after cleanup: $buildPath"
    }

    Write-Host "Removed build folder: $buildPath"
}
else {
    Write-Host "No build folder found, nothing to remove."
}

if ($Reconfigure) {
    Push-Location $resolvedProject
    try {
        Write-Host "Running: cmake -S . -B $BuildDir -DCMAKE_PREFIX_PATH=$QtPrefix"
        & cmake -S . -B $BuildDir "-DCMAKE_PREFIX_PATH=$QtPrefix"
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configure failed with exit code $LASTEXITCODE."
        }

        if ($Build) {
            Write-Host "Running: cmake --build $BuildDir --config Release"
            & cmake --build $BuildDir --config Release
            if ($LASTEXITCODE -ne 0) {
                throw "CMake build failed with exit code $LASTEXITCODE."
            }
        }
    }
    finally {
        Pop-Location
    }
}

Write-Host "Done."
