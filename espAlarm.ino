#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> //1.54 Inch Display SPI ST7789
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "button.h"
#include "mp3.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BUZZER_PIN 26

#define OLED_ADDR 0x3C

#define ONE_WIRE_BUS 0

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

#define BUTTON_PIN_UP 17
#define BUTTON_PIN_OK 19
#define BUTTON_PIN_DOWN 18

Button btnUp(BUTTON_PIN_UP);
Button btnOk(BUTTON_PIN_OK);
Button btnDown(BUTTON_PIN_DOWN);

SimpleMP3 mp3;

float currentTemperature = 0.0;
unsigned long lastTempRead = 0;
const unsigned long TEMP_READ_INTERVAL = 2000;

int choice = 0;

const int choiceCount = 3;

bool inViewTime = 0;
bool inSetTime = 0;
bool inSetAlarm1 = 0;
bool inSetAlarm2 = 0;
bool inWhichAlarm = 0;

unsigned long lastSecond = 0;

int seconds = 0;
int minutes = 0;
int hours = 0;

RTC_DS3231 rtc;

unsigned long lastDraw = 0;

bool menuDirty = 1;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* options[] = {

  "View time",
  "Set time",
  "Set alarm"

};

const char* alarmArray[] = {

  "Alarm 1",
  "Alarm 2"

};

void whichAlarm(){

  static int alarmChoice;
  menuDirty = true;

  if(btnUp.pressed()){
    
    alarmChoice++; 

    if(alarmChoice >= 2){alarmChoice = 0;};

    menuDirty = true;

  }else if(btnDown.pressed()){

    alarmChoice--;

    if(alarmChoice < 0){alarmChoice = 1;};

    menuDirty = true;

  }

  if(menuDirty){

    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);

    for(int i = 0; i < 2; i++){

      if(alarmChoice == i) display.print(">");
      display.println(alarmArray[i]);
      
    }

    getAlarms();

    display.display();
    menuDirty = false;

  }

  if(btnOk.pressed()){

    if(alarmChoice == 0){ inWhichAlarm = 0; display.clearDisplay(); inSetAlarm1 = 1; }
    else { inWhichAlarm = 0; display.clearDisplay(); inSetAlarm2 = 1; }

  }

}

void storeAlarm1(int hour, int minute){

  rtc.setAlarm1(
    DateTime(0, 0, 0, hour, minute, 0),
    DS3231_A1_Hour
  );

}

void storeAlarm2(int hour, int minute){

  rtc.setAlarm2(
    DateTime(0, 0, 0, hour, minute, 0),
    DS3231_A2_Hour
  );

}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C)
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  display.print(tempC);
  display.print("'C");
}

void setup() {
  // put your setup code here, to run once:

//////////////////////////////////////////////////////
      Serial.begin(115200);
      delay(2000);
      
      mp3.begin();
      
      // // Set volume (0-30)
      mp3.volume(5);
      delay(500);
//////////////////////////////////////////////////////

  btnUp.setPinMode();
  btnOk.setPinMode();
  btnDown.setPinMode();

  ////////////////////////// Temp sensor stuff

  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");

  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();

  //////////////////////////

  Wire.begin(21, 22);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)){

    Serial.println(F("OLED initialization failed :("));
    while(true);

  }

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // F stores the text in flash memory rather than SRAM
  //display.println(F("1 option A \n2 option B \n3 option C"));
  display.display();

  rtc.begin();

  if (rtc.lostPower()) {
    Serial.println(F("RTC is Lost Power!"));
    // This line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.disable32K();                 // optional, safe
  rtc.clearAlarm(1);                // clear old alarms
  rtc.clearAlarm(2);
  rtc.writeSqwPinMode(DS3231_OFF);

}

int choice2 = 0;

