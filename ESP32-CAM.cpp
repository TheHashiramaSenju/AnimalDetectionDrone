
// Required Libraries
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_system.h"

// Pin Definitions for AI Thinker ESP32-CAM module
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Status LED (built-in flash LED on GPIO 4)
#define FLASH_LED_PIN     4

// UART configuration
#define UART_BAUD_RATE    115200
#define SERIAL_SIZE_RX    1024
#define SERIAL_SIZE_TX    1024

// WiFi credentials - REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// Web server port
WebServer server(80);

// Variables for command processing
String receivedCommand = "";
bool commandComplete = false;
bool cameraInitialized = false;
bool isStreaming = false;

// Function prototypes
bool initCamera();
void setupWiFi();
void handleRoot();
void handleCapture();
void handleStream();
void handleNotFound();
void processCommand(String command);
void flashLED(int numFlashes, int flashDelay);
void sendResponse(String response);

void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Configure flash LED
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  // Start serial communication
  Serial.begin(UART_BAUD_RATE);
  Serial.setRxBufferSize(SERIAL_SIZE_RX);
  Serial.setTxBufferSize(SERIAL_SIZE_TX);
  Serial.println("\nESP32-CAM initialization starting...");

  // Initialize camera
  if (initCamera()) {
    flashLED(2, 100); // Success indication
    Serial.println("Camera initialized successfully");
    cameraInitialized = true;
  } else {
    flashLED(5, 100); // Error indication
    Serial.println("Camera initialization failed");
  }

  // Set up WiFi
  setupWiFi();

  // Configure web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/stream", HTTP_GET, handleStream);
  server.onNotFound(handleNotFound);

  // Start web server
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("ESP32-CAM ready for commands");

  // Double flash to indicate setup complete
  flashLED(2, 200);
}

void loop() {
  // Handle web server clients
  server.handleClient();
 
  // Handle serial commands
  while (Serial.available() > 0) {
    char inChar = (char)Serial.read();
   
    // Add character to command buffer
    if (inChar != '\n' && inChar != '\r') {
      receivedCommand += inChar;
    } else if (inChar == '\n') {
      // Process command on newline
      commandComplete = true;
    }
  }
 
  // Process completed command
  if (commandComplete) {
    processCommand(receivedCommand);
    receivedCommand = "";
    commandComplete = false;
  }
 
  // Add a small delay to prevent CPU hogging
  delay(10);
}

bool initCamera() {
  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
 
  // PSRAM check (for high-res images)
  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;  // FRAMESIZE_UXGA for higher resolution
    config.jpeg_quality = 10;
    config.fb_count = 2;
    Serial.println("PSRAM found and enabled");
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    Serial.println("No PSRAM detected");
  }
 
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return false;
  }
 
  // Set initial camera parameters
  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 = No Effect, 1 = Negative, 2 = Grayscale, 3 = Red Tint, 4 = Green Tint, 5 = Blue Tint, 6 = Sepia
  s->set_whitebal(s, 1);       // 0 = disable, 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable, 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - auto, sunny, cloudy, office, home
  s->set_exposure_ctrl(s, 1);  // 0 = disable, 1 = enable
  s->set_aec2(s, 0);           // 0 = disable, 1 = enable
  s->set_gain_ctrl(s, 1);      // 0 = disable, 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable, 1 = enable
  s->set_wpc(s, 1);            // 0 = disable, 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable, 1 = enable
  s->set_lenc(s, 1);           // 0 = disable, 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable, 1 = enable
  s->set_vflip(s, 0);          // 0 = disable, 1 = enable
  s->set_dcw(s, 1);            // 0 = disable, 1 = enable
 
  return true;
}

void setupWiFi() {
  Serial.println("Connecting to WiFi...");
 
  // Connect to WiFi network
  WiFi.begin(ssid, password);
 
  // Wait for connection with timeout
  int connectionAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;
  }
 
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
   
    // Set up mDNS responder
    if (MDNS.begin("esp32cam")) {
      Serial.println("mDNS responder started");
    }
  } else {
    Serial.println("");
    Serial.println("WiFi connection failed");
  }
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>ESP32-CAM Control</h1>";
  html += "<p>Camera Status: " + String(cameraInitialized ? "Ready" : "Not Initialized") + "</p>";
  html += "<p><a href=\"/capture\">Take Photo</a></p>";
  html += "<p><a href=\"/stream\">Start Stream</a></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleCapture() {
  if (!cameraInitialized) {
    server.send(503, "text/plain", "Camera not initialized");
    return;
  }
 
  // Take a photo
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }
 
  // Send the photo
  server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
 
  // Return the frame buffer back to the camera driver
  esp_camera_fb_return(fb);
 
  Serial.println("Photo captured and sent");
}

