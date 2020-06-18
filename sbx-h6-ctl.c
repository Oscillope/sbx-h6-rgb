#include <argp.h>
#include <arpa/inet.h>
#include <hidapi/hidapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *argp_program_version = "sbx-h6-ctl v0.1";
static char doc[] = "SoundblasterX H6 LED/DSP Control\n\tCurrently only allows setting the color.";
static char args_doc[] = "vendorId:productId";
static struct argp_option options[] = {
	{"color",	'c',	"HEX_COLOR",	0,	"Hex color to display on the headset's LED rings.", 0},
	{"brightness",	'b',	"VALUE",	0,	"LED brightness (0-255)", 0},
	{ 0 }
};

struct sbx_h6_settings {
	uint32_t vendor_id;
	uint32_t product_id;
	hid_device *handle;
	uint32_t color;
	uint8_t brightness;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct sbx_h6_settings *settings = state->input;
	int err = 0;
	switch (key) {
	case 'b':
		settings->brightness = strtol(arg, NULL, 0);
		break;
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
uint32_t sbx_rgb_to_rbg(uint32_t color, uint8_t brightness)
{
	uint8_t bright = (0xff - brightness);
	uint8_t r = ((color & 0xff0000) >> 16);
	uint8_t g = ((color & 0x00ff00) >> 8);
	uint8_t b = (color & 0x0000ff);
	if (bright > r) {
		r = 0;
	} else {
		r -= bright;
	}
	if (bright > b) {
		b = 0;
	} else {
		b -= bright;
	}
	if (bright > g) {
		g = 0;
	} else {
		g -= bright;
	}
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
	unsigned char buf[SBX_H6_CMD_LEN] = { 0 };
	int err = 0;
	buf[0] = 0xff;	// The first byte is always ff
	// It seems like this string precedes every color write
	*(uint32_t*)(buf + SBX_H6_CMD_INDEX) = htonl(SBX_H6_START_CMD);
	err = hid_write(settings->handle, buf, SBX_H6_CMD_LEN);
	if (err < 0) {
		printf("err %d during start transfer\n", err);
		return err;
	}
	*(uint32_t*)(buf + SBX_H6_CMD_INDEX) = htonl(SBX_H6_COLOR_CMD);
	// The actual color goes in these 3 bytes
	*(uint32_t*)(buf + SBX_H6_COLOR_INDEX) = sbx_rgb_to_rbg(settings->color, settings->brightness);
	err = hid_write(settings->handle, buf, SBX_H6_CMD_LEN);
	if (err < 0) {
		printf("err %d during color transfer\n", err);
		return err;
	}
	// Then we do this to display the color
	*(uint64_t*)(buf + SBX_H6_CMD_INDEX) = htonl(SBX_H6_END_CMD);
	err = hid_write(settings->handle, buf, SBX_H6_CMD_LEN);
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
	headset.brightness = 0xff;
	argp_parse(&argp_setup, argc, argv, 0, 0, &headset);

	err = hid_init();
	if (err) {
		printf("hid init error %d\n", err);
		return err;
	}
	headset.handle = hid_open(headset.vendor_id, headset.product_id, NULL);
	if (!headset.handle) {
		printf("Error opening USB device %04x:%04x\n", headset.vendor_id, headset.product_id);
		hid_exit();
		return 1;
	}
	if (headset.color) {
		printf("set color to %x\n", headset.color);
		err = sbx_set_color(&headset);
	}
	hid_close(headset.handle);
	hid_exit();
	return err;
}
