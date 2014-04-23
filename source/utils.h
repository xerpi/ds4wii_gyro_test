#ifndef UTILS_H
#define UTILS_H

#include <gccore.h>
#include <ogc/machine/processor.h>


#define VID 0x054C
#define PID 0x05C4


struct ds4_input {
    u8 report_ID;
    u8 leftX;
    u8 leftY;
    u8 rightX;
    u8 rightY;
    
    u8 triangle : 1;
    u8 circle   : 1;
    u8 cross    : 1;
    u8 square   : 1;
    u8 dpad     : 4;
  
    u8 R3      : 1;
    u8 L3      : 1;
    u8 options : 1;
    u8 share   : 1;
    u8 R2      : 1;
    u8 L2      : 1;
    u8 R1      : 1;
    u8 L1      : 1;
    
    u8 cnt1   : 6;
    u8 tpad   : 1;
    u8 PS     : 1;
    
    u8 Ltrigger;
    u8 Rtrigger;
    
    u8 cnt2;
    u8 cnt3;
    
    u8 battery;
    
    s16 accelX;
    s16 accelY;
    s16 accelZ;
    
    union {
        s16 roll;
        s16 gyroZ;
    };
    union {
        s16 yaw;
        s16 gyroY;
    };
    union {
        s16 pitch;
        s16 gyroX;
    };
    
    u8 unk1[5];
    
    u8 padding       : 1;
    u8 microphone    : 1;
    u8 headphones    : 1;
    u8 usb_plugged   : 1;
    u8 battery_level : 4;
    
    u8 unk2[2];
    u8 trackpadpackets;
    u8 packetcnt;
    
    //1920x940
    u32 finger1active : 1;
    u32 finger1ID     : 7;
    u32 finger1X      : 12;
    u32 finger1Y      : 12;
    
    u32 finger2active : 1;
    u32 finger2ID     : 7;
    u32 finger2X      : 12;
    u32 finger2Y      : 12;
    
} __attribute__((packed, aligned(32)));


int r, g , b;

int get_device_id();
int open_device(int id);
int leds_rumble(int fd, int r, int g, int b);
int random_leds(int fd);
int fetch_data(int fd, struct ds4_input *data);
void print_data(struct ds4_input *data);
int get_bdaddrs(int fd, unsigned char *paired_mac, unsigned char *ds4_mac);
int set_paired_mac(int fd, unsigned char *mac);
int get_link_key(int fd, unsigned char *link_key);

int removal_notify_cb(int result, void *usrdata);
int change_notify_cb(int result, void *usrdata);


void WiiResetPressed();
void WiiPowerPressed();
int run;

#endif
