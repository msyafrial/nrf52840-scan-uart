$ErrorActionPreference = "Stop"

$projectDir = $PSScriptRoot
$openocd = "C:\Users\syafr\.platformio\packages\tool-openocd\bin\openocd.exe"
$scriptsDir = "C:\Users\syafr\.platformio\packages\tool-openocd\openocd\scripts"

$hexCandidates = @(
    "$projectDir\build\nrf52840_ibeacon\zephyr\zephyr.hex",
    "$projectDir\build\zephyr\zephyr.hex"
)

$targetHex = $null
foreach ($path in $hexCandidates) {
    if (Test-Path $path) {
        $targetHex = $path
        break;
    }
}

if (-not $targetHex) {
    Write-Error "File hex tidak ditemukan di folder build! Pastikan sudah menjalankan build.ps1 terlebih dahulu."
    exit 1
}

$hexForwardSlash = $targetHex.Replace('\', '/')
Write-Host ">>> Flashing hex: $hexForwardSlash" -ForegroundColor Cyan

& $openocd -s $scriptsDir -f interface/stlink.cfg -f target/nrf52.cfg -c "adapter speed 500; init; halt; program {$hexForwardSlash} verify; reset run; exit"

if ($LASTEXITCODE -eq 0) {
    Write-Host ">>> FLASHING IBEACON SELESAI & TARGET BERJALAN! <<<" -ForegroundColor Green
} else {
    Write-Host ">>> FLASHING GAGAL! <<<" -ForegroundColor Red
    exit $LASTEXITCODE
}
