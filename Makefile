# CXXFLAGS    = -std=c++11 -g -Wall -O3 -funroll-loops -mavx -mtune=native -flax-vector-conversions
CXXFLAGS    = -fcilkplus -std=c++11 -g -Wall -O3 -funroll-loops -mavx -mtune=native -flax-vector-conversions

PROGS = GW
all: ${PROGS}

clean:
	rm -rf ${PROGS}
