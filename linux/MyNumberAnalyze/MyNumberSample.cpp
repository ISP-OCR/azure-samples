#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "MultiAnalyticalEngine.h"

#ifndef LOGD
#define LOGD(...) printf(__VA_ARGS__); printf("\n")
#endif

#define CARD_TYPE_NOTIFICATION_CARD 1
#define CARD_TYPE_INDIVIDUAL_NUMBER_CARD_FRONT 2
#define CARD_TYPE_INDIVIDUAL_NUMBER_CARD_BACK 3

static int execMyNumberAnalysis(char* imageFilePath, char* licenseFilePath, char* format);
static int readLicenseKey(char* licenseKey, char* licenseFilePath);
static void outputMyNumber(RecognizeInfo *info, int result, int type);

int main(int argc, char *argv[]) {
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

static int execMyNumberAnalysis(char* imageFilePath, char* licenseFilePath, char* format) {
	int result = MULTI_ANALYZE_ERR;
	char licenseKey[256] = "";
	RecognizeInfo recognizeInfo;
	int type = 0;
	AnalyzeFormat analyzeFormat = ANALYZE_FORMAT_120002;

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

	// Set AnalyzeFormat by card type.
	type = atoi(format);
	if (CARD_TYPE_NOTIFICATION_CARD == type) {
		analyzeFormat = ANALYZE_FORMAT_120002;
	} else if (CARD_TYPE_INDIVIDUAL_NUMBER_CARD_FRONT == type) {
		analyzeFormat = ANALYZE_FORMAT_90002;
	} else if (CARD_TYPE_INDIVIDUAL_NUMBER_CARD_BACK == type) {
		analyzeFormat = ANALYZE_FORMAT_80002;
	}

	// Analyze mynumber.
	memset(&recognizeInfo, 0, sizeof(RecognizeInfo));
	result = MultiAnalyzeJpgFile(imageFilePath, NULL, imageFilePath, "/usr/local/bin/ispocr/format", analyzeFormat, &recognizeInfo);

	// Output result.
	outputMyNumber(&recognizeInfo, result, type);

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

static void outputMyNumber(RecognizeInfo *info, int result, int type) {
	if (CARD_TYPE_NOTIFICATION_CARD == type) {
		if (result < MULTI_OK && result != MULTI_CHECK_DIGIT_ERR) {
			LOGD("%d: Failed to analyze.", result);
		} else {
			if (result == MULTI_CHECK_DIGIT_ERR) {
				LOGD("%d: check digit error.", result);
			}
			LOGD("value2: %s", info->groups[0].value2);
			LOGD("value4: %s", info->groups[0].value4);
			LOGD("value5: %s", info->groups[0].value5);
			LOGD("value6: %s", info->groups[0].value6);
			LOGD("value7: %s", info->groups[0].value7);
			LOGD("value8: %s", info->groups[0].value8);
			LOGD("value9: %s", info->groups[0].value9);
			LOGD("value11: %s", info->groups[0].value11);
		}
	} else if (CARD_TYPE_INDIVIDUAL_NUMBER_CARD_FRONT == type) {
		if (result < MULTI_OK) {
			LOGD("%d: Failed to analyze.", result);
		} else {
			LOGD("value2: %s", info->groups[0].value2);
			LOGD("value4: %s", info->groups[0].value4);
			LOGD("value5: %s", info->groups[0].value5);
			LOGD("value6: %s", info->groups[0].value6);
			LOGD("value7: %s", info->groups[0].value7);
			LOGD("value8: %s", info->groups[0].value8);
			LOGD("value10: %s", info->groups[0].value10);
		}
	} else if (CARD_TYPE_INDIVIDUAL_NUMBER_CARD_BACK == type) {
		if (result < MULTI_OK && result != MULTI_CHECK_DIGIT_ERR) {
			LOGD("%d: Failed to analyze.", result);
		} else {
			if (result == MULTI_CHECK_DIGIT_ERR) {
				LOGD("%d: check digit error.", result);
			}
			LOGD("value2: %s", info->groups[0].value2);
			LOGD("value3: %s", info->groups[0].value3);
			LOGD("value5: %s", info->groups[0].value5);
		}
	}
}

