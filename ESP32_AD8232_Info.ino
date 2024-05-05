#include <BluetoothSerial.h>
#include <Adafruit_SSD1306.h>

#define LED_BUILTIN 2 //pin with LED to turn on when BT connected
#define OLED_Address 0x3C

BluetoothSerial ESP_BT; // Object for Bluetooth
Adafruit_SSD1306 oled(1);

boolean BT_cnx = false;

int x = 0;
int lastx = 0;
int lasty = 0;
int LastTime = 0;
bool BPMTiming = false;
bool BeatComplete = false;
int BPM = 0;

#define UpperThreshold 550
#define LowerThreshold 500

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Client Connected");
    digitalWrite(LED_BUILTIN, HIGH);
    BT_cnx = true;
  }

  if (event == ESP_SPP_CLOSE_EVT) {
    Serial.println("Client disconnected");
    digitalWrite(LED_BUILTIN, LOW);
    BT_cnx = false;
    ESP.restart();
  }
}

void setup() {
  // initialize digital pin 2 as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize the serial communication:
  Serial.begin(9600);
  Serial.println(); // blank line in serial ...
  pinMode(41, INPUT); // leads off detection LO +
  pinMode(40, INPUT); // leads off detection LO -

  // BT CON
  ESP_BT.register_callback(callback);
  if (!ESP_BT.begin("ESP32_ECG")) {
    Serial.println("An error occurred initializing Bluetooth");
  } else {
    Serial.println("Bluetooth initialized... Bluetooth Device is Ready to Pair...");
  }

  oled.begin(SSD1306_SWITCHCAPVCC, OLED_Address);
  oled.clearDisplay();
  oled.setTextSize(2);
}

void loop() {
  if ((digitalRead(40) == 1) || (digitalRead(41) == 1)) {
    Serial.println('!');
    ESP_BT.println('!');
  } else {
    // send the value of analog input 0 to serial:
    Serial.println(analogRead(A0));
    //Do the same for Bluetooth
    if (BT_cnx) {
      ESP_BT.print('E');
      ESP_BT.println(analogRead(A0));
    }
  }

  if (x > 127) {
    oled.clearDisplay();
    x = 0;
    lastx = x;
  }

  int value = analogRead(0);
  oled.setTextColor(WHITE);
  int y = 60 - (value / 16);
  oled.writeLine(lastx, lasty, x, y, WHITE);
  lasty = y;
  lastx = x;
  
  // Calculate BPM
  if (value > UpperThreshold) {
    if (BeatComplete) {
      BPM = millis() - LastTime;
      BPM = int(60 / (float(BPM) / 1000));
      BPMTiming = false;
      BeatComplete = false;
    }
    if (BPMTiming == false) {
      LastTime = millis();
      BPMTiming = true;
    }
  }
  if ((value < LowerThreshold) & (BPMTiming))
    BeatComplete = true;
    
  // Display BPM
  oled.writeFillRect(0, 50, 128, 16, BLACK);
  oled.setCursor(0, 50);
  oled.print(BPM);
oled.print(" BPM");
oled.display();
x++;

//Wait a little to keep serial data from saturating
delay(1);
}
