all: MyNumberSample.o
	LD_LIBRARY_PATH=/usr/local/lib64 g++ -o MyNumberSample \
	MyNumberSample.o \
	-lMultiAnalyticalEngine

MyNumberSample.o: MyNumberSample.cpp
	g++ -O2 -I/usr/local/include/ispocr -fPIC -c MyNumberSample.cpp

clean:
	rm -f *.o
	rm -f MyNumberSample

