#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "MultiAnalyticalEngine.h"

#ifndef LOGD
#define LOGD(...) printf(__VA_ARGS__); printf("\n")
#endif

static int execAccountingAnalysis(char* imageFilePath, char* licenseFilePath, char* format);
static int readLicenseKey(char* licenseKey, char* licenseFilePath);
static void outputAccounting(RecognizeInfo *info, int result);

int main(int argc, char *argv[]) {
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

static int execAccountingAnalysis(char* imageFilePath, char* licenseFilePath, char* format) {
	int result = MULTI_ANALYZE_ERR;
	char licenseKey[256] = "";
	RecognizeInfo recognizeInfo;

	// Read license key.
	memset(licenseKey, 0x00, sizeof(licenseKey));
	if (!readLicenseKey(licenseKey, licenseFilePath)) {
		LOGD("Failed to read license key.");
		return result;
	}

	// Initialize license key.
	result = InitMultiLicense(licenseKey);
	if (MULTI_OK != result) {
		LOGD("%d: Failed to initialize license key.", result);
		return result;
	}

	// Analyze accounting.
	memset(&recognizeInfo, 0, sizeof(RecognizeInfo));
	result = MultiAnalyzeJpgFile(imageFilePath, NULL, imageFilePath, "", ANALYZE_FORMAT_10002, &recognizeInfo);

	// Output result.
	outputAccounting(&recognizeInfo, result);

	return result;
}

static int readLicenseKey(char* licenseKey, char* licenseFilePath) {
	int result = 0;
	FILE *fp;
    char key[256];
	if ((fp = fopen(licenseFilePath, "r")) == NULL) {
		return result;
	}
	if (fgets(key, 256, fp) != NULL) {
		memmove(licenseKey, key, 256);
		result = 1;
	}
	fclose(fp);
	return result;
}

static void outputAccounting(RecognizeInfo *info, int result) {
	if (result < MULTI_OK) {
		LOGD("%d: Failed to analyze.", result);
	} else {
		LOGD("value2: %s", info->groups[0].value2);
		LOGD("value3: %s", info->groups[0].value3);
		LOGD("value4: %s", info->groups[0].value4);
		LOGD("value5: %s", info->groups[0].value5);
		LOGD("value6: %s", info->groups[0].value6);
		LOGD("value8: %s", info->groups[0].value8);
		LOGD("value9: %s", info->groups[0].value9);
		LOGD("value10: %s", info->groups[0].value10);
		LOGD("value11: %s", info->groups[0].value11);
		LOGD("value12: %s", info->groups[0].value12);
	}
}

