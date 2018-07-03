cd /d %~dp0

@if "check%2" == "check" (
	@if "check%1" == "check" (
		echo image.jpg key.txt
		ReceiptSample.exe image.jpg key.txt
		exit /B 0
	)
	echo %1 key.txt
	ReceiptSample.exe %1 key.txt
	exit /B 0
)
echo %1 %2
ReceiptSample.exe %1 %2
