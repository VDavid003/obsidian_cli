#define SEND_DATA(x) libusb_control_transfer(dev_handle, 0x21, 9, 0x0307, 1, x, 8, 0)

enum rgbmode{Blinky,BlinkyMC,Static,Off,Unknown};
#define MC_BLUE (1)
#define MC_PINK (1<<2)
#define MC_RED (1<<4)
#define MC_PURPLE (1<<6)

void set_rgb(libusb_device_handle *dev_handle, enum rgbmode mode, int speed, uint8_t mcbitmask);
