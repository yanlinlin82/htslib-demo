.PHONY: all clean distclean

all: htslib-demo

clean:
	@rm -fv htslib-demo

distclean: clean
	${MAKE} -C htslib-1.9/ clean

htslib-demo: demo.cpp htslib-1.9/libhts.a
	g++ -Wall -O2 -o $@ $^ -Ihtslib-1.9/ -Lhtslib-1.9/ -lhts -lpthread -lz -lbz2 -llzma

htslib-1.9/libhts.a:
	${MAKE} -C htslib-1.9/ libhts.a
