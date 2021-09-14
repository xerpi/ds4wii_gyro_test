#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/machine/processor.h>
#include <grrlib.h>
#include <math.h>
#include "utils.h"
#include "verdana_ttf.h"

int main(int argc, char **argv)
{
    SYS_SetResetCallback(WiiResetPressed);
    SYS_SetPowerCallback(WiiPowerPressed);
    usleep(250*1000);
    GRRLIB_Init();
    GRRLIB_ttfFont *font = GRRLIB_LoadTTF(verdana_ttf, verdana_ttf_size);
    
    
    WPAD_Init();
    USB_Initialize();
    USB_DeviceChangeNotifyAsync(USB_CLASS_HID, change_notify_cb, NULL);    
    int dev_id = get_device_id();
    int fd = open_device(dev_id);
    
    struct ds4_input data;
    memset(&data, 0, sizeof(data));
    fetch_data(fd, &data);
    random_leds(fd);
    
    int screenW = rmode->fbWidth, screenH = rmode->efbHeight;
    float pos_x = screenW/2, pos_y = screenH/2;
    while (run) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);
        if (pressed & WPAD_BUTTON_B) {
            dev_id = get_device_id();
            fd = open_device(dev_id);
        }
        if (pressed & WPAD_BUTTON_A) {
            random_leds(fd);
        }
        GRRLIB_FillScreen(0x0);
        GRRLIB_PrintfTTF(15, 15, font, "ds4wii by xerpi | L1: center | R1: random color", 15, 0xFFFFFFFF);
        
        u32 color = (r<<24)|(g<<16)|(b<<8)|0xFF;
        GRRLIB_Circle(pos_x, pos_y, 15, color, 0);
        GRRLIB_Circle(pos_x, pos_y, 14, color, 0);
        
        memset(&data, 0, sizeof(data));
        fetch_data(fd, &data);
        if (data.L1) {pos_x = screenW/2, pos_y = screenH/2;}
        if (data.R1) {random_leds(fd);}
        //1920x940
        if (data.finger1active) {
            GRRLIB_Circle((screenW/1920.0f)*data.finger1X, (screenH/940.0f)*data.finger1Y, 10, 0x00FF00FF, 1);
        }
        if (data.finger2active) {
            GRRLIB_Circle((screenW/1920.0f)*data.finger2X, (screenH/940.0f)*data.finger2Y, 10, 0x0000FFFF, 1);
        }

        char buf[256];
        snprintf(buf, sizeof(buf), "aX: %8hi  aY: %8hi  aZ: %8hi", data.accelX, data.accelY, data.accelZ);
        GRRLIB_PrintfTTF(15, 30, font, buf, 14, 0xFFFFFFFF);
        
        snprintf(buf, sizeof(buf), "gyroX: %8hi  gyroY: %8hi  gyroZ: %8hi", data.gyroX, data.gyroY, data.gyroZ);
        GRRLIB_PrintfTTF(15, 45, font, buf, 14, 0xFFFFFFFF);
        
        #define THRESHOLD 50.0f
        if (fabs(data.accelX) > THRESHOLD)
            pos_y -= data.accelX/55.0f;
        if (fabs(data.accelY) > THRESHOLD)
            pos_x -= data.accelY/55.0f;
        
        if (pressed & WPAD_BUTTON_HOME) run = 0;
        GRRLIB_Render();
    }
    
    USB_CloseDevice(&fd);
    USB_Deinitialize();
    GRRLIB_FreeTTF (font);
    GRRLIB_Exit();
    exit(0);
    return 0;
}