int list(const char* listArray[], int arraySize, bool& previousPage){

  btnUp.update();
  btnOk.update();
  btnDown.update();

  static bool menu2Dirty = false;
  static bool initialized = false;

  if(!initialized){

    previousPage = 0;
    choice2 = 0;
    menu2Dirty = true;

  }

  if(btnUp.pressed()){
    
    choice2++; 

    if(choice2 >= arraySize){choice2 = 0;};

    menu2Dirty = true;

  }else if(btnDown.pressed()){

    choice2--;

    if(choice2 < 0){choice2 = arraySize - 1;};

    menu2Dirty = true;

  }

  if(menu2Dirty){

    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);

    for(int i = 0; i < arraySize; i++){

      if(choice2 == i) display.print("> ");
      display.println(listArray[i]);
      
    }

    display.display();
    menu2Dirty = false;

  }

  if(btnOk.pressed()){

    Serial.println("bobibobdipopopopopopp");
    previousPage = 1;
    initialized = false;
    return choice2;

  }

  return -1;

};

void ViewTime(){

  DateTime now = rtc.now();

  hours = now.hour();
  minutes = now.minute();
  seconds = now.second();

  int displayHour = hours % 12;
  if (displayHour == 0) displayHour = 12;
  bool isAM = (hours < 12);

  if (millis() - lastDraw >= 200) {

    display.setCursor(0, 0);
    display.setTextSize(2);

    lastDraw = millis();

    display.clearDisplay();

    if(displayHour < 10)display.print("");
    display.print(displayHour);
    display.print(":");

    if(minutes < 10)display.print("0");
    display.print(minutes);
    display.print(":");

    if(seconds < 10)display.print("0");
    display.println(seconds);

    if(isAM == 1)display.println("AM");
    else display.println("PM");
    display.println();

    sensors.requestTemperatures();
    printTemperature(insideThermometer);

    display.display();
  }
  
  if(btnOk.pressed()){ 
    
    display.clearDisplay();
    inViewTime = 0;
    menuDirty = 1;
  
  }

}

int input = 0;

int setStage = 0;

void SetTime(){

  static bool waited = false;
  static unsigned long startTime;

  if (!waited) {
    startTime = millis();
    waited = true;
    return;
  }

  if (millis() - startTime < 1000) {
    return;   // still waiting
  }

  display.setCursor(0, 0);
  display.setTextSize(2);

  if(setStage == 0){

  display.print("hour: ");

  if(btnUp.pressed()){
      input++;
      if(input > 23){input = 0;} 
      display.fillRect(72, 0, 24, 16, SSD1306_BLACK);
      display.print(input);
      }

  if(btnDown.pressed()){
      input--;
      if(input < 0){input = 23;}
      display.fillRect(72, 0, 24, 16, SSD1306_BLACK);
      display.print(input);
      }

  if(btnOk.pressed()){ hours = input; setStage = 1; input = 0; display.clearDisplay(); }

  
  }

  else if(setStage == 1){

    display.print("minute: ");

    if(btnUp.pressed()){
    input++; 
    if(input > 59){input = 0;}
    display.fillRect(96, 0, 24, 16, SSD1306_BLACK);
    display.print(input);
    }

    if(btnDown.pressed()){
    input--;
    if(input < 0){input = 59;}
    display.fillRect(96, 0, 24, 16, SSD1306_BLACK);
    display.print(input);
    }

    if(btnOk.pressed()){ minutes = input; setStage = 2; input = 0; display.clearDisplay(); }

  }

  else if(setStage == 2){

    display.print("second: ");

    if(btnUp.pressed()){
    input++;
    if(input > 59){input = 0;}
    display.fillRect(96, 0, 24, 16, SSD1306_BLACK);
    display.print(input);
    }

    if(btnDown.pressed()){
    input--;
    if(input < 0){input = 59;}
    display.fillRect(96, 0, 24, 16, SSD1306_BLACK);
    display.print(input);
    }

    if(btnOk.pressed()){ seconds = input; setStage = 0; inSetTime = 0; inViewTime = 1;
    DateTime now = rtc.now();
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), hours, minutes, seconds));
    }

  }

  display.display();

}

