$ErrorActionPreference = "Stop"

$projectDir = $PSScriptRoot
$buildDir = "$projectDir\build_xiao"
$west = "C:\ncs\toolchains\4f5b6ad6dd\opt\bin\Scripts\west.exe"

$env:PYTHONPATH = "C:\ncs\toolchains\4f5b6ad6dd\opt\bin;C:\ncs\toolchains\4f5b6ad6dd\opt\bin\Lib;C:\ncs\toolchains\4f5b6ad6dd\opt\bin\Lib\site-packages"
$env:PATH = "C:\ncs\toolchains\4f5b6ad6dd\bin;C:\ncs\toolchains\4f5b6ad6dd\opt\bin;C:\ncs\toolchains\4f5b6ad6dd\opt\bin\Scripts;C:\ncs\toolchains\4f5b6ad6dd\opt\zephyr-sdk\gnu\arm-zephyr-eabi\bin;" + $env:PATH
$env:ZEPHYR_BASE = "C:\ncs\v3.4.1\zephyr"

Write-Host ">>> BUILDING NRF52840_IBEACON FOR XIAO_BLE/NRF52840 <<<" -ForegroundColor Cyan

& $west build -p auto -b xiao_ble/nrf52840 -s $projectDir -d $buildDir

if ($LASTEXITCODE -eq 0) {
    Write-Host ">>> BUILD XIAO BLE BERHASIL! <<<" -ForegroundColor Green
} else {
    Write-Host ">>> BUILD GAGAL! <<<" -ForegroundColor Red
    exit $LASTEXITCODE
}
