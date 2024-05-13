#include "./src/Ethernet.h"

#define HEARTBEAT_LED_PIN 2
#define PWR_RELAY_PIN 6
#define SOUND_RELAY_PIN 7
#define DEBUG_LED1_PIN 8
#define DEBUG_LED2_PIN 9

#define HALL_SENSOR_PIN A1
#define SW1_PIN A2
#define SW2_PIN A3
#define SW3_PIN A4

class DigitalOutpin
{
private:
    int _p_no;
    int _state;

public:
    DigitalOutpin(int p_no)
    {
        pinMode(p_no, OUTPUT);
        _p_no = p_no;
    }

    void set(int val)
    {
        digitalWrite(_p_no, val);
        Serial.print("Pin ");
        Serial.print(_p_no);
        Serial.print(" is set to ");
        Serial.println(val);
        _state = val;
    }

    void toggle()
    {
        _state = !_state;
        digitalWrite(_p_no, _state);
    }
};

class HeartBeat : public DigitalOutpin
{
private:
    int _duty, _duty_count, _period;
    unsigned long _count;

public:
    HeartBeat(int p_no, int duty, int period) : DigitalOutpin(p_no)
    {
        _duty = duty;
        _duty_count = int(period * duty / 100);
        _period = period;
        _count = 0;
    }

    void run()
    {
        _count++;
        if ((_count == 1) | (_count == _duty_count))
            toggle();
        else if (_count == _period)
            _count = 0;
    }
};

class AnalogInpin
{
private:
    int _p_no;

public:
    AnalogInpin(int p_no)
    {
        pinMode(p_no, INPUT_PULLUP);
    }

    int get()
    {
        return analogRead(_p_no);
    }
};

int clientPrint(EthernetClient cl, char *msg)
{
    cl.println("HTTP/1.1 200 OK");
    cl.println("Content-Type: text/html");
    cl.println("Connection: close"); // the connection will be closed after completion of the response
    cl.println();
    cl.println("<!DOCTYPE HTML>");
    cl.println("<html>");
    cl.println(msg);
    cl.println("</html>");
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    cl.stop();
}

HeartBeat heartBeatLed(HEARTBEAT_LED_PIN, 2000, 5000);

DigitalOutpin pwrRelay(PWR_RELAY_PIN);
DigitalOutpin soundRelay(SOUND_RELAY_PIN);
DigitalOutpin debugLed1(DEBUG_LED1_PIN);
DigitalOutpin debugLed2(DEBUG_LED2_PIN);

AnalogInpin hallSensor(HALL_SENSOR_PIN);
AnalogInpin sw1(SW1_PIN);
AnalogInpin sw2(SW2_PIN);
AnalogInpin sw3(SW3_PIN);

byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetServer server(80);

char string[32];
char *p;
String suburl;

char buffer[10];

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    heartBeatLed.set(0);
    debugLed1.set(0);
    debugLed2.set(0);

    Serial.println("Ethernet WebServer");

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
    heartBeatLed.run();
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
                        Serial.println(p);
                        if (!strcmp(p, "/test"))
                        {
                            clientPrint(client, "This is test url");
                            Serial.println("test");
                        }
                        else if (!strcmp(p, "/pwrrelayon"))
                        {
                            clientPrint(client, "Power Relay is on");
                            Serial.println("PWR ON");
                            pwrRelay.set(0);
                            debugLed1.set(1);
                        }
                        else if (!strcmp(p, "/pwrrelayoff"))
                        {
                            clientPrint(client, "Power Relay is off");
                            Serial.println("PWR OFF");
                            pwrRelay.set(2);
                            debugLed1.set(0);
                        }
                        else if (!strcmp(p, "/soundrelayon"))
                        {
                            clientPrint(client, "Sound Relay is on");
                            Serial.println("SOUND ON");
                            soundRelay.set(0);
                            debugLed2.set(1);
                        }
                        else if (!strcmp(p, "/soundrelayoff"))
                        {
                            clientPrint(client, "Sound Relay is off");
                            Serial.println("SOUND OFF");
                            soundRelay.set(1);
                            debugLed2.set(0);
                        }
                        else if (!strcmp(p, "/status"))
                        {
                            itoa(hallSensor.get(), buffer, 10);
                            Serial.println(buffer);
                            clientPrint(client, buffer);
                        }
                        else
                        {
                            clientPrint(client, "Wrong URL");
                            Serial.println("Wrong URL");
                        }
                    }
                }
            }
        }

        Serial.println("client disconnected");
    }
}
