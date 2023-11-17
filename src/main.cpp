#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "DHTesp.h"

#include <NeoPixelBus.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <WiFi.h>
#include <NTPClient.h>

#include <Ticker.h>

#define ssid "acacio"
#define password "untajana"

WiFiUDP ntpUDP;
NTPClient timeClient (ntpUDP, "id.pool.ntp.org", 7*3600);
Ticker timer1sec;

int led_count = 16;
int led_strip_pin = 26;

int mosfetpin = 25;

// int seconds = 0;
// int minutes = 0;
// int hour = 0;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(led_count, led_strip_pin);

TFT_eSPI tft = TFT_eSPI();
DHTesp dht;
  
int LED = 14;
bool lampOn = false;

int TOUCH_THRESHOLD = 700;

bool setting_button_value = false;

uint8_t touchX, touchY;

SemaphoreHandle_t xSemaphore;

IRAM_ATTR void call (){
  xSemaphoreGive (xSemaphore);
}

void WiFiConnect (){
  Serial.println ("Conncecting to WiFi");
  WiFi.mode (WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED){
    Serial.println ("WiFi connected");
    Serial.println ("IP address: ");
    Serial.println (WiFi.localIP());
  }
  else{
    Serial.println ("WiFi connection failed");
  }
}

void playButton() {
  tft.fillScreen(TFT_BLACK);

  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;
  int radius = 40;

  tft.fillCircle(centerX, centerY, radius, TFT_WHITE);
  tft.fillTriangle(centerX + 60 / 2, centerY, centerX - radius / 2, centerY + radius / 2, centerX - radius / 2, centerY - radius / 2, TFT_BLACK);
}

// void displayTime() {
//   // tft.setTextSize(3);
//   // tft.setTextColor(TFT_WHITE);
//   // tft.drawString("10:30 AM", 90, 20);

//   // tft.setTextSize(2);
//   // tft.setTextColor(TFT_WHITE);
//   // tft.drawString("Jumat, 7 Jul 2023", 60, 50);

//   // seconds++;
//   //   if (seconds > 59) {
//   //     seconds = 0;
//   //     minutes++;
//   //     if (minutes > 59) {
//   //       minutes = 0;
//   //       hours++;
//   //       if (hours > 23) {
//   //         hours = 0;
//   //       }
//   //     }
//   //   }
//   // volatile int seconds = 0;
//   // volatile int minutes = 16;
//   // volatile int hours = 7;
// }

void drawNextButton() {
  int x = 280;
  int y = tft.height() / 2 - 20;
  int size = 40;

  tft.fillTriangle(x - size, y, x - size, y + size, x, y + size / 2, TFT_WHITE);
}

void drawPreviousButton() {
  int x = 40;
  int y = tft.height() / 2 - 20;
  int size = 40;

  tft.fillTriangle(x + size, y, x + size, y + size, x, y + size / 2, TFT_WHITE);
}

void settingsbutton (){

  int centerX = 30;
  int centerY = 210;
  int outerRadius = 22;
  int innerRadius = 16;
  int toothLength = 10;
  int toothWidth = 4;

  // Draw outer circle
  tft.fillCircle(centerX, centerY, outerRadius, TFT_BLACK);

  // Draw inner circle
  tft.fillCircle(centerX, centerY, innerRadius, TFT_WHITE);

  // Draw gear teeth
  for (int i = 0; i < 6; i++) {
    float angle = PI + (i * PI / 3);
    int x1 = centerX + (outerRadius - toothLength) * cos(angle);
    int y1 = centerY + (outerRadius - toothLength) * sin(angle);
    int x2 = x1 + toothWidth * cos(angle - PI / 2);
    int y2 = y1 + toothWidth * sin(angle - PI / 2);
    int x3 = x2 + toothLength * cos(angle);
    int y3 = y2 + toothLength * sin(angle);

    tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_BLACK);
  }

  // int x = 10;
  // int y = 200;
  // int width = 100;
  // int height = 30;
  
  // tft.fillRect(x, y, width, height, TFT_WHITE);

  // tft.setTextSize (2);
  // tft.setTextColor (TFT_BLACK);
  // tft.drawString ("Settings", 10, 205);
}

