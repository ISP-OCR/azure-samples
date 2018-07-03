#include <windows.h>
#include <stdio.h>
#include <io.h>
#include "locale.h"

#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#include "MultiAnalyticalEngineWrapper.h"

#ifndef LOGD
#define LOGD(...) printf(__VA_ARGS__); printf("\n")
#endif

typedef int(*RECEIPTANALYZERGB)(unsigned int*, int, int, ReceiptInfo*);
typedef int(*INITRECEIPTLICENSE)(char*);

int execReceiptAnalysis(char* imageFilePath, char* licenseFilePath);
int readLicenseKey(char* licenseKey, char* licenseFilePath);
void outputReceipt(ReceiptInfo *info, int result);

int main(int argc, char* argv[]) {
	int result = RECEIPT_ANALYZE_ERR;

	LOGD("--------------------------------");
	LOGD(" ReceiptAnalyticalEngine Sample");
	LOGD("--------------------------------");

	// Check number of argument.
	if (argc < 3) {
		LOGD("Illegal Argument");
		LOGD(" 1st arg: image file path");
		LOGD(" 2nd arg: license file path");
		return result;
	}

	// Execute Receipt Analysis.
	result = execReceiptAnalysis(argv[1], argv[2]);

	return result;
}

int execReceiptAnalysis(char* imageFilePath, char* licenseFilePath) {
	int result = RECEIPT_ANALYZE_ERR;
	char licenseKey[256] = "";
	ReceiptInfo receiptInfo;
	HINSTANCE hinstLib = NULL;
	RECEIPTANALYZERGB ReceiptAnalyzeRGBFunc = NULL;
	INITRECEIPTLICENSE InitReceiptLicenseFunc = NULL;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken = NULL;
	wchar_t path[MAX_PATH];
	size_t  pathLength = 0;
	int width;
	int height;
	unsigned int *image;
	Gdiplus::Bitmap* pBitmap;
	Gdiplus::Rect rect;
	Gdiplus::BitmapData data;
	byte* ptr;
	byte* buff;
	int i;

	// Read license key.
	memset(licenseKey, 0, sizeof(licenseKey));
	if (!readLicenseKey(licenseKey, licenseFilePath)) {
		LOGD("Failed to read license key.");
		return result;
	}

	// Load library
	hinstLib = LoadLibrary(TEXT("MultiAnalyticalEngine.dll"));
	if (hinstLib == NULL) {
		LOGD("Failed to load library.");
		return result;
	}

	// Get API address
	ReceiptAnalyzeRGBFunc = (RECEIPTANALYZERGB)GetProcAddress(hinstLib, "ReceiptAnalyzeRGB");
	InitReceiptLicenseFunc = (INITRECEIPTLICENSE)GetProcAddress(hinstLib, "InitReceiptLicense");
	if (ReceiptAnalyzeRGBFunc == NULL || InitReceiptLicenseFunc == NULL) {
		LOGD("Failed to get API address.");
		FreeLibrary(hinstLib);
		hinstLib = NULL;
		return result;
	}

	// Initialize license key.
	result = InitReceiptLicenseFunc(licenseKey);
	if (MULTI_OK != result) {
		LOGD("%d: Failed to initialize license key.", result);
		FreeLibrary(hinstLib);
		hinstLib = NULL;
		return result;
	}

	// Initialize GDI+.
	if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Gdiplus::Ok) {
		FreeLibrary(hinstLib);
		hinstLib = NULL;
		return result;
	}

	// Read image
	setlocale(LC_ALL, "Japanese_Japan.932");
	if (mbstowcs_s(&pathLength, &path[0], MAX_PATH, imageFilePath, _TRUNCATE) != 0) {
		LOGD("Failed to read image.");
		FreeLibrary(hinstLib);
		hinstLib = NULL;
		return result;
	}
	pBitmap = Gdiplus::Bitmap::FromFile(path);
	if (pBitmap && pBitmap->GetLastStatus() == Gdiplus::Ok) {
		width = (int)pBitmap->GetWidth();
		height = (int)pBitmap->GetHeight();
	} else {
		LOGD("Failed to read image.");
		FreeLibrary(hinstLib);
		hinstLib = NULL;
		return result;
	}
	image = new unsigned int[width * height];
	rect = Gdiplus::Rect(0, 0, width, height);
	pBitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &data);
	ptr = (byte*)data.Scan0;
	buff = new byte[width * height * 4];
	memcpy(buff, ptr, (width * height * 4) * sizeof(byte));
	for (i = 0; i < width * height; i++) {
		image[i] = 0xff000000
			+ ((buff[(i * 4) + 2] << 16) & 0x00ff0000)
			+ ((buff[(i * 4) + 1] <<  8) & 0x0000ff00)
			+ ((buff[(i * 4) + 0]      ) & 0x000000ff);
	}
	delete[] buff;
	pBitmap->UnlockBits(&data);
	delete pBitmap;

	// Analyze receipt.
	memset(&receiptInfo, 0, sizeof(ReceiptInfo));
	result = ReceiptAnalyzeRGBFunc(image, width, height, &receiptInfo);
	delete[] image;

	// Output result.
	outputReceipt(&receiptInfo, result);

	// Finalize GDI+.
	if (gdiplusToken) {
		Gdiplus::GdiplusShutdown(gdiplusToken);
		gdiplusToken = NULL;
	}
	// Free library
	FreeLibrary(hinstLib);
	hinstLib = NULL;

	return result;
}

int readLicenseKey(char* licenseKey, char* licenseFilePath) {
	int result = 0;
	FILE *fp = NULL;
	char key[256];
	if (0 != fopen_s(&fp, licenseFilePath, "r")) {
		return result;
	}
	if (fgets(key, 256, fp)) {
		memmove(licenseKey, key, 256);
		result = 1;
	}
	fclose(fp);
	return result;
}

void outputReceipt(ReceiptInfo *info, int result) {
	int i;
	if (result < RECEIPT_OK) {
		LOGD("%d: Failed to analyze.", result);
	} else {
		LOGD("date: %04d/%02d/%02d %02d:%02d:%02d",
			info->date.year,
			info->date.month,
			info->date.day,
			info->date.hour,
			info->date.minute,
			info->date.second
			);
		LOGD("tel: %s", info->tel);
		LOGD("total: %d", info->total);
		LOGD("priceAdjustment: %d", info->priceAdjustment);
		for (i = 0; i < (int)info->cnt; i++) {
			LOGD("[item(%d/%d)]", (i + 1), (int)info->cnt);
			LOGD(" name: %s", info->items[i].name);
			LOGD(" price: %d", info->items[i].price);
			LOGD(" unitPrice: %d", info->items[i].unitPrice);
			LOGD(" itemNum: %d", info->items[i].itemNum);
		}
	}
}