void handleStream() {
  if (!cameraInitialized) {
    server.send(503, "text/plain", "Camera not initialized");
    return;
  }
 
  // Simple streaming page with JavaScript to refresh image
  String html = "<html><body>";
  html += "<h1>ESP32-CAM Stream</h1>";
  html += "<img src=\"/capture\" id=\"stream\" width=\"640\" height=\"480\">";
  html += "<script>";
  html += "var img = document.getElementById('stream');";
  html += "function updateImage() {";
  html += "  img.src = '/capture?' + new Date().getTime();";
  html += "  setTimeout(updateImage, 1000);";
  html += "}";
  html += "updateImage();";
  html += "</script>";
  html += "</body></html>";
 
  server.send(200, "text/html", html);
  Serial.println("Stream page served");
  isStreaming = true;
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void processCommand(String command) {
  Serial.println("Received command: " + command);
 
  // Convert to uppercase and trim
  command.trim();
  command.toUpperCase();
 
  if (command == "RESET") {
    sendResponse("Resetting ESP32-CAM...");
    delay(100);
    ESP.restart();
  }
  else if (command == "STATUS") {
    String status = "Camera: " + String(cameraInitialized ? "OK" : "FAIL");
    status += ", WiFi: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    status += ", IP: " + WiFi.localIP().toString();
    sendResponse(status);
  }
  else if (command == "PHOTO") {
    if (!cameraInitialized) {
      sendResponse("ERROR: Camera not initialized");
      return;
    }
   
    // Take photo
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      sendResponse("ERROR: Camera capture failed");
      return;
    }
   
    // Flash LED to indicate photo taken
    flashLED(1, 100);
   
    // For UART communication, we could implement a protocol to transfer the image
    // Here, we'll just acknowledge the capture
    sendResponse("PHOTO READY");
   
    // Return the frame buffer back to the camera driver
    esp_camera_fb_return(fb);
  }
  else if (command.startsWith("RES:")) {
    if (!cameraInitialized) {
      sendResponse("ERROR: Camera not initialized");
      return;
    }
   
    // Extract resolution parameter
    String resolution = command.substring(4);
    framesize_t frame_size = FRAMESIZE_VGA;
   
    // Set resolution based on parameter
    if (resolution == "QVGA") frame_size = FRAMESIZE_QVGA;
    else if (resolution == "VGA") frame_size = FRAMESIZE_VGA;
    else if (resolution == "SVGA") frame_size = FRAMESIZE_SVGA;
    else if (resolution == "XGA") frame_size = FRAMESIZE_XGA;
    else if (resolution == "HD") frame_size = FRAMESIZE_HD;
    else if (resolution == "SXGA") frame_size = FRAMESIZE_SXGA;
    else if (resolution == "UXGA") frame_size = FRAMESIZE_UXGA;
    else {
      sendResponse("ERROR: Invalid resolution");
      return;
    }
   
    // Apply new resolution
    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, frame_size);
    sendResponse("Resolution set to " + resolution);
  }
  else if (command == "LED_ON") {
    digitalWrite(FLASH_LED_PIN, HIGH);
    sendResponse("LED turned ON");
  }
  else if (command == "LED_OFF") {
    digitalWrite(FLASH_LED_PIN, LOW);
    sendResponse("LED turned OFF");
  }
  else if (command == "WIFI_INFO") {
    String info = "SSID: " + String(ssid);
    info += ", Status: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    info += ", IP: " + WiFi.localIP().toString();
    info += ", RSSI: " + String(WiFi.RSSI()) + " dBm";
    sendResponse(info);
  }
  else if (command.startsWith("WIFI_CONFIG:")) {
    // Format: WIFI_CONFIG:SSID,PASSWORD
    int commaIndex = command.indexOf(',', 12);  // Start searching after "WIFI_CONFIG:"
    if (commaIndex > 0) {
      String newSSID = command.substring(12, commaIndex);
      String newPassword = command.substring(commaIndex + 1);
     
      // Save to global variables (would need to save to EEPROM/SPIFFS for persistence)
      // This implementation just changes the current session
      sendResponse("New WiFi config received. Reconnecting...");
     
      // Reconnect with new credentials
      WiFi.disconnect();
      delay(1000);
      WiFi.begin(newSSID.c_str(), newPassword.c_str());
     
      // Wait for connection
      int connectionAttempts = 0;
      while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
        delay(500);
        connectionAttempts++;
      }
     
      if (WiFi.status() == WL_CONNECTED) {
        sendResponse("Connected to new network. IP: " + WiFi.localIP().toString());
      } else {
        sendResponse("Failed to connect to new network");
      }
    } else {
      sendResponse("ERROR: Invalid WiFi config format");
    }
  }
  else {
    sendResponse("ERROR: Unknown command");
  }
}

void flashLED(int numFlashes, int flashDelay) {
  for (int i = 0; i < numFlashes; i++) {
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(flashDelay);
    digitalWrite(FLASH_LED_PIN, LOW);
    delay(flashDelay);
  }
}

void sendResponse(String response) {
  Serial.println(response);
}
