Add-Type -AssemblyName System.Drawing
Get-ChildItem -Filter *.bmp | ForEach-Object {
    $bmp = New-Object System.Drawing.Bitmap($_.FullName)
    $pngName = $_.BaseName + ".png"
    $pngPath = Join-Path $_.DirectoryName $pngName
    $bmp.Save($pngPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    Write-Host "Converted $($_.Name) -> $pngName"
}
