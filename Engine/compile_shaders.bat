@echo off

:: TODO: Only build when file has changed
:: TODO: Compile to .obj files so the linker can insert them into executable??

if exist %OUT_DIR%core\shaders (
	echo Clearing shaders folder
	rmdir /s /q %OUT_DIR%core\shaders
)

setlocal enabledelayedexpansion

for /r %%f in (*.vert *.frag) do (
	set FILE_PATH=%%~dpnxf
	set FILE_DIR=%%~dpf

	set RELATIVE_PATH=!FILE_PATH:%PROJ_DIR%=!
	set RELATIVE_DIR=!FILE_DIR:%PROJ_DIR%=!

	set OUTPUT_PATH=!OUT_DIR!!RELATIVE_PATH!
	set OUTPUT_DIR=!OUT_DIR!!RELATIVE_DIR!

	mkdir !OUTPUT_DIR!

	glslc !FILE_PATH! -o !OUTPUT_PATH!.spv
)

endlocal