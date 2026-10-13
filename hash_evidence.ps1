$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$captureDir = Join-Path $repoRoot "captures"
$evidenceDir = Join-Path $repoRoot "evidence"
$outputFile = Join-Path $evidenceDir "capture-hashes.csv"

New-Item -ItemType Directory -Force -Path $evidenceDir | Out-Null

$files = Get-ChildItem -Path $captureDir -File -ErrorAction SilentlyContinue
if (-not $files) {
    Write-Host "No capture files found in $captureDir"
    Write-Host "Place PCAP/PCAPNG files there and run this script again."
    exit 0
}

$hashes = $files | Get-FileHash -Algorithm SHA256
$hashes |
    Select-Object Path, Algorithm, Hash |
    Export-Csv -Path $outputFile -NoTypeInformation -Encoding UTF8

Write-Host "SHA-256 manifest written to: $outputFile"
$hashes | Format-Table Path, Hash -AutoSize
