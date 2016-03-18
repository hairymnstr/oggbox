#ifndef DOUGAL_USB_H
#define DOUGAL_USB_H 1

#define USB_STATUS_DOWN 0
#define USB_STATUS_STARTING 1
#define USB_STATUS_ENUMERATING 2
#define USB_STATUS_ACTIVE 3
#define USB_STATUS_CHARGER 4

int usb_get_status();
void usb_init();
void usb_sys_tick_handler();
void usb_off();

#endif /* ifndef DOUGAL_USB_H */
