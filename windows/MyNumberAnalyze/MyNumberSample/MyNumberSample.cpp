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

#define CARD_TYPE_NOTIFICATION_CARD 1
#define CARD_TYPE_INDIVIDUAL_NUMBER_CARD_FRONT 2
#define CARD_TYPE_INDIVIDUAL_NUMBER_CARD_BACK 3

typedef int (*INITMULTIANALYTICALENGINE)();
typedef void (*FINALMULTIANALYTICALENGINE)();
typedef int (*MULTIANALYZEJPGFILEWRAPPER)(const char*, const char*, const char*, const char*, AnalyzeFormat);
typedef int (*GETMULTIANALYZEINFO)(int, MultiAnalyzeInfo*);
typedef int(*INITMULTILICENSEWRAPPER)(char*);

int execMyNumberAnalysis(char* imageFilePath, char* licenseFilePath, char* format);
int readLicenseKey(char* licenseKey, char* licenseFilePath);
void outputMyNumber(MultiAnalyzeInfo *info, int result, int type);

int main(int argc, char* argv[]) {
	int result = MULTI_ANALYZE_ERR;
	int type = 0;

	LOGD("---------------------------------");
	LOGD(" MyNumberAnalyticalEngine Sample");
	LOGD("---------------------------------");

	// Check number of argument.
	if (argc < 4) {
		LOGD("Illegal Argument");
		LOGD(" 1st arg: image file path");
		LOGD(" 2nd arg: license file path");
		LOGD(" 3rd arg: card type");
		LOGD("           1: Notification card");
		LOGD("           2: Individual Number Card (front)");
		LOGD("           3: Individual Number Card (back)");
		return result;
	}

	// Check card type
	type = atoi(argv[3]);
	if (CARD_TYPE_NOTIFICATION_CARD != type && CARD_TYPE_INDIVIDUAL_NUMBER_CARD_FRONT != type && CARD_TYPE_INDIVIDUAL_NUMBER_CARD_BACK != type) {
		LOGD("Illegal card type");
		LOGD(" 1: Notification card");
		LOGD(" 2: Individual Number Card (front)");
		LOGD(" 3: Individual Number Card (back)");
		return result;
	}

	// Execute MyNumber Analysis.
	result = execMyNumberAnalysis(argv[1], argv[2], argv[3]);

	return result;
}

int execMyNumberAnalysis(char* imageFilePath, char* licenseFilePath, char* format) {
	int result = MULTI_ANALYZE_ERR;
	char licenseKey[256] = "";
	MultiAnalyzeInfo analyzeInfo;
	int type = 0;
	AnalyzeFormat analyzeFormat = ANALYZE_FORMAT_120002;
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

	// Set AnalyzeFormat by card type.
	type = atoi(format);
	if (CARD_TYPE_NOTIFICATION_CARD == type) {
		analyzeFormat = ANALYZE_FORMAT_120002;
	}
	else if (CARD_TYPE_INDIVIDUAL_NUMBER_CARD_FRONT == type) {
		analyzeFormat = ANALYZE_FORMAT_90002;
	}
	else if (CARD_TYPE_INDIVIDUAL_NUMBER_CARD_BACK == type) {
		analyzeFormat = ANALYZE_FORMAT_80002;
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

	// Analyze mynumber.
	memset(&analyzeInfo, 0, sizeof(MultiAnalyzeInfo));
	result = MultiAnalyzeJpgFileWrapperFunc(imageFilePath, NULL, imageFilePath, "C:\\Program Files\\ispocr\\bin\\format\\", analyzeFormat);
	GetMultiAnalyzeInfoFunc(0, &analyzeInfo);

	// Output result.
	outputMyNumber(&analyzeInfo, result, type);

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

void outputMyNumber(MultiAnalyzeInfo *info, int result, int type) {
	if (CARD_TYPE_NOTIFICATION_CARD == type) {
		if (result < MULTI_OK && result != MULTI_CHECK_DIGIT_ERR) {
			LOGD("%d: Failed to analyze.", result);
		} else {
			if (result == MULTI_CHECK_DIGIT_ERR) {
				LOGD("%d: check digit error.", result);
			}
			LOGD("value2: %s", info->groups.value2);
			LOGD("value4: %s", info->groups.value4);
			LOGD("value5: %s", info->groups.value5);
			LOGD("value6: %s", info->groups.value6);
			LOGD("value7: %s", info->groups.value7);
			LOGD("value8: %s", info->groups.value8);
			LOGD("value9: %s", info->groups.value9);
			LOGD("value11: %s", info->groups.value11);
		}
	} else if (CARD_TYPE_INDIVIDUAL_NUMBER_CARD_FRONT == type) {
		if (result < MULTI_OK) {
			LOGD("%d: Failed to analyze.", result);
		} else {
			LOGD("value2: %s", info->groups.value2);
			LOGD("value4: %s", info->groups.value4);
			LOGD("value5: %s", info->groups.value5);
			LOGD("value6: %s", info->groups.value6);
			LOGD("value7: %s", info->groups.value7);
			LOGD("value8: %s", info->groups.value8);
			LOGD("value10: %s", info->groups.value10);
		}
	} else if (CARD_TYPE_INDIVIDUAL_NUMBER_CARD_BACK == type) {
		if (result < MULTI_OK && result != MULTI_CHECK_DIGIT_ERR) {
			LOGD("%d: Failed to analyze.", result);
		} else {
			if (result == MULTI_CHECK_DIGIT_ERR) {
				LOGD("%d: check digit error.", result);
			}
			LOGD("value2: %s", info->groups.value2);
			LOGD("value3: %s", info->groups.value3);
			LOGD("value5: %s", info->groups.value5);
		}
	}
}
