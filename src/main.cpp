#include <Arduino.h>
#include <Lora.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include "boards.h"
#include "math.h"

WebServer server(80);

int const nodes = 3;

int ids[nodes] = {0, 0, 0};
int longitudes[nodes];
int latitudes[nodes];
int times[nodes];
double distances[nodes];
int rssis[nodes];

int id = 0;
int type = 0;
int longitude = 0;
int latitude = 0;

double calculateDistance(double rssi)
{
    return pow(10, -rssi / 10 * 2);
}

void onReceive(int size)
{
    if (size != 0)
    {
        double rssi = LoRa.packetRssi();

        double distance = calculateDistance(rssi);

        int receivedId = LoRa.readStringUntil(';').toInt();

        int index = 0;

        for (int i = 0; nodes < i; i++)
        {
            if (ids[i] == receivedId)
            {
                index = i;

                break;
            }

            else if (ids[i] == 0)
            {
                index = i;

                ids[index] = receivedId;

                break;
            }
        }

        distances[index] = distance;
        rssis[index] = rssi;
        longitudes[index] = LoRa.readStringUntil(';').toInt();
        latitudes[index] = LoRa.readStringUntil(';').toInt();

        Serial.println("----");
        Serial.println("Result!");

        Serial.println(LoRa.readStringUntil(';'));

        Serial.println();
    }
}

void sendMessage(String message)
{
    String outgoing = String(id) + ";" +
                      String(longitude) + ";" +
                      String(latitude) + ";" +
                      message;

    LoRa.beginPacket();

    LoRa.write(outgoing.length());
    LoRa.print(outgoing);

    LoRa.endPacket();

    LoRa.receive();
}

void onId()
{
    server.send(200, "text/html", String(id));
}

void onLat()
{
    server.send(200, "text/html", String(latitude));
}

void onLon()
{
    server.send(200, "text/html", String(longitude));
}

void onType()
{
    server.send(200, "text/html", String(type));
}

void onUpdate()
{
    String data = server.arg(0);

    int index1 = 0;
    int index2 = data.indexOf(';', index1);
    id = data.substring(index1, index2).toInt();

    index1 = index2 + 1;
    index2 = data.indexOf(';', index1);
    longitude = data.substring(index1, index2).toInt();

    index1 = index2 + 1;
    index2 = data.indexOf(';', index1);
    latitude = data.substring(index1, index2).toInt();

    index1 = index2 + 1;
    index2 = data.indexOf(';', index1);
    type = data.substring(index1, index2).toInt();

    server.send(200);
}

void onMessage()
{
    sendMessage(server.arg(0));

    server.send(200);
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

    delay(1500);

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
    server.on("/id", onId);
    server.on("/lat", onLat);
    server.on("/lon", onLon);
    server.on("/type", onType);
    server.on("/update", HTTP_POST, onUpdate);
    server.on("/message", HTTP_POST, onMessage);
    server.onNotFound(onNotFound);
    server.begin();

    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);

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
    server.handleClient();

    delay(1000);
}