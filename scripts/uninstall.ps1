[CmdletBinding()]
param(
    [string]$InstallDirectory = (
        Join-Path (
            [Environment]::GetFolderPath("LocalApplicationData")
        ) "Programs\FScope"
    ),
    [switch]$SkipPathUpdate
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

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

$executablePath = Join-Path $resolvedInstallDirectory "fscope.exe"
if (Test-Path -LiteralPath $executablePath) {
    Remove-Item -LiteralPath $executablePath -Force
}

if (!$SkipPathUpdate) {
    $userPath = [Environment]::GetEnvironmentVariable(
        "Path",
        [EnvironmentVariableTarget]::User
    )

    $updatedEntries = @(
        ($userPath -split ";") |
            Where-Object {
                ![string]::IsNullOrWhiteSpace($_) -and
                (Get-NormalizedPath $_) -ine $resolvedInstallDirectory
            }
    )
    $updatedPath = $updatedEntries -join ";"

    if ($updatedPath -ne $userPath) {
        [Environment]::SetEnvironmentVariable(
            "Path",
            $updatedPath,
            [EnvironmentVariableTarget]::User
        )
    }
}

if (
    (Test-Path -LiteralPath $resolvedInstallDirectory) -and
    !(Get-ChildItem -LiteralPath $resolvedInstallDirectory -Force)
) {
    Remove-Item -LiteralPath $resolvedInstallDirectory -Force
}

Write-Host "FScope was removed."
if (!$SkipPathUpdate) {
    Write-Host "Open a new terminal to refresh PATH."
}
