#define BATTERY_PIN 27
#define LEFT_BUTTON_PIN 33
#define RIGHT_BUTTON_PIN 32
#define SDA 21
#define SCL 22

#define BATTERY_WARNING_VALUE 2900 // 3,6 Volt
#define BATTERY_SHUTDOWN_VALUE 2700 // 3,4 Volt

const char* MQTT_CLIENT_NAME = "PoolAnzeige";
IPAddress LOCAL_IP(192, 168, 0, 101);


// NTP Server info
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 2*3600;

// Stations
// You can get the links from http://192.168.0.10:8080/developer/api-explorer
int numberOfStations = 4;
String filterGetApiUrl = "http://192.168.0.10:8080/rest/items/Pool_Filteranlage/state";
String filterSetApiUrl = "http://192.168.0.10:8080/rest/items/Pool_Filteranlage";
String stationApiUrl[] = {"http://192.168.0.10:8080/rest/items/Pool_Wassertemperatur/state",
                   "http://192.168.0.10:8080/rest/items/HofStation_Temperatur/state",
                   "http://192.168.0.10:8080/rest/items/Pool_Aussentemperatur/state",
                   "http://192.168.0.10:8080/rest/items/WohnzimmerStation_Temperatur/state"};

char* stationLabel[] = {"Pool",
                  "Hof",
                  "Einfahrt",
                  "Wohnzimmer"};