void SetAlarm1(){

  display.setCursor(0, 0);
  display.setTextSize(2);

  static int hour;
  static int minute;

  if(setStage == 0){

  display.print("hour: ");

  if(btnUp.pressed()){
      input++;
      if(input > 23){input = 0;} 
      display.fillRect(72, 0, 24, 16, SSD1306_BLACK);
      display.print(input);
      }

  if(btnDown.pressed()){
      input--;
      if(input < 0){input = 23;}
      display.fillRect(72, 0, 24, 16, SSD1306_BLACK);
      display.print(input);
      }

  if(btnOk.pressed()){ hour = input; setStage = 1; input = 0; display.clearDisplay(); }

  
  }

  else if(setStage == 1){

    display.print("minute: ");

    if(btnUp.pressed()){
    input++; 
    if(input > 59){input = 0;}
    display.fillRect(96, 0, 24, 16, SSD1306_BLACK);
    display.print(input);
    }

    if(btnDown.pressed()){
    input--;
    if(input < 0){input = 59;}
    display.fillRect(96, 0, 24, 16, SSD1306_BLACK);
    display.print(input);
    }

    if(btnOk.pressed()){

      minute = input;
      storeAlarm1(hour, minute);
      setStage = 0;
      inSetAlarm1 = 0;
      menuDirty = 1;
      display.clearDisplay();

    }

  }

  display.display();

}

void SetAlarm2(){

  display.setCursor(0, 0);
  display.setTextSize(2);

  static int hour;
  static int minute;

  if(setStage == 0){

  display.print("hour: ");

  if(btnUp.pressed()){
      input++;
      if(input > 23){input = 0;} 
      display.fillRect(72, 0, 24, 16, SSD1306_BLACK);
      display.print(input);
      }

  if(btnDown.pressed()){
      input--;
      if(input < 0){input = 23;}
      display.fillRect(72, 0, 24, 16, SSD1306_BLACK);
      display.print(input);
      }

  if(btnOk.pressed()){ hour = input; setStage = 1; input = 0; display.clearDisplay(); }

  
  }

  else if(setStage == 1){

    display.print("minute: ");

    if(btnUp.pressed()){
    input++; 
    if(input > 59){input = 0;}
    display.fillRect(96, 0, 24, 16, SSD1306_BLACK);
    display.print(input);
    }

    if(btnDown.pressed()){
    input--;
    if(input < 0){input = 59;}
    display.fillRect(96, 0, 24, 16, SSD1306_BLACK);
    display.print(input);
    }

    if(btnOk.pressed()){

      minute = input;
      storeAlarm2(hour, minute);
      setStage = 0;
      inSetAlarm2 = 0;
      menuDirty = 1;
      display.clearDisplay();

    }

  }

  display.display();

}

void getAlarms(){

  DateTime a1 = rtc.getAlarm1();

  display.print("A1 ");
  display.print(a1.hour());
  display.print(":");
  display.println(a1.minute());

  DateTime a2 = rtc.getAlarm2();
  
  display.print("A2 ");
  display.print(a2.hour());
  display.print(":");
  display.print(a2.minute());

}

void checkAlarms(){

  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 6000) return;
  else {
    
    lastCheck = millis();

    DateTime alarm1 = rtc.getAlarm1();
    DateTime alarm2 = rtc.getAlarm2();
    DateTime now = rtc.now();

    const int durationMinutes = 60;

    int currentMinute = now.hour()*60 + now.minute();
    int alarm1Start = alarm1.hour()*60 + alarm1.minute();
    int alarm2Start = alarm2.hour()*60 + alarm2.minute();
    int alarm1End = alarm1.hour()*60 + alarm1.minute() + durationMinutes;
    int alarm2End = alarm2.hour()*60 + alarm2.minute() + durationMinutes;

    if(currentMinute >= alarm1Start && currentMinute < alarm1End){

      //play our mp3 file
      Serial.println("alarm 1 ON wake up dude");
      mp3.playFolder(1, 1);

      //playMelodyLong(); Delay problem

    }

    if(currentMinute >= alarm2Start && currentMinute < alarm2End){

      //play our mp3 file
      Serial.println("alarm 2 ON wake up dude");
      mp3.playFolder(1, 1);

      //playMelodyLong(); Delay problem

    }

  }

}

