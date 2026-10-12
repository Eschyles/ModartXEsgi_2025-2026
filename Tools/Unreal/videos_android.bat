@echo off
set "CONTENT=D:\Travail\FREELANCE\Cours\2026\DFD5ESGIProjet\ModartXEsgi_2025-2026\Content\Movies"
set "ANDROID=D:\Travail\FREELANCE\Cours\2026\DFD5ESGIProjet\ModartXEsgi_2025-2026\Movies_Android"
set "BACKUP=D:\Travail\FREELANCE\Cours\2026\DFD5ESGIProjet\ModartXEsgi_2025-2026\Movies_Original_Backup"

echo Sauvegarde des videos originales...
robocopy "%CONTENT%" "%BACKUP%" /E /NFL /NDL
echo Mise en place des videos Android...
robocopy "%ANDROID%" "%CONTENT%" /E /NFL /NDL

echo.
echo LANCE LE PACKAGING ANDROID MAINTENANT.
echo Quand il est termine, appuie sur une touche pour restaurer les originales.
pause

echo Restauration des videos originales...
robocopy "%BACKUP%" "%CONTENT%" /E /NFL /NDL
echo Termine.
pause