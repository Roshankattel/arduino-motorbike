
#include "config.h"

#ifdef UNO_BOARD
// #include "avr8-stub.h"
// #include "app_api.h"

#include <SPI.h>
#include <WiFiNINA.h>
#include <Servo.h>

int status = WL_IDLE_STATUS;

void printEncryptionType(int thisType)
{
    // read the encryption type and print out the name:
    switch (thisType)
    {
    case ENC_TYPE_WEP:
        Serial.println("WEP");
        break;
    case ENC_TYPE_TKIP:
        Serial.println("WPA");
        break;
    case ENC_TYPE_CCMP:
        Serial.println("WPA2");
        break;
    case ENC_TYPE_NONE:
        Serial.println("None");
        break;
    case ENC_TYPE_AUTO:
        Serial.println("Auto");
        break;
    case ENC_TYPE_UNKNOWN:
    default:
        Serial.println("Unknown");
        break;
    }
}

void printMacAddress(byte mac[])
{
    for (int i = 5; i >= 0; i--)
    {
        if (mac[i] < 16)
        {
            Serial.print("0");
        }
        Serial.print(mac[i], HEX);
        if (i > 0)
        {
            Serial.print(":");
        }
    }
    Serial.println();
}

#else
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#endif

int led = LED_BUILTIN;

WiFiServer server(80);
Servo servoArm[3];

int armValues[3] = {90, 90, 90};

bool forward = false;
bool backward = false;

uint64_t bikeUpTime = 0;

int Speed = 200;

void printWifiStatus();
void setServo(uint8_t index, int change);
void bikeForward(void);
void bikeBackward(void);
void bikeStop(void);

void setup()
{
    // Initialize serial and wait for port to open:
    Serial.begin(115200);
    Serial.println("WiFi Scanning test");
    pinMode(led, OUTPUT);

    servoArm[SERVO_LEFT].attach(LEFT_ARM);
    servoArm[SERVO_MID].attach(MID_ARM);
    servoArm[SERVO_RIGHT].attach(RIGHT_ARM);

    servoArm[SERVO_LEFT].write(armValues[SERVO_LEFT]);
    servoArm[SERVO_MID].write(armValues[SERVO_MID]);
    servoArm[SERVO_RIGHT].write(armValues[SERVO_RIGHT]);

    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    // pinMode(ENB, OUTPUT);
    //  pinMode(IN3, OUTPUT);
    //  pinMode(IN4, OUTPUT);

#ifdef UNO_BOARD
    // Set up the pins!
    WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);

    // check for the WiFi module:
    while (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");
        // don't continue
        delay(1000);
    }
    String fv = WiFi.firmwareVersion();
    Serial.println(fv);
    if (fv < "1.0.0")
    {
        Serial.println("Please upgrade the firmware");
        while (1)
            delay(10);
    }
    Serial.println("Firmware OK");
    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    printMacAddress(mac);

    status = WiFi.beginAP(WIFI_SSID, WIFI_PASS);
    if (status != WL_AP_LISTENING)
    {
        Serial.println("Creating access point failed");
        // don't continue
        while (true)
            ;
    }
    server.begin();
    printWifiStatus();
