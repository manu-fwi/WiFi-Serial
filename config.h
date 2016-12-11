#ifndef CONFIG_H
#define CONFIG_H

#define CMD_PIN 5

#define CONNECTED 0x01 << 0
#define DATA_READY 0x01 << 1
#define ERROR 0x01 << 7
#define DEBUG(M) { Serial.print("DEBUG(HUZZAH-WIFI):");\
                   Serial.println(M); }
#define DEBUG(M)

#endif // CONFIG_H
