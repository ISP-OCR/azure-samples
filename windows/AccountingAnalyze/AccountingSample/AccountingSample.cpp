#include <windows.h>
#include <stdio.h>
#include <io.h>

#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#include "MultiAnalyticalEngineWrapper.h"

#ifndef LOGD
#define LOGD(...) printf(__VA_ARGS__); printf("\n")
#endif

typedef int (*INITMULTIANALYTICALENGINE)();
typedef void (*FINALMULTIANALYTICALENGINE)();
typedef int (*MULTIANALYZEJPGFILEWRAPPER)(const char*, const char*, const char*, const char*, AnalyzeFormat);
typedef int (*GETMULTIANALYZEINFO)(int, MultiAnalyzeInfo*);
typedef int(*INITMULTILICENSEWRAPPER)(char*);

int execAccountingAnalysis(char* imageFilePath, char* licenseFilePath, char* format);
int readLicenseKey(char* licenseKey, char* licenseFilePath);
void outputAccounting(MultiAnalyzeInfo *info, int result);

int main(int argc, char* argv[]) {
	int result = MULTI_ANALYZE_ERR;

	LOGD("-----------------------------------");
	LOGD(" AccountingAnalyticalEngine Sample");
	LOGD("-----------------------------------");

	// Check number of argument.
	if (argc < 3) {
		LOGD("Illegal Argument");
		LOGD(" 1st arg: image file path");
		LOGD(" 2nd arg: license file path");
		return result;
	}

	// Execute Accounting Analysis.
	result = execAccountingAnalysis(argv[1], argv[2], NULL);

	return result;
}

int execAccountingAnalysis(char* imageFilePath, char* licenseFilePath, char* format) {
	int result = MULTI_ANALYZE_ERR;
	char licenseKey[256] = "";
	MultiAnalyzeInfo analyzeInfo;
	HINSTANCE hinstLib = NULL;
	INITMULTIANALYTICALENGINE InitMultiAnalyticalEngineFunc = NULL;
	FINALMULTIANALYTICALENGINE FinalMultiAnalyticalEngineFunc = NULL;
	MULTIANALYZEJPGFILEWRAPPER MultiAnalyzeJpgFileWrapperFunc = NULL;
	GETMULTIANALYZEINFO GetMultiAnalyzeInfoFunc = NULL;
	INITMULTILICENSEWRAPPER InitMultiLicenseWrapperFunc = NULL;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken = NULL;

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
	InitMultiAnalyticalEngineFunc = (INITMULTIANALYTICALENGINE)GetProcAddress(hinstLib, "InitMultiAnalyticalEngine"); 
	FinalMultiAnalyticalEngineFunc = (FINALMULTIANALYTICALENGINE)GetProcAddress(hinstLib, "FinalMultiAnalyticalEngine");
	MultiAnalyzeJpgFileWrapperFunc = (MULTIANALYZEJPGFILEWRAPPER)GetProcAddress(hinstLib, "MultiAnalyzeJpgFileWrapper");
	GetMultiAnalyzeInfoFunc = (GETMULTIANALYZEINFO)GetProcAddress(hinstLib, "GetMultiAnalyzeInfo");
	InitMultiLicenseWrapperFunc = (INITMULTILICENSEWRAPPER)GetProcAddress(hinstLib, "InitMultiLicenseWrapper");
	if (InitMultiAnalyticalEngineFunc == NULL || FinalMultiAnalyticalEngineFunc == NULL || MultiAnalyzeJpgFileWrapperFunc == NULL || GetMultiAnalyzeInfoFunc == NULL || InitMultiLicenseWrapperFunc == NULL) {
		LOGD("Failed to get API address.");
		FreeLibrary(hinstLib);
		hinstLib = NULL;
		return result;
	}

	// Initialize license key.
	result = InitMultiLicenseWrapperFunc(licenseKey);
	if (MULTI_OK != result) {
		LOGD("%d: Failed to initialize license key.", result);
		FreeLibrary(hinstLib);
		hinstLib = NULL;
		return result;
	}

	// Initialize MultiAnalyticalEngine.
	result = InitMultiAnalyticalEngineFunc();
	if (MULTI_OK != result) {
		LOGD("%d: Failed to initialize MultiAnalyticalEngine.", result);
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

	// Analyze accounting.
	memset(&analyzeInfo, 0, sizeof(MultiAnalyzeInfo));
	result = MultiAnalyzeJpgFileWrapperFunc(imageFilePath, NULL, imageFilePath, "", ANALYZE_FORMAT_10002);
	GetMultiAnalyzeInfoFunc(0, &analyzeInfo);

	// Output result.
	outputAccounting(&analyzeInfo, result);

	// Finalize MultiAnalyticalEngine.
	FinalMultiAnalyticalEngineFunc();
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

void outputAccounting(MultiAnalyzeInfo *info, int result) {
	if (result < MULTI_OK) {
		LOGD("%d: Failed to analyze.", result);
	} else {
		LOGD("value2: %s", info->groups.value2);
		LOGD("value3: %s", info->groups.value3);
		LOGD("value4: %s", info->groups.value4);
		LOGD("value5: %s", info->groups.value5);
		LOGD("value6: %s", info->groups.value6);
		LOGD("value8: %s", info->groups.value8);
		LOGD("value9: %s", info->groups.value9);
		LOGD("value10: %s", info->groups.value10);
		LOGD("value11: %s", info->groups.value11);
		LOGD("value12: %s", info->groups.value12);
	}
}
