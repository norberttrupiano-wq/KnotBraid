Add-Type -AssemblyName System.Drawing

$ErrorActionPreference = "Stop"

function New-RoundedRectPath {
    param(
        [float]$X,
        [float]$Y,
        [float]$Width,
        [float]$Height,
        [float]$Radius
    )

    $path = New-Object System.Drawing.Drawing2D.GraphicsPath
    $diameter = $Radius * 2

    $path.AddArc($X, $Y, $diameter, $diameter, 180, 90)
    $path.AddArc($X + $Width - $diameter, $Y, $diameter, $diameter, 270, 90)
    $path.AddArc($X + $Width - $diameter, $Y + $Height - $diameter, $diameter, $diameter, 0, 90)
    $path.AddArc($X, $Y + $Height - $diameter, $diameter, $diameter, 90, 90)
    $path.CloseFigure()

    return $path
}

function Write-IcoFile {
    param(
        [System.Drawing.Bitmap]$Bitmap,
        [string]$IcoPath
    )

    $pngStream = New-Object System.IO.MemoryStream
    $Bitmap.Save($pngStream, [System.Drawing.Imaging.ImageFormat]::Png)
    $pngBytes = $pngStream.ToArray()
    $pngStream.Dispose()

    $fileStream = [System.IO.File]::Open($IcoPath, [System.IO.FileMode]::Create)
    $writer = New-Object System.IO.BinaryWriter($fileStream)

    $writer.Write([UInt16]0)
    $writer.Write([UInt16]1)
    $writer.Write([UInt16]1)
    $writer.Write([byte]0)
    $writer.Write([byte]0)
    $writer.Write([byte]0)
    $writer.Write([byte]0)
    $writer.Write([UInt16]1)
    $writer.Write([UInt16]32)
    $writer.Write([UInt32]$pngBytes.Length)
    $writer.Write([UInt32]22)
    $writer.Write($pngBytes)

    $writer.Flush()
    $writer.Dispose()
    $fileStream.Dispose()
}

function Save-IconPair {
    param(
        [string]$PngPath,
        [string]$IcoPath,
        [scriptblock]$Painter
    )

    $size = 256
    $bitmap = New-Object System.Drawing.Bitmap($size, $size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)

    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
    $graphics.Clear([System.Drawing.Color]::Transparent)

    & $Painter $graphics $size

    $bitmap.Save($PngPath, [System.Drawing.Imaging.ImageFormat]::Png)
    Write-IcoFile -Bitmap $bitmap -IcoPath $IcoPath

    $graphics.Dispose()
    $bitmap.Dispose()
}

function Draw-Background {
    param(
        [System.Drawing.Graphics]$Graphics,
        [int]$Size,
        [System.Drawing.Color]$TopColor,
        [System.Drawing.Color]$BottomColor
    )

    $path = New-RoundedRectPath -X 12 -Y 12 -Width ($Size - 24) -Height ($Size - 24) -Radius 52
    $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
        (New-Object System.Drawing.Point(0, 0)),
        (New-Object System.Drawing.Point($Size, $Size)),
        $TopColor,
        $BottomColor
    )

    $Graphics.FillPath($brush, $path)

    $highlight = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(28, 255, 255, 255))
    $Graphics.FillEllipse($highlight, 28, 18, 150, 88)

    $borderPen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(60, 255, 255, 255), 4)
    $Graphics.DrawPath($borderPen, $path)

    $borderPen.Dispose()
    $highlight.Dispose()
    $brush.Dispose()
    $path.Dispose()
}

