CFLAGS = -lusb-1.0

all: test ctl
test:
	gcc $(CFLAGS) sbx-h6-test.c -o sbx-h6-test
ctl:
	gcc $(CFLAGS) sbx-h6-ctl.c -o sbx-h6-ctl
clean:
	rm sbx-h6-test sbx-h6-ctl
