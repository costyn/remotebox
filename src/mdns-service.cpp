#include "mdns-service.h"

// https://techtutorialsx.com/2021/dddd10/29/esp32-mdns-host-name-resolution/
String getLumiUrl()
{
    IPAddress serverIp;

    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        printf("MDNS Init failed: %d\n", err);
        return "";
    }

    Serial.print("Resolving host Lumifera: ");
    // display.println("Resolving Lumifera: ");
    // display.display();

    while (serverIp.toString() == "0.0.0.0") {
        delay(250);
        serverIp = MDNS.queryHost("Lumifera");
    }
    
    Serial.println(serverIp.toString());
    // display.println(" " + serverIp.toString());
    // display.println();
    // display.display();

    return "ws://" + serverIp.toString() + ":" + WEBSOCKET_SERVER_PORT + WEBSOCKET_SERVER_PATH;
}
