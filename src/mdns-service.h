#ifndef MDNS_SERVICE_H
#define MDNS_SERVICE_H

#include <Arduino.h>
#include <ESPmDNS.h>

// https://techtutorialsx.com/2021/dddd10/29/esp32-mdns-host-name-resolution/
IPAddress serverIp;

void start_mdns_service()
{
    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        printf("MDNS Init failed: %d\n", err);
        return;
    }

}

void resolveLumi() {
    while (serverIp.toString() == "0.0.0.0") {
        delay(250);
        serverIp = MDNS.queryHost("Lumifera");
    }
}

#endif