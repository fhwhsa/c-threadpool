demo: threadpool.c demo.c
	gcc threadpool.c wrapFunc/wrap.c demo.c -pthread -w -o demo

generateStaticLib:
	gcc -c threadpool.c -w -o threadpool.o
	gcc -c wrapFunc/wrap.c -w -o wrap.o
	ar -rcs libthreadpool.a threadpool.o wrap.o
	rm threadpool.o wrap.o

.PHONY: clean
clean:
	rm -rf demo
