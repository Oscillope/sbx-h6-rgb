#include <libusb-1.0/libusb.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	if (libusb_init(NULL)) {
		return -1;
	}
	// discover devices
	libusb_device_handle *handle;
	char errfn[40] = { 0 };
	int err = 0;
	handle = libusb_open_device_with_vid_pid(NULL, 0x041e, 0x3255);
	if (!handle) {
		err = 1;
		sprintf(errfn, "open");
		goto out;
	}
	err = libusb_set_auto_detach_kernel_driver(handle, 1);
	if (err) {
		sprintf(errfn, "detach");
		goto out;
	}
	err = libusb_claim_interface(handle, 3);
	if (err) {
		sprintf(errfn, "claim");
		goto out;
	}
	unsigned char buf[] = {0xff, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	err = libusb_control_transfer(handle, 0x21, 9, 0x02ff, 3, buf, 16, 0);
	if (err < 0) {
		sprintf(errfn, "xfer1");
		goto out;
	}
	buf[1] = 0x04;
	buf[3] = 0x00;
	buf[4] = 0xc0;
	buf[5] = 0xff;
	buf[6] = 0x10;
	err = libusb_control_transfer(handle, 0x21, 9, 0x02ff, 3, buf, 16, 0);
	if (err < 0) {
		sprintf(errfn, "xfer2");
		goto out;
	}
	buf[1] = 0x01;
	buf[4] = 0x00;
	buf[5] = 0x00;
	buf[6] = 0x00;
	err = libusb_control_transfer(handle, 0x21, 9, 0x02ff, 3, buf, 16, 0);
	if (err < 0) {
		sprintf(errfn, "xfer2");
		goto out;
	}
	libusb_close(handle);
out:
	libusb_exit(NULL);
	if (err < 0) {
		printf("%s: %s\n", errfn, libusb_strerror(err));
	} else {
		err = 0;
	}
	return err;
}
