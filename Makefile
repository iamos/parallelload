loader: sources/parallelload.c
	clang -fPIC -shared -o paraload.so sources/parallelload.c -ldl -lm -pthread
	echo "LD_PRELOAD=./paraload.so [target]"