CFLAGS = -lusb-1.0

all:
	gcc $(CFLAGS) sbx-h6-test.c -o sbx-h6-test
clean:
	rm sbx-h6-test