void settingspage (){

  tft.setTextSize(2);
  tft.setTextColor (TFT_BLUE);
  tft.drawString ("Settings Page", 100, 20);

  int x = 10;
  int y = 200;
  int width = 100;
  int height = 30;
  
  tft.fillRect(x, y, width, height, TFT_WHITE);

  tft.setTextSize (2);
  tft.setTextColor (TFT_BLACK);
  tft.drawString ("Back", 10, 205);
}

void onTimer1Sec(){
  tft.setTextColor (TFT_WHITE);
  tft.setTextSize (2);
  tft.drawString (timeClient.getFormattedTime(), 100, 0);
}

void RGB (void *pvParameters){

  while (1){
  // Set all LEDs to a single color
  RgbColor color (255, 0, 0);  // Red color
  strip.ClearTo(color);
  strip.Show();
  delay(1000);

  // Set all LEDs to a different color
  color = RgbColor(0, 255, 0);  // Green color
  strip.ClearTo(color);
  strip.Show();
  delay(1000);

  // Fade in and out effect
  for (int brightness = 0; brightness <= 255; brightness++) {
    color = RgbColor(brightness, brightness, brightness);
    strip.ClearTo(color);
    strip.Show();
    delay(10);
  }

  for (int brightness = 255; brightness >= 0; brightness--) {
    color = RgbColor(brightness, brightness, brightness);
    strip.ClearTo(color);
    strip.Show();
    delay(10);
  }
  }
}

// void displaytimeRTOS (void *pvParameters){
//   timeClient.begin();
//   timer1sec.attach (1, onTimer1Sec);
//   while (1){
//     timeClient.update();
//   }
// }

void buttonsdisplay (void *pvParameters){
  playButton();
  drawNextButton();
  drawPreviousButton();
  settingsbutton();

  while (1){
  // playButton();
  // drawNextButton();
  // drawPreviousButton();
  // settingsbutton(); 
  }
}

void setup() {
  Serial.begin (115200);

  strip.Begin();
  strip.Show();

  tft.begin();
  tft.setRotation(3);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // WiFiConnect();

  digitalWrite (mosfetpin, HIGH);

  xSemaphore = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore (RGB, "RGB", configMINIMAL_STACK_SIZE+2048, NULL, 1, NULL, 1);
  // xTaskCreatePinnedToCore (displaytimeRTOS, "displaytimeRTOS", configMINIMAL_STACK_SIZE+8192, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore (buttonsdisplay, "buttondisplay", configMINIMAL_STACK_SIZE+4096, NULL, 1, NULL, 0);
}

void loop() {

  uint16_t touchX, touchY;

  if (tft.getTouch(&touchX, &touchY, TOUCH_THRESHOLD) && touchX >= 100 && touchX <= 200 && touchY >= 60 && touchY <= 110) {
    lampOn = !lampOn;
    digitalWrite(LED, lampOn ? HIGH : LOW);
    Serial.println ("Play Button pressed");
    delay (300);
  }

  if (tft.getTouch(&touchX, &touchY, TOUCH_THRESHOLD) && touchX >= 10 && touchX <= 40 && touchY >= 10 && touchY <= 40 && setting_button_value == false){
    tft.fillScreen (TFT_BLACK);
    settingspage();
    setting_button_value = true;
    delay (300);
  }

  if (tft.getTouch(&touchX, &touchY, TOUCH_THRESHOLD) && touchX >= 10 && touchX <= 100 && touchY >= 10 && touchY <= 40 && setting_button_value == true){
  tft.fillScreen (TFT_BLACK);
  playButton();
  // displayTime();
  drawNextButton();
  drawPreviousButton();
  settingsbutton();
  setting_button_value = false;
  delay (300);
  }  
}
