#include <argp.h>
#include <arpa/inet.h>
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *argp_program_version = "sbx-h6-ctl v0.1";
static char doc[] = "SoundblasterX H6 LED/DSP Control\n\tCurrently only allows setting the color.";
static char args_doc[] = "vendorId:productId";
static struct argp_option options[] = {
	{"color",	'c',	"HEX_COLOR",	0,	"Hex color to display on the headset's LED rings.", 0},
	{ 0 }
};

struct sbx_h6_settings {
	uint32_t vendor_id;
	uint32_t product_id;
	libusb_device_handle *handle;
	uint32_t color;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct sbx_h6_settings *settings = state->input;
	int err = 0;
	switch (key) {
	case 'c':
		settings->color = strtol(arg, NULL, 16);
		break;
	case ARGP_KEY_ARG:
	{
		char *vendor = strsep(&arg, ":");
		settings->vendor_id = strtol(vendor, NULL, 16);
		settings->product_id = strtol(arg, NULL, 16);
		break;
	}
	case ARGP_KEY_NO_ARGS:
		argp_error(state, "Device ID is required");
		return -1;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp_setup = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

// Helper function to convert from RGB to RBG (headset takes RBG)
uint32_t sbx_rgb_to_rbg(uint32_t color)
{
	uint8_t r = (color & 0xff0000) >> 16;
	uint8_t g = (color & 0x00ff00) >> 8;
	uint8_t b = (color & 0x0000ff);
	return (r | (b << 8) | (g << 16));
}

#define SBX_H6_CMD_INDEX	1
#define SBX_H6_START_CMD	0x03000100
#define SBX_H6_COLOR_CMD	0x04000000
#define SBX_H6_COLOR_INDEX	4
#define SBX_H6_END_CMD		0x01000000
#define SBX_H6_CMD_LEN		16
int sbx_set_color(struct sbx_h6_settings *settings)
{
	// It seems like this string precedes every color write
	unsigned char buf[SBX_H6_CMD_LEN] = { 0 };
	int err = 0;
	buf[0] = 0xff;	// The first byte is always ff
	*(uint32_t*)(buf + SBX_H6_CMD_INDEX) = htonl(SBX_H6_START_CMD);
	err = libusb_control_transfer(settings->handle, 0x21, 9, 0x02ff, 3, buf, SBX_H6_CMD_LEN, 0);
	if (err < 0) {
		printf("err %d during start transfer\n", err);
		return err;
	}
	*(uint32_t*)(buf + SBX_H6_CMD_INDEX) = htonl(SBX_H6_COLOR_CMD);
	// The actual color goes in these 3 bytes
	*(uint32_t*)(buf + SBX_H6_COLOR_INDEX) = sbx_rgb_to_rbg(settings->color);
	err = libusb_control_transfer(settings->handle, 0x21, 9, 0x02ff, 3, buf, SBX_H6_CMD_LEN, 0);
	if (err < 0) {
		printf("err %d during color transfer\n", err);
		return err;
	}
	// Then we do this to display the color
	*(uint64_t*)(buf + SBX_H6_CMD_INDEX) = htonl(SBX_H6_END_CMD);
	err = libusb_control_transfer(settings->handle, 0x21, 9, 0x02ff, 3, buf, SBX_H6_CMD_LEN, 0);
	if (err < 0) {
		printf("err %d during end transfer\n", err);
		return err;
	}
	return 0;
}

int main(int argc, char **argv)
{
	struct sbx_h6_settings headset;
	int err = 0;
	memset(&headset, 0, sizeof(headset));
	argp_parse(&argp_setup, argc, argv, 0, 0, &headset);

	err = libusb_init(NULL);
	if (err) {
		printf("libusb init error %s", libusb_strerror(err));
		return err;
	}
	headset.handle = libusb_open_device_with_vid_pid(NULL, headset.vendor_id, headset.product_id);
	if (!headset.handle) {
		printf("Error opening USB device %04x:%04x\n", headset.vendor_id, headset.product_id);
		libusb_exit(NULL);
		return 1;
	}
	err = libusb_set_auto_detach_kernel_driver(headset.handle, 1);
	if (!err) {
		err = libusb_claim_interface(headset.handle, 3);
	}
	if (err) {
		printf("libusb iface error %s", libusb_strerror(err));
		libusb_close(headset.handle);
		libusb_exit(NULL);
		return err;
	}
	if (headset.color) {
		printf("set color to %x\n", headset.color);
		err = sbx_set_color(&headset);
	}
	libusb_close(headset.handle);
	libusb_exit(NULL);
	if (err) {
		printf("failed with %s\n", libusb_strerror(err));
	}
	return err;
}
