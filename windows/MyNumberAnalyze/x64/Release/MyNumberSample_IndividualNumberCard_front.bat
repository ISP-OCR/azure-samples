cd /d %~dp0

@if "check%2" == "check" (
	@if "check%1" == "check" (
		echo image.jpg key.txt
		MyNumberSample.exe image.jpg key.txt 2
		exit /B 0
	)
	echo %1 key.txt
	MyNumberSample.exe %1 key.txt 2
	exit /B 0
)
echo %1 %2
MyNumberSample.exe %1 %2 2
