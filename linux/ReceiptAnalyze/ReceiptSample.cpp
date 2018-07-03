#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <wand/MagickWand.h>
#include "ReceiptAnalyticalEngine.h"

#ifndef LOGD
#define LOGD(...) printf(__VA_ARGS__); printf("\n")
#endif

#define ANALYZE_WIDTH_MAX 4800
#define ANALYZE_HEIGHT_MAX 4800

// QXGA: 2048*1536
#define SIZE_3M 3145728
#define SIZE_PRE3M 3000000

#define ASPECT_RATIO_OTHER 0
#define ASPECT_RATIO_STANDARD 1
#define ASPECT_RATIO_WIDE 2

// QXGA: 2048*1536
#define STANDARD_WIDTH 2048
#define STANDARD_HEIGHT 1536
// WQHD: 2560*1440
#define WIDE_WIDTH 2560
#define WIDE_HEIGHT 1440

static int execReceiptAnalysis(char* imageFilePath, char* licenseFilePath);
static int readLicenseKey(char* licenseKey, char* licenseFilePath);
static void outputReceipt(ReceiptInfo *info, int result);
static int getAspectRatioKind(int width, int height);

int main(int argc, char *argv[]) {
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

static int execReceiptAnalysis(char* imageFilePath, char* licenseFilePath) {
	int result = RECEIPT_ANALYZE_ERR;
	char licenseKey[256] = "";
	MagickWand *magickWand;
	int width = 0;
	int height = 0;
	int pixelSize = 0;
	unsigned char *imageBuff;
	unsigned int *image;
	ReceiptInfo receiptInfo;
	int i;
	int aspectRatioKind = ASPECT_RATIO_OTHER;
	float zoomRate;

	// Read license key.
	memset(licenseKey, 0x00, sizeof(licenseKey));
	if (!readLicenseKey(licenseKey, licenseFilePath)) {
		LOGD("Failed to read license key.");
		return result;
	}

	// Initialize license.
	result = InitReceiptLicense(licenseKey);
	if (RECEIPT_OK != result) {
		LOGD("%d: Failed to initialize license.", result);
		return result;
	}

	// Read image file by MagickWand.
	MagickWandGenesis();
	magickWand = NewMagickWand();
	if (!MagickReadImage(magickWand, imageFilePath)) {
		LOGD("Failed to read image file.");
		magickWand = DestroyMagickWand(magickWand);
		MagickWandTerminus();
		return result;
	}

	// Check image size.
	width = (int)MagickGetImageWidth(magickWand);
	height = (int)MagickGetImageHeight(magickWand);
	pixelSize = width * height;
	// If image is too small, resize it.
	if (pixelSize < SIZE_PRE3M) {
		// Check aspect ratio.
		int aspectRatioKind = getAspectRatioKind(width, height);
		// 4:3
		if (aspectRatioKind == ASPECT_RATIO_STANDARD) {
			// QXGA: 2048*1536
			if (width > height) {
				width = STANDARD_WIDTH;
				height = STANDARD_HEIGHT;
			} else {
				width = STANDARD_HEIGHT;
				height = STANDARD_WIDTH;
			}
		// 16:9
		} else if (aspectRatioKind == ASPECT_RATIO_WIDE) {
			// WQHD: 2560*1440
			if (width > height) {
				width = WIDE_WIDTH;
				height = WIDE_HEIGHT;
			} else {
				width = WIDE_HEIGHT;
				height = WIDE_WIDTH;
			}
		// other
		} else {
			zoomRate = (float) sqrt((float)((float)SIZE_3M / (float)pixelSize));
			if (width > height) {
				if ((int)(width * zoomRate) > ANALYZE_WIDTH_MAX) {
					zoomRate = (float)((float)ANALYZE_WIDTH_MAX / (float)width);
				}
			} else {
				if ((int)(height * zoomRate) > ANALYZE_HEIGHT_MAX) {
					zoomRate = (float)((float)ANALYZE_HEIGHT_MAX / (float)height);
				}
			}
			width = (int)((float)width * zoomRate);
			height = (int)((float)height * zoomRate);
		}
		pixelSize = width * height;
		if (!MagickResizeImage(magickWand, (unsigned long)width, (unsigned long)height, LanczosFilter, 1.0f)) {
			LOGD("Failed to read image file.");
			magickWand = DestroyMagickWand(magickWand);
			MagickWandTerminus();
			return result;
		}
	}

	// Read pixels.
	imageBuff = (unsigned char *) malloc(pixelSize * 3 * sizeof(unsigned char));
	memset(imageBuff, 0x00, pixelSize * 3 * sizeof(unsigned char));
	if (!MagickExportImagePixels(magickWand, 0, 0, (unsigned long)width, (unsigned long)height, "RGB", CharPixel, imageBuff)) {
		LOGD("Failed to read image file.");
		free(imageBuff);
		magickWand = DestroyMagickWand(magickWand);
		MagickWandTerminus();
		return result;
	}
	magickWand = DestroyMagickWand(magickWand);
	MagickWandTerminus();

	image = (unsigned int *) malloc(pixelSize * sizeof(unsigned int));
	memset(image, 0x00, pixelSize * sizeof(unsigned int));
	for (i = 0; i < pixelSize; i++) {
		image[i] = 0xff000000
				+ ((((int)imageBuff[(i * 3) + 0]) << 16) & 0x00ff0000)
				+ ((((int)imageBuff[(i * 3) + 1]) <<  8) & 0x0000ff00)
				+ ((((int)imageBuff[(i * 3) + 2])      ) & 0x000000ff);
	}
	free(imageBuff);
	imageBuff = NULL;

	// Analyze receipt.
	memset(&receiptInfo, 0, sizeof(ReceiptInfo));
	result = ReceiptAnalyzeRGB(image, width, height, &receiptInfo);
	free(image);
	image = NULL;

	// Output result.
	outputReceipt(&receiptInfo, result);

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

static void outputReceipt(ReceiptInfo *info, int result) {
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

static int getAspectRatioKind(int width, int height) {
	if (width * 3 == height * 4 || width * 4 == height * 3) {
		// 4:3
		return ASPECT_RATIO_STANDARD;
	} else if (width * 9 == height * 16 || width * 16 == height * 9) {
		// 16:9
		return ASPECT_RATIO_WIDE;
	} else {
		return ASPECT_RATIO_OTHER;
	}
}
