
#include "IRremote.h"
#include <EtherCard.h>
#include <DHT.h>

#define DHTPIN 2 // PIN data of DHT22
#define DHTTYPE DHT11 // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

IRsend irsend;
static byte myip[] = { 192, 168, 4, 200 };
static byte gwip[] = { 192, 168, 4, 1 };
static byte mymac[] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };
byte Ethernet::buffer[500];
unsigned on[] = { 8972, 4592, 500, 612, 524, 616, 520, 620, 500, 612, 524, 616, 520, 1732, 524, 616, 520, 620, 496, 1756, 520, 1732, 524, 1756, 496, 1756, 520, 1732, 636, 504, 516, 1736, 524, 1752, 500, 616, 524, 1752, 524, 592, 524, 612, 524, 616, 500, 616, 548, 588, 524, 592, 524, 1752, 524, 616, 496, 1756, 520, 1732, 524, 1756, 548, 1704, 520, 1732, 524, 1756, 496, 40204, 9004, 2284, 524 };

const char page[] PROGMEM = "HTTP/1.0 200 OK\r\n"
                            "Content-Type: text/html\r\n"
                            "\r\n"
                            "%D|%D";
char strBuf[100];
void setup()
{
    Serial.begin(57600);
    if (ether.begin(sizeof Ethernet::buffer, mymac, 8) == 0)
        Serial.println("Failed to access Ethernet controller");
    if (!ether.dhcpSetup())
        ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);
    ether.printIp("DNS: ", ether.dnsip);
    pinMode(3, OUTPUT);
}

void loop()
{
    word pos = ether.packetLoop(ether.packetReceive());
    if (pos) {
        char* data = (char*)Ethernet::buffer + pos;
        if (strncmp("GET /stat ", data, 10) == 0) {
            int t = dht.readTemperature();
            int h = dht.readHumidity();
            sprintf(strBuf, sizeof(strBuf), page, t, h);
            memcpy_P(ether.tcpOffset(), strBuf, sizeof(strBuf));
            ether.httpServerReply(sizeof strBuf - 1);
        }

        if (strncmp("GET /onoff ", data, 11) == 0) {
            irsend.sendRaw(on, sizeof(on) / sizeof(int), 38);
            memcpy_P(ether.tcpOffset(), page, sizeof(page));
            ether.httpServerReply(sizeof page - 1);
        }
    }
}
