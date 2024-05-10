demo: threadpool.c demo.c
	gcc threadpool.c demo.c -pthread -w -o demo

.PHONY: clean
clean:
	rm -rf demo
