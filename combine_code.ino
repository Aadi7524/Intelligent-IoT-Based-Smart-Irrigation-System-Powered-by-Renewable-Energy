#define BLYNK_TEMPLATE_ID "TMPL3HNrCZNPH"
#define BLYNK_TEMPLATE_NAME "smart irrigation system"
#define BLYNK_AUTH_TOKEN "lJdA3ePMZE050CvhR_dlJdpQ12aPnfTt"


#include <ESP32Servo.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// --- LDR and Servo Setup ---
#define LDR1 34    // ADC1_CH6 (OK)
#define LDR2 35    // ADC1_CH7 (OK)
#define SERVO_PIN 13
#define LDR_ERROR 50

Servo servo;
int Spoint = 90;

// --- Soil Moisture & Pump Setup ---
#define SOIL_MOISTURE_PIN 32   // Use different ADC pin (safe)
#define THRESHOLD_MOISTURE 3000
#define PUMP_PIN 2             // Digital output pin
#define PUMP_SWITCH V6         // Virtual pin on Blynk

// --- Blynk Credentials ---

#define BLYNK_PRINT Serial

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "EACCESS";
char pass[] = "hostelnet";

BlynkTimer timer;
bool isPumpOn = false;

// --- BLYNK PUMP CONTROL ---
BLYNK_WRITE(PUMP_SWITCH) {
  isPumpOn = param.asInt();
  if (isPumpOn) {
    Serial.println("Pump manually turned ON");
  } else {
    Serial.println("Pump manually turned OFF");
    digitalWrite(PUMP_PIN, LOW);
  }
}

// --- Sensor Functions ---
void sendSensorData() {
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  Serial.print("Soil Moisture (Raw): ");
  Serial.println(soilMoisture);

  Blynk.virtualWrite(V5, soilMoisture);

  if (isPumpOn || soilMoisture < THRESHOLD_MOISTURE) {
    digitalWrite(PUMP_PIN, HIGH);
    if (!isPumpOn) {
      Blynk.logEvent("moisture_alert", "Soil moisture is below the threshold!");
    }
  } else {
    if (!isPumpOn) {
      digitalWrite(PUMP_PIN, LOW);
    }
  }
}

void trackLight() {
  int ldr1 = analogRead(LDR1);
  int ldr2 = analogRead(LDR2);
  int diff = abs(ldr1 - ldr2);

  Serial.print("LDR1: "); Serial.print(ldr1);
  Serial.print(" | LDR2: "); Serial.print(ldr2);
  Serial.print(" | Diff: "); Serial.println(diff);

  if (diff > LDR_ERROR) {
    if (ldr1 > ldr2 && Spoint > 0) {
      Spoint--;
    } else if (ldr2 > ldr1 && Spoint < 180) {
      Spoint++;
    }
    servo.write(Spoint);
  }
  delay(80);
}

// --- Setup ---
void setup() {
  Serial.begin(115200);

  // Servo setup
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, 1000, 2000);
  servo.write(Spoint);

  // Pump setup
  pinMode(PUMP_PIN, OUTPUT);

  // Blynk setup
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(3000L, sendSensorData);
  timer.setInterval(200L, trackLight);  // Run light tracking every 200ms

  Blynk.virtualWrite(PUMP_SWITCH, isPumpOn);
  Blynk.syncVirtual(PUMP_SWITCH);
}

// --- Main Loop ---
void loop() {
  Blynk.run();
  timer.run();
}
