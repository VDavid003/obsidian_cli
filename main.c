#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include "obsidian.h"

void set_rgb(libusb_device_handle *dev_handle, enum rgbmode mode, int speed, uint8_t mcbitmask) {
	unsigned char data[8] = { 0x07, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	switch (mode) {
		case Off:
			data[3] = 0x07;
			break;
		case Static:
			data[3] = 0x06;
			break;
		case Blinky:
			data[3] = speed;
			break;
		case BlinkyMC:
			data[3] = 0x10 + speed;
			data[2] = mcbitmask;
			break;
	}
	SEND_DATA(data);
}

void get_mouse(libusb_device_handle **dev_handle) {
	libusb_device **devs;
	int i = 0;
	int r;

	r = libusb_get_device_list(NULL, &devs);
	if (r < 0) {
		return;
	}

	libusb_device *dev;
	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			libusb_free_device_list(devs, 1);
			return;
		}

		if (desc.idVendor == 0x18f8 && desc.idProduct == 0x1286) {
			r = libusb_open(dev, dev_handle);
			libusb_free_device_list(devs, 1);
			return;
		}
	}

	libusb_free_device_list(devs, 1);
}

enum mode{Init,Help,RGB};

int main(int argc, char *argv[]) {
	int r;
	enum mode mode = Init;
	libusb_device_handle *dev_handle = NULL;
	
	//RGB vars
	enum rgbmode rgbmode = Unknown;
	int rgbspeed = -1;
	uint8_t rgbmcfilter = 0;

	printf("Cross-platform Aula Obsidian settings program (c) VDavid003\nAula is probably (c) Suoai\nVersion:%s\n\n", VERSION);
	
	r = libusb_init(NULL);
	if (r < 0)
		return r;

	get_mouse(&dev_handle);
	if (dev_handle == NULL) {
		printf("Aula Obsidian Gaming Mouse not found or could not be opened.\nExiting...\n");
		goto quit;
	}

	//Could use getopt but let's just use standard c stuff wherever we can.
	if(argv[1]) {
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) 
		{
			mode = Help;
		} 
		else if (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--RGB") == 0) 
		{
			mode = RGB;
		} 
	}
	for (int i = 2; i < argc; i++) {
		switch (mode) {
			case RGB:
				if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--multicolor") == 0) 
				{
					rgbmode = BlinkyMC;
				} 
				else if (strcmp(argv[i], "-bl") == 0 || strcmp(argv[i], "--blinky") == 0) 
				{
					rgbmode = Blinky;
				} 
				else if (strcmp(argv[i], "-st") == 0 || strcmp(argv[i], "--static") == 0) 
				{
					rgbmode = Static;
				} 
				else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--off") == 0) 
				{
					rgbmode = Off;
				} 
				else if (strcmp(argv[i], "-sp") == 0 || strcmp(argv[i], "--speed") == 0) 
				{
					i++;
					rgbspeed = atoi(argv[i]);
				} 
				else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--red") == 0) 
				{
					rgbmcfilter |= MC_RED;
				} 
				else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--blue") == 0) 
				{
					rgbmcfilter |= MC_BLUE;
				} 
				else if (strcmp(argv[i], "-pu") == 0 || strcmp(argv[i], "--purple") == 0) 
				{
					rgbmcfilter |= MC_PURPLE;
				} 
				else if (strcmp(argv[i], "-pi") == 0 || strcmp(argv[i], "--pink") == 0) 
				{
					rgbmcfilter |= MC_PINK;
				} 
			break;
		}
		
	}

	libusb_detach_kernel_driver(dev_handle, 0);
	libusb_detach_kernel_driver(dev_handle, 1);
	switch (mode) {
		case Init:
		case Help:
			printf(	"Usage:\n"
				"%s [-h/-r] [mode-specific options]\n"
				"Modes:\n"
				"\tHelp (-h/--help):\n"
				"\t\tPrints this message.\n"
				"\tRGB (-r/--RGB):\n"
				"\t\tSets RGB options.\n"
				"\t\tParameters:\n"
				"\t\t\t-o/--off\t\tTurns RGB off.\n"
				"\t\t\t-st/--static\t\tTurns RGB to static mode.\n"
				"\t\t\t-bl/--blinky\t\tTurns RGB to blinky mode.\n"
				"\t\t\t-m/--multicolor\t\tTurns RGB to muilticolor blinky mode.\n"
				"\t\t\t-sp/--speed [0-5]\tSets blinking speed.\n"
				"\t\t\t-r/--red\t\tEnables Red in multicolored mode.\n"
				"\t\t\t-b/--blue\t\tEnables Blue in multicolored mode.\n"
				"\t\t\t-pu/--purple\t\tEnables Purple in multicolored mode.\n"
				"\t\t\t-pi/--pink\t\tEnables Pink in multicolored mode.\n", argv[0]);
			break;
		case RGB:
			switch (rgbmode) {
				case Unknown:
					printf("Please specify RGB mode!\n");
					goto close;
					break;
				case BlinkyMC:
					if(rgbmcfilter == 0) {
						printf("Please specify at least one color!\n");
						goto close;
					}
				case Blinky:
					if(rgbspeed < 0 || rgbspeed > 5) {
						printf("Please specify blinking speed between 0 and 5!\n");
						goto close;
					}
					break;
			}
			set_rgb(dev_handle, rgbmode, 5 - rgbspeed, rgbmcfilter);
			break;
	}

	close:
	libusb_attach_kernel_driver(dev_handle, 0);
	libusb_attach_kernel_driver(dev_handle, 1);
	printf("Closing...\n");
	libusb_close(dev_handle);

	quit:
	libusb_exit(NULL);
	return 0;
}
