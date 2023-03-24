#include <Arduino.h>
#include <Lora.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include "boards.h"
#include "math.h"

#define LoRa_signal_bandwidth 125E3
#define LoRa_spreading_factor 7

#define c 299792458 // Speed of light in m/s
#define Gt 1        // Transmitter antenna gain in dBi
#define Gr 1        // Receiver antenna gain in dBi

WebServer server(80);

int interval = 1000;
int lastSendTime = 0;
int msgCount = 0;

// fiis
double distance(double rssi, double snr)
{
    return pow(10, ((-45 - rssi) / (10 * 2)));
}

void onReceive(int size)
{
    if (size != 0)
    {
        double rssi = LoRa.packetRssi();
        double snr = LoRa.packetSnr();

        Serial.println(LoRa.readString());

        Serial.println(rssi);
        Serial.println(snr);

        Serial.println("Result!");
        Serial.println(distance(rssi, snr));
    }
}

void sendMessage(String outgoing)
{
    LoRa.beginPacket();
    // LoRa.write(destination);
    // LoRa.write(localAddress);
    LoRa.write(msgCount);
    LoRa.write(outgoing.length());
    LoRa.print(outgoing);
    LoRa.endPacket();
    msgCount++;
}

void onIndex()
{
    File f = SPIFFS.open("/web/index.html", "r");

    server.send(200, "text/html", f.readString());

    f.close();
}

void onNotFound()
{
    server.send(404, "text/html", "Not Found!");
}

void setup()
{
    initBoard();

    if (!SPIFFS.begin())
    {
        Serial.println("File System error");
    }

    File config = SPIFFS.open("/config", "r");

    String ssid = config.readStringUntil('\n');
    String pass = config.readStringUntil('\n');

    config.close();

    ssid.trim();
    pass.trim();

    WiFi.begin(ssid.c_str(), pass.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("/");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", onIndex);
    server.onNotFound(onNotFound);
    server.begin();

    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);

    LoRa.setSignalBandwidth(LoRa_signal_bandwidth);
    LoRa.setSpreadingFactor(LoRa_spreading_factor);

    if (LoRa.begin(LoRa_frequency))
    {
        Serial.println("LoRa OK!");
        LoRa.onReceive(onReceive);
        LoRa.receive();
    }

    else
    {
        Serial.println("Starting LoRa failed!");
    }
}

void loop()
{
    /*if (millis() - lastSendTime > interval)
    {
        String message = "HeLoRa World!";
        sendMessage(message);
        Serial.println("Sending " + message);
        lastSendTime = millis();
        interval = random(2000) + 1000;
        LoRa.receive();
    }

    else
    {
        server.handleClient();
    }*/
}