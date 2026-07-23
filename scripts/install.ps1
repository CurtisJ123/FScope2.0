[CmdletBinding()]
param(
    [string]$InstallDirectory = (
        Join-Path (
            [Environment]::GetFolderPath("LocalApplicationData")
        ) "Programs\FScope"
    ),
    [string]$DownloadUrl = (
        "https://github.com/CurtisJ123/FScope2.0/" +
        "releases/latest/download/fscope.exe"
    ),
    [string]$ChecksumUrl = (
        "https://github.com/CurtisJ123/FScope2.0/" +
        "releases/latest/download/fscope.exe.sha256"
    ),
    [switch]$SkipPathUpdate
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ($env:OS -ne "Windows_NT") {
    throw "FScope currently supports Windows only."
}

if ([string]::IsNullOrWhiteSpace($InstallDirectory)) {
    throw "The install directory cannot be empty."
}

function Get-NormalizedPath {
    param([string]$Path)

    if ([string]::IsNullOrWhiteSpace($Path)) {
        return ""
    }

    $expandedPath = [Environment]::ExpandEnvironmentVariables(
        $Path.Trim().Trim('"')
    )

    try {
        return [System.IO.Path]::GetFullPath($expandedPath).TrimEnd(
            [System.IO.Path]::DirectorySeparatorChar,
            [System.IO.Path]::AltDirectorySeparatorChar
        )
    }
    catch {
        return $expandedPath.TrimEnd("\", "/")
    }
}

$resolvedInstallDirectory = [System.IO.Path]::GetFullPath(
    $InstallDirectory
).TrimEnd(
    [System.IO.Path]::DirectorySeparatorChar,
    [System.IO.Path]::AltDirectorySeparatorChar
)

New-Item -ItemType Directory -Force -Path $resolvedInstallDirectory |
    Out-Null

$executablePath = Join-Path $resolvedInstallDirectory "fscope.exe"
$temporarySuffix = "$PID.$([Guid]::NewGuid().ToString('N'))"
$temporaryPath = Join-Path (
    $resolvedInstallDirectory
) "fscope.$temporarySuffix.download"
$temporaryChecksumPath = Join-Path (
    $resolvedInstallDirectory
) "fscope.$temporarySuffix.sha256"
$backupPath = Join-Path (
    $resolvedInstallDirectory
) "fscope.$temporarySuffix.backup"
$installed = $false

try {
    Write-Host "Downloading FScope..."
    Invoke-WebRequest `
        -Uri $DownloadUrl `
        -OutFile $temporaryPath `
        -UseBasicParsing

    Invoke-WebRequest `
        -Uri $ChecksumUrl `
        -OutFile $temporaryChecksumPath `
        -UseBasicParsing

    $download = Get-Item -LiteralPath $temporaryPath
    if ($download.Length -lt 1024) {
        throw "The downloaded executable is unexpectedly small."
    }

    $stream = [System.IO.File]::OpenRead($temporaryPath)
    try {
        if ($stream.ReadByte() -ne 0x4D -or $stream.ReadByte() -ne 0x5A) {
            throw "The downloaded file is not a Windows executable."
        }
    }
    finally {
        $stream.Dispose()
    }

    $checksumText = (
        Get-Content -LiteralPath $temporaryChecksumPath -Raw
    ).Trim()
    $expectedHash = ($checksumText -split "\s+")[0]

    if ($expectedHash -notmatch "^[a-fA-F0-9]{64}$") {
        throw "The published checksum has an invalid format."
    }

    $actualHash = (
        Get-FileHash -LiteralPath $temporaryPath -Algorithm SHA256
    ).Hash

    if ($actualHash -ine $expectedHash) {
        throw "The downloaded executable failed SHA-256 verification."
    }

    if (Test-Path -LiteralPath $executablePath) {
        try {
            [System.IO.File]::Replace(
                $temporaryPath,
                $executablePath,
                $backupPath,
                $true
            )
        }
        catch {
            throw (
                "Could not update fscope.exe. Close any running FScope " +
                "processes and try again. $($_.Exception.Message)"
            )
        }
    }
    else {
        [System.IO.File]::Move($temporaryPath, $executablePath)
    }

    $installed = $true
}
finally {
    if (Test-Path -LiteralPath $temporaryPath) {
        Remove-Item -LiteralPath $temporaryPath -Force
    }
    if (Test-Path -LiteralPath $temporaryChecksumPath) {
        Remove-Item -LiteralPath $temporaryChecksumPath -Force
    }
    if ($installed -and (Test-Path -LiteralPath $backupPath)) {
        Remove-Item -LiteralPath $backupPath -Force
    }
}

if (!$SkipPathUpdate) {
    $userPath = [Environment]::GetEnvironmentVariable(
        "Path",
        [EnvironmentVariableTarget]::User
    )

    $pathEntries = @(
        ($userPath -split ";") |
            Where-Object { ![string]::IsNullOrWhiteSpace($_) }
    )

    $remainingPathEntries = @(
        $pathEntries |
            Where-Object {
                (Get-NormalizedPath $_) -ine $resolvedInstallDirectory
            }
    )
    $updatedPath = @(
        $resolvedInstallDirectory
        $remainingPathEntries
    ) -join ";"

    if ($updatedPath -ne $userPath) {
        [Environment]::SetEnvironmentVariable(
            "Path",
            $updatedPath,
            [EnvironmentVariableTarget]::User
        )
    }

    $currentPathEntries = @($env:Path -split ";")
    if (
        !(
            $currentPathEntries |
                Where-Object {
                    (Get-NormalizedPath $_) -ieq
                        $resolvedInstallDirectory
                }
        )
    ) {
        $env:Path = "$resolvedInstallDirectory;$env:Path"
    }

    $conflictingCommands = @(
        Get-Command fscope -All -ErrorAction SilentlyContinue |
            Where-Object {
                ![string]::IsNullOrWhiteSpace($_.Source) -and
                (Get-NormalizedPath $_.Source) -ine $executablePath
            }
    )

    if ($conflictingCommands.Count -gt 0) {
        Write-Warning (
            "Another fscope command is also on PATH. A new terminal should " +
            "prefer the newly installed executable if its PATH entry comes " +
            "first."
        )
    }
}

Write-Host ""
Write-Host "FScope was installed to:"
Write-Host "  $executablePath"

if ($SkipPathUpdate) {
    Write-Host "PATH was not changed because -SkipPathUpdate was used."
}
else {
    Write-Host "Open a new terminal and run: fscope"
}
