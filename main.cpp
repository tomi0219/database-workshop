#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#include <ESP32Servo.h>

// Insert your network credentials
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASS"

#define DATABASE_URL "database_url.firebaseio.com"
#define API_KEY "API_KEYS"

// Firebase Objects
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdoStream; // Dedicated object for the Real-time Servo Stream
FirebaseData fbdoSet;    // Dedicated object for the IR Sensor Uploads

// Hardware Pins
int servo_pin = __;
int ir_sensor = __; 
int lastStatus = -1;

Servo servo;

// --- CALLBACK FUNCTIONS ---

// This function runs automatically the instant the website slider moves
void streamCallback(FirebaseStream data) {
  if (data.dataType() == "int") {
    int ____ = data.intData();
    
    // Only move if the value is different from the last known position
    static int lastServoPos = -1;
    if (servoValue != lastServoPos) {
      Serial.println("Moving Servo to: " + String(servoValue));
      servo.write(servoValue);
      lastServoPos = servoValue;
    }
  }
}

void streamTimeoutCallback(bool timeout) {
  if (____) Serial.println("Stream timeout, resuming...");
}

// --- HELPER FUNCTIONS ---

void initWiFi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != _____){
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected!");

  // Sync time
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) { // Wait until time is updated (past 1970)
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nTime synchronized.");
}

void uploadSensorData(int status) {
  // Give the SSL engine a moment to finish any background stream tasks
  delay(10); 
  
  if (Firebase.RTDB.setInt(&___, "sensor/obstacle", status)) {
    Serial.println("IR Status Uploaded: " + String(status));
  } else {
    Serial.println("IR Upload failed: " + fbdoSet.errorReason());
  }
}
// --- MAIN SETUP ---

void setup() {
  Serial.begin(115200);
  
  pinMode(____, INPUT);
  servo.attach(____);
  servo.write(90); // Start at center
  
  initWiFi();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up and start Firebase
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase Auth OK");
  } else {
    Serial.printf("Auth Error: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  // Set the keep-alive timeout to 1 minute (60 seconds)
config.timeout.serverResponse = 60 * 1000;
// Disable "Pause" mode to keep the SSL engine active
fbdoStream.keepAlive(1, 1, 1);
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // START STREAMING: Listening to "servo/msgTitle" using 'fbdoStream'
  if (!Firebase.RTDB.beginStream(&fbdoStream, "servo/msgTitle")) {
    Serial.println("Stream begin error: " + fbdoStream.errorReason());
  }
  Firebase.RTDB.setStreamCallback(&fbdoStream, streamCallback, streamTimeoutCallback);
}

// --- MAIN LOOP ---

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    initWiFi();
  }

  // Check if Firebase is ready before doing anything
  if (!___________()) return;

  // Read IR Sensor (0 = Object, 1 = Clear)
  int currentStatus = digitalRead(ir_sensor); 

  // Only upload to Firebase if the status changes to save bandwidth
  if (currentStatus != lastStatus) {
    uploadSensorData(currentStatus);
    lastStatus = currentStatus;
  }

  // Small delay for stability; Servo is handled instantly by streamCallback
  delay(50); 
}
