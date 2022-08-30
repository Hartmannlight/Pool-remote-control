// TODO Dictionary
// TODO Fix printBatteryState (wrong values because ESP32 internal resistor is too small)
// TODO Check the battery state continuously
// TODO Switch to better SSD1306 library
// TODO Build new Version with

// configs
#include <WifiConfig.h>
#include "Config.h"
#include "Icons.h"

// libraries
#include <WiFi.h>
#include "time.h"              // NTP
#include "Button2.h"           // debounce switch
#include "Wire.h"              // Display
#include "SSD1306.h"           // Display
#include <HTTPClient.h>        // OTA
#include <AsyncTCP.h>          // OTA
#include <ESPAsyncWebServer.h> // OTA
#include <AsyncElegantOTA.h>   // OTA

// virtual objects
HTTPClient http;
AsyncWebServer server(80);

// physical objects
Button2 leftButton;
Button2 rightButton;
SSD1306 display(0x3c, SDA, SCL);

// state Variables
int currentLocation = 4;      // State, which temperature is currently shown on the display
unsigned long lastUpdate = 0; // counter: refresh displayed values every x ms

void setup()
{
  // pinMode(BATTERY_PIN, INPUT); // TODO this might fix the wrong voltage on the voltage divider

  display.init();
  display.flipScreenVertically();

  WiFi.mode(WIFI_STA);
  WiFi.config(LOCAL_IP, GATEWAY, SUBNET, PRIMARY_DNS, SECONDARY_DNS); // use static IP --> no DHCP --> connection is established faster
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // printBatteryState();
  printConnectionState();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // NTP init
  printTime();

  leftButton.begin(LEFT_BUTTON_PIN);
  leftButton.setClickHandler(leftButtonClick);
  leftButton.setLongClickTime(400);

  rightButton.begin(RIGHT_BUTTON_PIN);
  rightButton.setClickHandler(rightButtonClick);
  rightButton.setLongClickTime(400);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", MQTT_CLIENT_NAME); });

  AsyncElegantOTA.begin(&server);
  server.begin();
}

void loop()
{
  leftButton.loop();
  rightButton.loop();
  delay(1); // Sometimes this prevents the ESP from heating up

  if (millis() >= lastUpdate + 5000)
  {
    if (currentLocation == numberOfStations)
    {
      printTime();
    }
    else
    {
      printTemperature(currentLocation);
    }
    lastUpdate = millis();
  }
}

void rightButtonClick(Button2 &btn)
{
  currentLocation = (currentLocation + 1) % 4;
  printTemperature(currentLocation);
}

void leftButtonClick(Button2 &btn)
{
  setFilterState(!getFilterState());
}

void printBatteryState()
{
  int average = 0;
  for (int i = 0; i < 500; i++) // 500 iterations because ESP32 ADCs are bad but also quit fast
  {
    average = average + analogRead(BATTERY_PIN);
  }
  average = average / 500;

  if (average <= BATTERY_SHUTDOWN_VALUE)
  {
    display.clear();
    display.drawXbm(0, 0, 128, 64, icon_deathBattery);
    display.display();
    delay(1500); // go to deep sleep instead
  }
  else if (average <= BATTERY_WARNING_VALUE)
  {
    display.clear();
    display.drawXbm(0, 0, 128, 64, icon_lowBattery);
    display.display();
    delay(1000);
  }
}

void printConnectionState()
{
  display.clear();
  display.drawXbm(0, 0, 128, 64, icon_wifi);
  display.display();

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(10);
  }
}

void printTime()
{
  struct tm timeInfo;

  if (!getLocalTime(&timeInfo))
  {
    return;
  }

  char timeHour[6];
  strftime(timeHour, 6, "%H:%M", &timeInfo); // 5 digits + 1 end of line ; %H:%M = 24h:minutes

  display.clear();
  display.setFont(Open_Sans_Hebrew_10);
  display.drawString(0, 0, "Zeit");
  display.setFont(Open_Sans_Hebrew_35);
  display.drawString(15, 12, timeHour);
  display.display();
}

void printTemperature(int stationNumber)
{
  http.begin(stationApiUrl[stationNumber]);
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    String payload = http.getString();
    char charBuf[5];
    payload.toCharArray(charBuf, 5);

    display.clear();
    display.setFont(Open_Sans_Hebrew_10);
    display.drawString(0, 0, stationLabel[stationNumber]);

    display.drawString(110, 0, getFilterState() ? "ON" : "OFF");

    display.setFont(Open_Sans_Hebrew_35);
    display.drawString(5, 12, charBuf);
    display.drawString(85, 12, "°C");

    display.display();
  }
  http.end();
}

bool getFilterState()
{
  http.begin(filterGetApiUrl);
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    String payload = http.getString();
    return payload.equals("ON");
  }
  http.end();
}

void setFilterState(bool state)
{
  if (http.begin(filterSetApiUrl))
  {
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("Accept", "application/json");

    int httpCode = http.POST(state ? "ON" : "OFF");

    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = http.getString(); // TODO remove?
      }
    }
    http.end();
  }
}
