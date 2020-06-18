CFLAGS = -lhidapi-libusb

all: ctl
ctl:
	gcc $(CFLAGS) sbx-h6-ctl.c -o sbx-h6-ctl
clean:
	rm sbx-h6-ctl
