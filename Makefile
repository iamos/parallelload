loader: sources/parallelload.c
	clang -fPIC -shared -o paraload.so sources/parallelload.c -ldl -lm -pthread
	echo "LD_PRELOAD=./paraload.so [target]"

collector: sources/collector.c
	clang -fPIC -shared -o collector.so sources/collector.c -ldl -lm -pthread