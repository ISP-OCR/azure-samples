all: ReceiptSample.o
	LD_LIBRARY_PATH=/usr/local/lib64 g++ -o ReceiptSample \
	ReceiptSample.o \
	-lReceiptAnalyticalEngine \
	-lMagickWand

ReceiptSample.o: ReceiptSample.cpp
	g++ -O2 -I/usr/include/ImageMagick -I/usr/local/include/ispocr -fPIC -c ReceiptSample.cpp

clean:
	rm -f *.o
	rm -f ReceiptSample

