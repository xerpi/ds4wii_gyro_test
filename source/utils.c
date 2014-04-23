#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>

int run = 1;
int r = 0x00, g = 0x0, b = 0xFF;

void print_data(struct ds4_input *data)
{ 
    printf("\x1b[10;0H");
    printf("PS: %1i   OPTIONS: %1i  SHARE: %1i   /\\: %1i   []: %1i   O: %1i   X: %1i\n", \
            data->PS, data->options, data->share, data->triangle, \
            data->square, data->circle, data->cross);
            
    printf("TPAD: %1i   L3: %1i   R3: %1i\n", \
            data->tpad, data->L3, data->R3);

    printf("L1: %1i   L2: %1i   R1: %1i   R2: %1i   DPAD: %1i\n", \
            data->L1, data->L2, data->R1, data->R2, \
            data->dpad);
    printf("LX: %2X   LY: %2X   RX: %2X   RY: %2X  battery: %1X\n", \
            data->leftX, data->leftY, data->rightX, data->rightY, data->battery);
    
    printf("headphones: %1X   microphone: %1X   usb_plugged: %1X  battery_level: %2X\n", \
            data->headphones, data->microphone, data->usb_plugged, data->battery_level);    

    printf("aX: %5hi  aY: %5hi  aZ: %5hi  gyroX: %5hi  gyroY: %5hi  gyroZ: %5hi\n", \
            data->accelX, data->accelY, data->accelZ, data->gyroX, data->gyroY, data->gyroZ);

    printf("Ltrigger: %2X   Rtrigger: %2X  trackpadpackets: %4i  packetcnt: %4i\n", \
            data->Ltrigger, data->Rtrigger, data->trackpadpackets, data->packetcnt);
            
    printf("f1active: %2X   f1ID: %2X  f1X: %4i  f1Y: %4i\n", \
            data->finger1active, data->finger1ID, data->finger1X, data->finger1Y);
    printf("f2active: %2X   f2ID: %2X  f2X: %4i  f2Y: %4i\n", \
            data->finger2active, data->finger2ID, data->finger2X, data->finger2Y);

}


int set_paired_mac(int fd, unsigned char *mac)
{
    u8 ATTRIBUTE_ALIGN(32) buf[] = {
        0x13,
        mac[5], mac[4], mac[3], mac[2], mac[1], mac[0],
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    return USB_WriteCtrlMsg(fd,
        USB_REQTYPE_INTERFACE_SET,
        USB_REQ_SETCONFIG,
        (USB_REPTYPE_FEATURE<<8) | 0x13,
        0,
        sizeof(buf),
        buf);
}

int get_link_key(int fd, unsigned char *link_key)
{
    u8 ATTRIBUTE_ALIGN(32) buf[0x10];
    int ret = USB_ReadCtrlMsg(fd,
        USB_REQTYPE_INTERFACE_GET,
        USB_REQ_CLEARFEATURE,
        (USB_REPTYPE_FEATURE<<8) | 0x13,
        0,
        sizeof(buf),
        buf);
    
    if (ret != USB_OK) return USB_FAILED;    
    
    int i;
    for(i = 0; i < ret; i++) {
        link_key[i] = buf[i];
    }
    return USB_OK;
}

int get_bdaddrs(int fd, unsigned char *paired_mac, unsigned char *ds4_mac)
{
    u8 ATTRIBUTE_ALIGN(32) buf[0x10];
    int ret = USB_ReadCtrlMsg(fd,
        USB_REQTYPE_INTERFACE_GET,
        USB_REQ_CLEARFEATURE,
        (USB_REPTYPE_FEATURE<<8) | 0x12,
        0,
        sizeof(buf),
        buf);
    
    if (ret != USB_OK) return USB_FAILED;
    
    if (paired_mac) {
        paired_mac[0] = buf[15];
        paired_mac[1] = buf[14];
        paired_mac[2] = buf[13];
        paired_mac[3] = buf[12];
        paired_mac[4] = buf[11];
        paired_mac[5] = buf[10];
    }
    if (ds4_mac) {
        ds4_mac[0] = buf[6];
        ds4_mac[1] = buf[5];
        ds4_mac[2] = buf[4];
        ds4_mac[3] = buf[3];
        ds4_mac[4] = buf[2];
        ds4_mac[5] = buf[1];
    }

    return USB_OK;
}

int fetch_data(int fd, struct ds4_input *data)
{
    int ret = USB_ReadIntrMsg(fd, 0xa1, sizeof(*data), (u8 *)data);
    
    //To big-endian
    data->gyroX  = bswap16(data->gyroX);
    data->gyroY  = bswap16(data->gyroY);
    data->gyroZ  = bswap16(data->gyroZ);
    data->accelX = bswap16(data->accelX);
    data->accelY = bswap16(data->accelY);
    data->accelZ = bswap16(data->accelZ);

    register u8 *p8 = ((u8*)data) + 36;
    data->finger1X = p8[0] | (p8[1]&0xF)<<8;
    data->finger1Y = ((p8[1]&0xF0)>>4) | (p8[2]<<4);
    data->finger1active = !data->finger1active;
    p8 += 4;
    data->finger2X = p8[0] | (p8[1]&0xF)<<8;
    data->finger2Y = ((p8[1]&0xF0)>>4) | (p8[2]<<4);
    data->finger2active = !data->finger2active;
    
    //data->dpad = (~data->dpad) & 0x7;

    return ret;
}

int leds_rumble(int fd, int r, int g, int b)
{
    u8 ATTRIBUTE_ALIGN(32) buf[] = {
        0x05, //Report ID
        0x03, 0x00, 0x00,
        0x00, //Fast motor
        0x00, //Slow motor
        r, g, b, // RGB
        0x00, //LED on duration
        0x00  //LED off duration
    };
    return USB_WriteIntrMsg(fd, 0x05, sizeof(buf), buf);
}

int random_leds(int fd)
{
    r = rand()%0xFF;
    g = rand()%0xFF;
    b = rand()%0xFF;
    return leds_rumble(fd, r, g, b);
}

int open_device(int id)
{
    int fd;
    USB_OpenDevice(id, VID, PID, &fd);
    USB_DeviceRemovalNotifyAsync(fd, removal_notify_cb, NULL);
    random_leds(fd);
    return fd;
}

int get_device_id()
{
    #define MAXCNT 10
    usb_device_entry devlist[MAXCNT];
    u8 i, devcnt = 0;
    USB_GetDeviceList(devlist, MAXCNT, USB_CLASS_HID, &devcnt);
    printf("Found %i USB device(s)\n", devcnt);
    for (i = 0; i < devcnt; i++) {
        printf ("%i VID: 0x%X  PID: 0x%X  dev_id: %i  token: %i\n", i, devlist[i].vid,
            devlist[i].pid, devlist[i].device_id, devlist[i].token);
        if (devlist[i].vid == VID && devlist[i].pid == PID) {
            return devlist[i].device_id;
        }
    }
    return 0;
}

int change_notify_cb(int result, void *usrdata)
{
    //printf("change_notify: %i\n", result);
    USB_DeviceChangeNotifyAsync(USB_CLASS_HID, change_notify_cb, NULL);
    return 1;
}

int removal_notify_cb(int result, void *usrdata)
{
    //printf("removal_notify: %i\n", result);
    return 1;
}

void WiiResetPressed()
{
    run = 0;
}

void WiiPowerPressed()
{
    run = 0;
}
