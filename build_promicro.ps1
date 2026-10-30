$ErrorActionPreference = "Stop"

$projectDir = $PSScriptRoot
$buildDir = "$projectDir\build_promicro"
$python = "C:\ncs\toolchains\4f5b6ad6dd\opt\bin\python.exe"

$env:PYTHONPATH = "$projectDir\tools;C:\ncs\toolchains\4f5b6ad6dd\opt\bin\Lib\site-packages"
$env:PATH = "C:\ncs\toolchains\4f5b6ad6dd\bin;C:\ncs\toolchains\4f5b6ad6dd\opt\bin;C:\ncs\toolchains\4f5b6ad6dd\opt\bin\Scripts;C:\ncs\toolchains\4f5b6ad6dd\opt\zephyr-sdk\gnu\arm-zephyr-eabi\bin;" + $env:PATH
$env:ZEPHYR_BASE = "C:\ncs\v3.4.1\zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR = "C:\ncs\toolchains\4f5b6ad6dd\opt\zephyr-sdk"

Write-Host ">>> BUILDING NRF52840_IBEACON FOR PRO MICRO NRF52840 (UF2) <<<" -ForegroundColor Cyan

& $python -m west build -b promicro_nrf52840/nrf52840/uf2 -s $projectDir -d $buildDir

if ($LASTEXITCODE -eq 0) {
    Write-Host ">>> BUILD PRO MICRO BERHASIL! <<<" -ForegroundColor Green
} else {
    Write-Host ">>> BUILD GAGAL! <<<" -ForegroundColor Red
    exit $LASTEXITCODE
}
