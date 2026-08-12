#ifndef MP3_H
#define MP3_H

#include <SoftwareSerial.h>

#define MP3_RX 16  // ESP32 GPIO16 → MP3 TX
#define MP3_TX 4   // ESP32 GPIO4 → MP3 RX

SoftwareSerial mp3Serial(MP3_RX, MP3_TX);

class SimpleMP3 {
private:
  void sendCommand(byte cmd, uint16_t param = 0) {
    byte data[10] = {0x7E, 0xFF, 0x06, cmd, 0x00, 
                     (byte)(param >> 8), (byte)(param & 0xFF), 0, 0, 0xEF};
    
    // Calculate checksum (FIXED!)
    uint16_t sum = 0;
    for(int i = 1; i <= 6; i++) {
      sum += data[i];
    }
    uint16_t checksum = 0xFFFF - sum + 1;
    
    data[7] = checksum >> 8;
    data[8] = checksum & 0xFF;
    
    for(int i = 0; i < 10; i++) {
      mp3Serial.write(data[i]);
    }
    delay(100); // Important: Wait for command to process
  }
  
public:
  void begin() {
    mp3Serial.begin(9600); //MUST BE baud 9600!!!
    delay(1000); // Let module boot up
  }
  
  void volume(uint8_t vol) {
    if(vol > 30) vol = 30;
    sendCommand(0x06, vol);
  }
  
  void play(uint16_t track) {
    sendCommand(0x03, track);
  }
  
  void stop() {
    sendCommand(0x16);
  }
  
  void pause() {
    sendCommand(0x0E);
  }
  
  void resume() {
    sendCommand(0x0D);
  }
  
  void next() {
    sendCommand(0x01);
  }
  
  void previous() {
    sendCommand(0x02);
  }
  
  void playFolder(uint8_t folder, uint8_t track) {
    sendCommand(0x0F, (folder << 8) | track);
  }
};

#endif