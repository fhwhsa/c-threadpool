demo: threadpool.c demo.c
	gcc threadpool.c wrapFunc/wrap.c demo.c -pthread -w -o demo

.PHONY: clean
clean:
	rm -rf demo