function Draw-Monogram {
    param(
        [System.Drawing.Graphics]$Graphics,
        [string]$Text,
        [float]$FontSize,
        [float]$Y,
        [System.Drawing.Color]$Color
    )

    $font = New-Object System.Drawing.Font("Segoe UI", $FontSize, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $brush = New-Object System.Drawing.SolidBrush($Color)
    $format = New-Object System.Drawing.StringFormat
    $format.Alignment = [System.Drawing.StringAlignment]::Center
    $format.LineAlignment = [System.Drawing.StringAlignment]::Center
    $rect = New-Object System.Drawing.RectangleF(0, $Y, 256, 100)

    $Graphics.DrawString($Text, $font, $brush, $rect, $format)

    $format.Dispose()
    $brush.Dispose()
    $font.Dispose()
}

$targets = @(
    @{
        Name = "KnotBraid"
        PngPath = "E:\KnotBraid\KnotBraidLauncher\resources\icons\app_knotbraid.png"
        IcoPath = "E:\KnotBraid\KnotBraidLauncher\resources\icons\app_knotbraid.ico"
        Painter = {
            param($g, $size)

            Draw-Background -Graphics $g -Size $size `
                -TopColor ([System.Drawing.Color]::FromArgb(255, 38, 70, 83)) `
                -BottomColor ([System.Drawing.Color]::FromArgb(255, 233, 196, 106))

            $penA = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(235, 250, 250, 250), 16)
            $penA.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
            $penA.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
            $penB = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(215, 22, 33, 62), 9)
            $penB.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
            $penB.EndCap = [System.Drawing.Drawing2D.LineCap]::Round

            $g.DrawArc($penA, 34, 54, 88, 88, 32, 308)
            $g.DrawArc($penA, 134, 54, 88, 88, 212, 308)
            $g.DrawArc($penB, 34, 54, 88, 88, 210, 100)
            $g.DrawArc($penB, 134, 54, 88, 88, 30, 100)

            Draw-Monogram -Graphics $g -Text "KB" -FontSize 90 -Y 122 -Color ([System.Drawing.Color]::FromArgb(245, 255, 255, 255))

            $penA.Dispose()
            $penB.Dispose()
        }
    },
    @{
        Name = "LogiKnotting"
        PngPath = "E:\KnotBraid\LogiKnotting\resources\icons\app_logiknotting.png"
        IcoPath = "E:\KnotBraid\LogiKnotting\resources\icons\app_logiknotting.ico"
        Painter = {
            param($g, $size)

            Draw-Background -Graphics $g -Size $size `
                -TopColor ([System.Drawing.Color]::FromArgb(255, 20, 55, 88)) `
                -BottomColor ([System.Drawing.Color]::FromArgb(255, 63, 114, 175))

            $loopPen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(235, 255, 247, 230), 15)
            $loopPen.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
            $loopPen.EndCap = [System.Drawing.Drawing2D.LineCap]::Round

            $g.DrawBezier($loopPen,
                (New-Object System.Drawing.Point(48, 156)),
                (New-Object System.Drawing.Point(70, 48)),
                (New-Object System.Drawing.Point(172, 52)),
                (New-Object System.Drawing.Point(186, 118)))
            $g.DrawBezier($loopPen,
                (New-Object System.Drawing.Point(186, 118)),
                (New-Object System.Drawing.Point(200, 188)),
                (New-Object System.Drawing.Point(116, 204)),
                (New-Object System.Drawing.Point(88, 142)))
            $g.DrawBezier($loopPen,
                (New-Object System.Drawing.Point(88, 142)),
                (New-Object System.Drawing.Point(122, 110)),
                (New-Object System.Drawing.Point(158, 122)),
                (New-Object System.Drawing.Point(198, 206)))

            $accent = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(255, 246, 189, 96))
            $g.FillEllipse($accent, 154, 68, 26, 26)

            Draw-Monogram -Graphics $g -Text "K" -FontSize 104 -Y 124 -Color ([System.Drawing.Color]::FromArgb(232, 255, 255, 255))

            $accent.Dispose()
            $loopPen.Dispose()
        }
    },
    @{
        Name = "LogiBraiding"
        PngPath = "E:\KnotBraid\LogiBraiding\resources\icons\app_logibraiding.png"
        IcoPath = "E:\KnotBraid\LogiBraiding\resources\icons\app_logibraiding.ico"
        Painter = {
            param($g, $size)

            Draw-Background -Graphics $g -Size $size `
                -TopColor ([System.Drawing.Color]::FromArgb(255, 45, 106, 79)) `
                -BottomColor ([System.Drawing.Color]::FromArgb(255, 96, 150, 102))

            $shadow = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(55, 0, 0, 0), 22)
            $shadow.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
            $shadow.EndCap = [System.Drawing.Drawing2D.LineCap]::Round

            $left = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(255, 231, 111, 81), 18)
            $mid = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(255, 244, 241, 222), 18)
            $right = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(255, 42, 157, 143), 18)

            foreach ($pen in @($left, $mid, $right)) {
                $pen.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
                $pen.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
            }

            $g.DrawBezier($shadow, 70, 56, 24, 96, 124, 162, 70, 208)
            $g.DrawBezier($shadow, 128, 48, 186, 90, 72, 162, 128, 210)
            $g.DrawBezier($shadow, 186, 56, 128, 102, 226, 162, 186, 206)

            $g.DrawBezier($left, 70, 50, 24, 90, 124, 156, 70, 202)
            $g.DrawBezier($mid, 128, 42, 186, 86, 72, 156, 128, 204)
            $g.DrawBezier($right, 186, 50, 128, 96, 226, 156, 186, 200)

            Draw-Monogram -Graphics $g -Text "B" -FontSize 96 -Y 126 -Color ([System.Drawing.Color]::FromArgb(215, 255, 255, 255))

            $shadow.Dispose()
            $left.Dispose()
            $mid.Dispose()
            $right.Dispose()
        }
    }
)

foreach ($target in $targets) {
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $target.PngPath) | Out-Null
    Save-IconPair -PngPath $target.PngPath -IcoPath $target.IcoPath -Painter $target.Painter
    Write-Host "Generated $($target.Name) icon assets"
}
