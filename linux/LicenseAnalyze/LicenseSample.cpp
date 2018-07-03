#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "MultiAnalyticalEngine.h"

#ifndef LOGD
#define LOGD(...) printf(__VA_ARGS__); printf("\n")
#endif

static int execLicenseAnalysis(char* imageFilePath, char* licenseFilePath, char* format);
static int readLicenseKey(char* licenseKey, char* licenseFilePath);
static void outputLicense(RecognizeInfo *info, int result);

int main(int argc, char *argv[]) {
	int result = MULTI_ANALYZE_ERR;

	LOGD("--------------------------------");
	LOGD(" LicenseAnalyticalEngine Sample");
	LOGD("--------------------------------");

	// Check number of argument.
	if (argc < 3) {
		LOGD("Illegal Argument");
		LOGD(" 1st arg: image file path");
		LOGD(" 2nd arg: license file path");
		return result;
	}

	// Execute License Analysis.
	result = execLicenseAnalysis(argv[1], argv[2], NULL);

	return result;
}

static int execLicenseAnalysis(char* imageFilePath, char* licenseFilePath, char* format) {
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

	// Analyze license.
	memset(&recognizeInfo, 0, sizeof(RecognizeInfo));
	result = MultiAnalyzeJpgFile(imageFilePath, NULL, imageFilePath, "/usr/local/bin/ispocr/format", ANALYZE_FORMAT_30002, &recognizeInfo);

	// Output result.
	outputLicense(&recognizeInfo, result);

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

static void outputLicense(RecognizeInfo *info, int result) {
	if (result < MULTI_OK && result != MULTI_CHECK_DIGIT_ERR && result != MULTI_CARD_EXPIRED_ERR) {
		LOGD("%d: Failed to analyze.", result);
	} else {
		if (result == MULTI_CHECK_DIGIT_ERR) {
			LOGD("%d: check digit error.", result);
		} else if (result == MULTI_CARD_EXPIRED_ERR) {
			LOGD("%d: card expired error.", result);
		}
		LOGD("value2: %s", info->groups[0].value2);
		LOGD("value4: %s", info->groups[0].value4);
		LOGD("value5: %s", info->groups[0].value5);
		LOGD("value6: %s", info->groups[0].value6);
		LOGD("value8: %s", info->groups[0].value8);
		LOGD("value7: %s", info->groups[0].value7);
		LOGD("value9: %s", info->groups[0].value9);
		LOGD("value11: %s", info->groups[0].value11);
		LOGD("value12: %s", info->groups[0].value12);
	}
}

