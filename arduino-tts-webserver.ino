#include "./src/Ethernet.h"

byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetServer server(80);

char string[16];
char *p;
String suburl;
void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.println("Ethernet WebServer Example");

    if (Ethernet.begin(mac) == 0)
    {

        Serial.println("Failed to configure Ethernet using DHCP");

        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {

            Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        }
        else if (Ethernet.linkStatus() == LinkOFF)
        {

            Serial.println("Ethernet cable is not connected.");
        }
    }

    // start the server
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
}

void loop()
{
    EthernetClient client = server.available();

    if (client)
    {
        while (client.connected())
        {
            if (client.available())
            {
                String rcvd = client.readStringUntil('\r');
                rcvd.trim();
                rcvd.toCharArray(string, sizeof(string));
                p = strtok(string, " ");
                while (p != NULL)
                {
                    p = strtok(NULL, " ");
                    if (p[0] == '/')
                    {
                        if (!strcmp(p, "/test"))
                            Serial.println("test");
                        if (!strcmp(p, "/relay1on"))
                            Serial.println("Relay 1 is on");
                        if (!strcmp(p, "/relay1off"))
                            Serial.println("Relay 1 is off");
                        if (!strcmp(p, "/relay2on"))
                            Serial.println("Relay 2 is on");
                        if (!strcmp(p, "/relay2off"))
                            Serial.println("Relay 2 is off");
                    }
                }
            }
        }
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();
        Serial.println("client disconnected");
    }
}
