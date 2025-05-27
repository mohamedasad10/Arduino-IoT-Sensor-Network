#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>

// Replace with your network credentials (STATION)
const char* ssid = "4x4V1";
const char* password = "AD@NATe018949";

// UI Team HTMl here
const char* index_html = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP-NOW Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>ESP-NOW Sensor Dashboard</h1>
  <p>Board Data will be displayed here</p>
</body>
</html>
)rawliteral";

// Structure must match sender, packed to avoid alignment issues
typedef struct __attribute__((packed)) struct_message {
    int id;
    float temperature;
    float humidity;
    int light;
    int readingId;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;
struct_message board4;

JSONVar board;

AsyncWebServer server(80);
AsyncEventSource events("/events");

// Create an array with all the structures
struct_message boardsStruct[4] = {board1, board2, board3, board4};

// Callback when data is received (updated for newer ESP32 Arduino core)
void OnDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len) {
  // Check if received data size matches expected structure size
  if (len != sizeof(struct_message)) {
    Serial.println("Error: Received data size mismatch");
    return;
  }

  memcpy(&myData, incomingData, sizeof(myData));

  // Validate board ID to prevent array out-of-bounds
  if (myData.id < 1 || myData.id > 4) {
    Serial.printf("Error: Invalid board ID %d\n", myData.id);
    return;
  }

  // Update the corresponding board structure
  boardsStruct[myData.id-1].id = myData.id;
  boardsStruct[myData.id-1].temperature = myData.temperature;
  boardsStruct[myData.id-1].humidity = myData.humidity;
  boardsStruct[myData.id-1].light = myData.light;
  boardsStruct[myData.id-1].readingId = myData.readingId;

  Serial.println("=== Data Received ===");
  // Access MAC address from recv_info->src_addr
  const uint8_t *mac_addr = recv_info->src_addr; // Fixed typo: changed 'recvcave_info' to 'recv_info'
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Packet received from: ");
  Serial.println(macStr);

  // Print data with correct format specifiers
  Serial.printf("Board ID: %d \t", boardsStruct[myData.id-1].id);
  Serial.printf("Reading ID: %d \n", boardsStruct[myData.id-1].readingId);
  Serial.printf("Temperature: %.2f C\t", boardsStruct[myData.id-1].temperature);
  Serial.printf("Humidity: %.2f %RH\t", boardsStruct[myData.id-1].humidity);
  Serial.printf("Light: %d \n", boardsStruct[myData.id-1].light);
  Serial.println("=====================");
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ok......");

  // Register receive callback
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
   
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
}

void loop() {
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping", NULL, millis());
    lastEventTime = millis();
  }
}