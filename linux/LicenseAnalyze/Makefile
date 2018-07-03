all: LicenseSample.o
	LD_LIBRARY_PATH=/usr/local/lib64 g++ -o LicenseSample \
	LicenseSample.o \
	-lMultiAnalyticalEngine

LicenseSample.o: LicenseSample.cpp
	g++ -O2 -I/usr/local/include/ispocr -fPIC -c LicenseSample.cpp

clean:
	rm -f *.o
	rm -f LicenseSample

