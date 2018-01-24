all: httpd

httpd: reverse_httpd.c
	gcc -g -o httpd reverse_httpd.c -lpthread


clean:
	rm httpd
