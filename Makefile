LFLAGS = -lhidapi-libusb

all: ctl
ctl:
	gcc sbx-h6-ctl.c -o sbx-h6-ctl $(LFLAGS)
clean:
	rm sbx-h6-ctl
