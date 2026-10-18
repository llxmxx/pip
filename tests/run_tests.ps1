Set-Location $PSScriptRoot
$failed = 0
foreach ($test in Get-ChildItem *.asm){
    $name = $test.BaseName
    ../asm $test.Name "$name.bin"
    $actual = (../vm "$name.bin") -join "`n"
    $expected = (Get-Content "$name.expected") -join "`n"
    if ($actual -eq $expected) { Write-Host "PASS $name"}
    else { Write-Host "FAIL $name"; $failed++}
}
Write-Host "$failed failed"