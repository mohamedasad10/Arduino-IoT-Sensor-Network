#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>

// Replace with your network credentials (STATION)
const char* ssid = "4x4V1";
const char* password = "AD@NATe018949";

// UI Team HTMl here
const char* index_html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP-NOW Sensor Dashboard</title>
    <link rel="icon" href="data:,">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: #333;
        }

        .header {
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            border-bottom: 1px solid rgba(255, 255, 255, 0.2);
            padding: 1rem 2rem;
            text-align: center;
            box-shadow: 0 4px 20px rgba(0, 0, 0, 0.1);
        }

        .header h1 {
            color: white;
            font-size: 2.5rem;
            font-weight: 700;
            text-shadow: 0 2px 10px rgba(0, 0, 0, 0.3);
            margin-bottom: 0.5rem;
        }

        .connection-status {
            display: inline-flex;
            align-items: center;
            gap: 0.5rem;
            background: rgba(255, 255, 255, 0.2);
            padding: 0.5rem 1rem;
            border-radius: 25px;
            color: white;
            font-size: 0.9rem;
            margin-top: 0.5rem;
        }

        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: #ff4757;
            animation: pulse 2s infinite;
        }

        .status-dot.connected {
            background: #2ed573;
        }

        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }

        .dashboard-container {
            max-width: 1400px;
            margin: 2rem auto;
            padding: 0 1rem;
        }

        .boards-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 2rem;
            margin-bottom: 2rem;
        }

        .board-card {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(20px);
            border-radius: 20px;
            padding: 2rem;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
            border: 1px solid rgba(255, 255, 255, 0.2);
            transition: all 0.3s ease;
            position: relative;
            overflow: hidden;
        }

        .board-card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 4px;
            background: linear-gradient(90deg, #667eea, #764ba2);
        }

        .board-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 12px 40px rgba(0, 0, 0, 0.15);
        }

        .board-header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 1.5rem;
        }

        .board-title {
            font-size: 1.5rem;
            font-weight: 700;
            color: #2c3e50;
        }

        .board-id {
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
            padding: 0.5rem 1rem;
            border-radius: 12px;
            font-size: 0.9rem;
            font-weight: 600;
        }

        .sensors-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(100px, 1fr));
            gap: 1rem;
        }

        .sensor-item {
            text-align: center;
            padding: 1rem;
            background: rgba(255, 255, 255, 0.5);
            border-radius: 15px;
            transition: all 0.3s ease;
        }

        .sensor-item:hover {
            background: rgba(255, 255, 255, 0.8);
            transform: scale(1.02);
        }

        .sensor-icon {
            font-size: 2rem;
            margin-bottom: 0.5rem;
        }

        .sensor-icon.temperature { color: #ff6b6b; }
        .sensor-icon.humidity { color: #4ecdc4; }
        .sensor-icon.light { color: #ffe66d; }

        .sensor-value {
            font-size: 1.8rem;
            font-weight: 700;
            margin-bottom: 0.2rem;
            color: #2c3e50;
        }

        .sensor-label {
            font-size: 0.8rem;
            color: #7f8c8d;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 0.5rem;
        }

        .reading-id {
            font-size: 0.7rem;
            color: #95a5a6;
            padding: 0.2rem 0.5rem;
            background: rgba(149, 165, 166, 0.1);
            border-radius: 8px;
            display: inline-block;
        }

        .no-data {
            color: #95a5a6;
            font-style: italic;
        }

        .last-update {
            text-align: center;
            margin-top: 2rem;
            color: rgba(255, 255, 255, 0.8);
            font-size: 0.9rem;
        }

        @media (max-width: 768px) {
            .header h1 {
                font-size: 2rem;
            }
            
            .dashboard-container {
                margin: 1rem auto;
                padding: 0 0.5rem;
            }
            
            .boards-grid {
                grid-template-columns: 1fr;
                gap: 1rem;
            }
            
            .board-card {
                padding: 1.5rem;
            }
            
            .sensors-grid {
                grid-template-columns: 1fr;
                gap: 0.8rem;
            }
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }

        .sensor-item.updated {
            animation: fadeIn 0.5s ease;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>ESP-NOW Sensor Dashboard</h1>
        <div class="connection-status">
            <div class="status-dot" id="statusDot"></div>
            <span id="connectionStatus">Connecting...</span>
        </div>
    </div>

    <div class="dashboard-container">
        <div class="boards-grid" id="boardsContainer">
            <!-- Board cards will be dynamically generated -->
        </div>
        
        <div class="last-update">
            Last Update: <span id="lastUpdateTime">Never</span>
        </div>
    </div>

    <script>
        class Dashboard {
            constructor() {
                this.boards = new Map();
                this.eventSource = null;
                this.init();
            }

            init() {
                this.connectEventSource();
                this.createBoardCards();
            }

            connectEventSource() {
                if (!!window.EventSource) {
                    this.eventSource = new EventSource('/events');

                    this.eventSource.addEventListener('open', (e) => {
                        console.log("Events Connected");
                        this.updateConnectionStatus(true);
                    });

                    this.eventSource.addEventListener('error', (e) => {
                        if (e.target.readyState != EventSource.OPEN) {
                            console.log("Events Disconnected");
                            this.updateConnectionStatus(false);
                        }
                    });

                    this.eventSource.addEventListener('new_readings', (e) => {
                        console.log("new_readings", e.data);
                        try {
                            const data = JSON.parse(e.data);
                            this.updateBoardData(data);
                            this.updateLastUpdateTime();
                        } catch (error) {
                            console.error("Error parsing sensor data:", error);
                        }
                    });
                } else {
                    console.error("Your browser doesn't support Server-Sent Events");
                    document.getElementById('connectionStatus').textContent = 'Browser not supported';
                }
            }

            updateConnectionStatus(connected) {
                const statusDot = document.getElementById('statusDot');
                const statusText = document.getElementById('connectionStatus');
                
                if (connected) {
                    statusDot.classList.add('connected');
                    statusText.textContent = 'Connected';
                } else {
                    statusDot.classList.remove('connected');
                    statusText.textContent = 'Disconnected';
                }
            }

            createBoardCards() {
                const container = document.getElementById('boardsContainer');
                
                // Create cards for boards 1-4
                for (let i = 1; i <= 4; i++) {
                    const boardCard = this.createBoardCard(i);
                    container.appendChild(boardCard);
                }
            }

            createBoardCard(boardId) {
                const card = document.createElement('div');
                card.className = 'board-card';
                card.id = `board-${boardId}`;
                
                card.innerHTML = `
                    <div class="board-header">
                        <div class="board-title">Sensor Board</div>
                        <div class="board-id">Board #${boardId}</div>
                    </div>
                    <div class="sensors-grid">
                        <div class="sensor-item">
                            <div class="sensor-icon temperature">🌡️</div>
                            <div class="sensor-value" id="temp-${boardId}">--</div>
                            <div class="sensor-label">Temperature (°C)</div>
                            <div class="reading-id" id="temp-reading-${boardId}">No data</div>
                        </div>
                        <div class="sensor-item">
                            <div class="sensor-icon humidity">💧</div>
                            <div class="sensor-value" id="humidity-${boardId}">--</div>
                            <div class="sensor-label">Humidity (%)</div>
                            <div class="reading-id" id="humidity-reading-${boardId}">No data</div>
                        </div>
                        <div class="sensor-item">
                            <div class="sensor-icon light">💡</div>
                            <div class="sensor-value" id="light-${boardId}">--</div>
                            <div class="sensor-label">Light (lx)</div>
                            <div class="reading-id" id="light-reading-${boardId}">No data</div>
                        </div>
                    </div>
                `;
                
                return card;
            }

            updateBoardData(data) {
                const { id, temperature, humidity, light, readingId } = data;
                
                console.log("Received data:", data); // Debug log
                
                // Validate board ID
                if (id < 1 || id > 4) {
                    console.warn(`Invalid board ID: ${id}`);
                    return;
                }

                // Update temperature (check for valid number, including 0)
                if (temperature !== undefined && temperature !== null && !isNaN(temperature)) {
                    const tempElement = document.getElementById(`temp-${id}`);
                    const tempReadingElement = document.getElementById(`temp-reading-${id}`);
                    if (tempElement && tempReadingElement) {
                        tempElement.textContent = temperature.toFixed(1);
                        tempReadingElement.textContent = `Reading #${readingId}`;
                    }
                }

                // Update humidity (check for valid number, including 0)
                if (humidity !== undefined && humidity !== null && !isNaN(humidity)) {
                    const humidityElement = document.getElementById(`humidity-${id}`);
                    const humidityReadingElement = document.getElementById(`humidity-reading-${id}`);
                    if (humidityElement && humidityReadingElement) {
                        humidityElement.textContent = humidity.toFixed(1);
                        humidityReadingElement.textContent = `Reading #${readingId}`;
                    }
                }

                // Update light (check for valid number, including 0)
                if (light !== undefined && light !== null && !isNaN(light)) {
                    const lightElement = document.getElementById(`light-${id}`);
                    const lightReadingElement = document.getElementById(`light-reading-${id}`);
                    if (lightElement && lightReadingElement) {
                        lightElement.textContent = light;
                        lightReadingElement.textContent = `Reading #${readingId}`;
                    }
                }

                // Store board data
                this.boards.set(id, data);
            }

            animateUpdate(element) {
                element.classList.remove('updated');
                setTimeout(() => {
                    element.classList.add('updated');
                }, 10);
            }

            updateLastUpdateTime() {
                const now = new Date();
                const timeString = now.toLocaleTimeString();
                document.getElementById('lastUpdateTime').textContent = timeString;
            }
        }

        // Initialize dashboard when page loads
        document.addEventListener('DOMContentLoaded', () => {
            new Dashboard();
        });
    </script>
</body>
</html>)rawliteral";

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
  if (len != sizeof(struct_message)) {
    Serial.println("Error: Received data size mismatch");
    return;
  }

  memcpy(&myData, incomingData, sizeof(myData));

  if (myData.id < 1 || myData.id > 4) {
    Serial.printf("Error: Invalid board ID %d\n", myData.id);
    return;
  }

  boardsStruct[myData.id-1] = myData;

  // Serialize myData to JSON string
  JSONVar jsonData;
  jsonData["id"] = myData.id;
  jsonData["temperature"] = myData.temperature;
  jsonData["humidity"] = myData.humidity;
  jsonData["light"] = myData.light;
  jsonData["readingId"] = myData.readingId;

  String jsonString = JSON.stringify(jsonData);

  // Send SSE event named "new_readings" with JSON data
  events.send(jsonString.c_str(), "new_readings", millis());

  // (Optional) Debug prints here...
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