@echo off
set "SRC=D:\Travail\FREELANCE\Cours\2026\DFD5ESGIProjet\ModartXEsgi_2025-2026\Content\Movies"
set "DST=D:\Travail\FREELANCE\Cours\2026\DFD5ESGIProjet\ModartXEsgi_2025-2026\Movies_Android"
if not exist "%DST%" mkdir "%DST%"
for %%f in ("%SRC%\*.mp4" "%SRC%\*.mov") do (
 ffmpeg -y -i "%%f" -vf "scale=trunc(iw/4)*2:trunc(ih/4)*2" -c:v libx264 -profile:v high -pix_fmt yuv420p -crf 26 -preset slow -c:a aac -b:a 128k -movflags +faststart "%DST%\%%~nxf"
)
pause