void loop() {

  // put your main code here, to run repeatedly:
  //delay(50); // this speeds up the simulation
  static unsigned long lastLoop = 0;
  if (millis() - lastLoop < 50) return;
  lastLoop = millis();

  btnUp.update();
  btnOk.update();
  btnDown.update();

  checkAlarms(); // triggers alarm way too fast back to back preventing the speaker from playing the sound, must make a time limiter function to wait for atleast 10 to 30 seconds

  if (inViewTime) {
    ViewTime();
    return;
  }

  else if(inSetTime){
    SetTime();
    return;
  }

  else if(inWhichAlarm){
    whichAlarm();
    return;
  }

  else if(inSetAlarm1){
    SetAlarm1();
    return;
  }

  else if(inSetAlarm2){
    SetAlarm2();
    return;
  }

  if(btnUp.pressed()){ choice--; menuDirty = 1; if(choice < 0){choice = choiceCount - 1;}}
  else if(btnDown.pressed()){ choice++; menuDirty = 1; if(choice >= choiceCount){choice = 0;}}

  if(menuDirty == 1){

  // --- Draw menu ---
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);

  for(int i = 0; i < 3; i++){

    
    if(choice == i){display.print(">");}
    else{display.print("");}
  
    display.println(options[i]);
    
  }

  display.display();
  
  // tone(26, 20, 1);

  menuDirty = 0;

}

  if(btnOk.pressed()){

    switch (choice) 
    {
      case 0:
      
      display.clearDisplay();
      inViewTime = 1;
      lastDraw = 0;

      break;

      case 1:

      display.clearDisplay();
      inSetTime = 1;

      break;

      case 2:

      display.clearDisplay();
      inWhichAlarm = 1;
      menuDirty = true;

      break;
    
    default:
      break;
    }

  }

}


void playMelodyLong() {

  int melody[] = {
    523, 659, 784, 659, 523, 587, 659, 784,
    880, 784, 659, 523, 587, 659, 523, 494,
    523, 659, 784, 880, 988, 880, 784, 659,
    698, 784, 880, 784, 659, 523, 587, 523
  };

  int durations[] = {
    200, 200, 300, 200, 200, 200, 300, 400,
    200, 200, 300, 200, 200, 200, 300, 400,
    200, 200, 300, 200, 300, 200, 300, 400,
    200, 200, 300, 200, 200, 200, 300, 600
  };

  int length = sizeof(melody) / sizeof(melody[0]);

  for (int i = 0; i < length; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    delay(durations[i] * 1.3);
  }

  noTone(BUZZER_PIN);
}


// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
// #define OLED_ADDR 0x3C

// // I2C pins for your ESP32
// #define SDA_PIN 21
// #define SCL_PIN 22

// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// void setup() {
//   Wire.begin(SDA_PIN, SCL_PIN);

//   if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)){
//     Serial.println(F("OLED init failed!"));
//     while(true);
//   }

//   display.clearDisplay();
// }

// void loop() {
//   display.clearDisplay();

//   // Draw a filled wave pattern
//   for(int x = 0; x < SCREEN_WIDTH; x++){
//     // Simple sine wave
//     float y = 20.0 * sin((x + millis() / 10.0) * 0.1) + 32; 
    
//     // Fill from bottom of screen up to the wave
//     for(int yy = (int)y; yy < SCREEN_HEIGHT; yy++){
//       display.drawPixel(x, yy, SSD1306_WHITE);
//     }
//   }

//   display.display();
//    // controls animation speed
// }