#else
    // WiFi.softAP(WIFI_SSID, WIFI_PASS, 1, true, 4);//Hidden
    WiFi.softAP(WIFI_SSID, WIFI_PASS, 1, false, 4);
    Serial.println("Soft AP started");
    server.begin();
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
#endif
    Serial.println("HTTP server started");
}
void loop()
{
    WiFiClient client = server.available(); // listen for incoming clients
    if (client)
    {
        // Serial.println("new client"); // print a message out the serial port
        String currentLine = ""; // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                // Serial.write(c);        // print it out the serial monitor
                if (c == '\n')
                { // if the byte is a newline character

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        client.println(F("HTTP/1.1 200 OK"));
                        client.println(F("Content-type:text/html"));
                        client.println();
                        client.println(F("<head>"));
                        client.println(F("<title>Arduino MotorBike</title>"));
                        client.println(F("<style>"));
                        client.write(webCSS, strlen(webCSS));
                        client.println(F("</style>"));
                        client.println(F("</head>"));
                        client.println("<body style=background:#F0FFFF>");

                        // Title
                        client.println(F("<a href=\"/\">"));
                        client.println(F("<h1><center><b>Arduino <span style='color:red'>MotorBike</span></b></center></h1>"));
                        client.println(F("</a><hr>"));

                        // Movement buttons
                        client.println(F("<center>"));
                        client.println(F("<h1><b>Mid Arm:"));
                        client.print(armValues[SERVO_MID]);
                        client.println(F("&deg;</b></h1>"));
                        client.println(F("<a href=\"/front\"><button class='custom-button'>&#8593;<br>FORWARD</button></a><br><br>"));
                        client.println(F("<a href=\"/mid-left\"><button class='custom-button custom-blue'>&#8592;&nbsp;LEFT&nbsp;</button></a>"));
                        client.println(F("<a href=\"/mid-right\"><button class='custom-button custom-blue'>&nbsp;RIGHT&nbsp;&#8594;</button></a><br><br>"));
                        client.println(F("<a href=\"/back\"><button class='custom-button'>BACKWARD<br>&#8595;</button></a>"));
                        client.println(F("</center>"));

                        // Arm controls
                        client.println(F("<div style='display: flex; justify-content: space-between;'>"));
                        client.println(F("<div style='text-align: left;'>"));
                        client.println(F("<h1><b>Left Arm:"));
                        client.print(armValues[SERVO_LEFT]);
                        client.println(F("&deg;</b></h1>"));
                        client.println(F("<a href=\"/l1-left\"><button class='custom-button custom-blue'>STEP DOWN</button></a>"));
                        client.println(F("<a href=\"/l1-right\"><button class='custom-button custom-blue'>STEP UP</button></a>"));
                        client.println(F("</div>"));
                        client.println(F("<div style='text-align: right;'>"));
                        client.println(F("<h1><b>Right Arm:"));
                        client.print(armValues[SERVO_RIGHT]);
                        client.println(F("&deg;</b></h1>"));
                        client.println(F("<a href=\"/l2-left\"><button class='custom-button custom-blue'>STEP DOWN</button></a>"));
                        client.println(F("<a href=\"/l2-right\"><button class='custom-button custom-blue'>STEP UP</button></a>"));
                        client.println(F("</div>"));
                        client.println(F("</div>"));

                        client.println(F("</body>"));
                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }

                // Check to see if the client request was "GET /H" or "GET /L":
                if (currentLine.endsWith("GET /front"))
                {
                    bikeForward();
                }
                else if (currentLine.endsWith("GET /back"))
                {
                    bikeBackward();
                }
                else if (currentLine.endsWith("GET /mid-left"))
                {
                    setServo(SERVO_MID, -10);
                }
                else if (currentLine.endsWith("GET /mid-right"))
                {
                    setServo(SERVO_MID, 10);
                }
                else if (currentLine.endsWith("GET /l1-left"))
                {
                    setServo(SERVO_LEFT, -30);
                }
                else if (currentLine.endsWith("GET /l1-right"))
                {
                    setServo(SERVO_LEFT, 30);
                }
                else if (currentLine.endsWith("GET /l2-left"))
                {
                    setServo(SERVO_RIGHT, -30);
                }
                else if (currentLine.endsWith("GET /l2-right"))
                {
                    setServo(SERVO_RIGHT, 30);
                }
            }
        }
        // close the connection:
        client.stop();
        // Serial.println("client disconnected");
    }
    if (millis() - bikeUpTime >= BIKE_RUN_TIME * 1000)
        bikeStop();
}

void setServo(uint8_t index, int change)
{
    armValues[index] = armValues[index] + change;

    if (index == SERVO_MID)
    {
        if (armValues[index] > 180)
            armValues[index] = 180;
    }
    else
    {
        if (armValues[index] > 360)
            armValues[index] = 360;
    }
    if (armValues[index] < 0)
        armValues[index] = 0;

    servoArm[index].write(armValues[index]);
}

void bikeForward(void)
{
    Serial.println("Moving Forwward:");
    analogWrite(ENA, Speed);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    bikeUpTime = millis();
}

void bikeBackward(void)
{
    Serial.println("Moving Backward:");
    analogWrite(ENA, Speed);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    bikeUpTime = millis();
}

void bikeStop(void)
{
    digitalWrite(ENA, LOW);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    bikeUpTime = 0;
}

void printWifiStatus(void)
{
    // print the WIFI_SSID of the network you're attached to:
    Serial.print("WIFI_SSID: ");
    Serial.println(WiFi.SSID());

    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
    // print where to go in a browser:
    Serial.print("To see this page in action, open a browser to http://");
    Serial.println(ip);
}