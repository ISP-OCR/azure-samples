all: AccountingSample.o
	LD_LIBRARY_PATH=/usr/local/lib64 g++ -o AccountingSample \
	AccountingSample.o \
	-lMultiAnalyticalEngine

AccountingSample.o: AccountingSample.cpp
	g++ -O2 -I/usr/local/include/ispocr -fPIC -c AccountingSample.cpp

clean:
	rm -f *.o
	rm -f AccountingSample

