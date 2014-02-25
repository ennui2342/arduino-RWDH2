#include <RWDH2.h>
#include <SoftwareSerial.h>

uint8_t rx_pin = 11;
uint8_t tx_pin = 12;
uint8_t cts_pin = 10;

SoftwareSerial rwdh2_serial(rx_pin, tx_pin);

// To use hardware serial replace rwdh2_serial with Serial
RWDH2 rwdh2(rwdh2_serial, cts_pin);

uint8_t my_tag[] = { 0x82, 0x5B, 0xC7, 0x1C };

// OEM factory defaults (vendor tags are likely to be altered from these defaults)
uint8_t rwdh2_password[] = { 0x4D, 0x49, 0x4B, 0x52 };
uint8_t tag_password[] = { 0xAA, 0x48, 0x54 };
uint8_t tag_userdata[] = { 0x4E, 0x5F, 0x4F, 0x4B, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55 };

void setup() {
  Serial.begin(115200);
  rwdh2_serial.begin(RWDH2_BAUD);
  
  Serial.println("Initialising RWDH2...");
  rwdh2.begin();

  uint8_t id[200];
  if (rwdh2.getIdentifier(id)) {
    Serial.println((char*) id);
  } else {
    Serial.print("RWDH2 initialisation failed: ");
    Serial.println(rwdh2.getLastAck(), HEX);
  }
  
  Serial.println("Deauthorising all tags (please wait)...");
  if (!rwdh2.deauthoriseAllTags()) {
    Serial.print("Deauthorisation failed: ");
    Serial.println(rwdh2.getLastAck(), HEX);
  }

  if (rwdh2.getStatus()) {
    Serial.print("RWDH2 status: ");
    Serial.println(rwdh2.getLastAck(), HEX);
  } else {
    Serial.print("RWDH2 status check failed: ");
    Serial.println(rwdh2.getLastAck(), HEX);
  }
  
  Serial.println("Setting RWD password in EEPROM...");
  if(!rwdh2.writeEEPROMRWDH2Password(rwdh2_password)) {
    Serial.print("EEPROM set RWDH2 password failed: ");
    Serial.println(rwdh2.getLastAck(), HEX);
  }

  Serial.println("Setting tag password in EEPROM...");  
  if(!rwdh2.writeEEPROMTagPassword(tag_password)) {
    Serial.print("EEPROM set transponder password failed: ");
    Serial.println(rwdh2.getLastAck(), HEX);
  }

  Serial.println("Authorising tag...");
  if (!rwdh2.authoriseTag(my_tag, 0)) {
    Serial.print("Tag authorisation failed: ");
    Serial.println(rwdh2.getLastAck(), HEX);
  }
}

void loop() {
  uint8_t uid[4];
  if (rwdh2.getUID(uid)) {
    Serial.print("Got a tag: ");
    for (uint8_t i=0; i < 4; i++) {
      Serial.print("0x");
      Serial.print(uid[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    uint8_t data[16];
    if (rwdh2.readTagUserData(data)) {
      for (uint8_t j = 0; j < 16; j++) {
        Serial.print("0x");
        Serial.print(data[j], HEX);
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.print("Reading tag user data failed: ");
      Serial.println(rwdh2.getLastAck(), HEX);
    }
    
    if (!rwdh2.writeTagUserData(tag_userdata)) {
      Serial.print("Writing tag user data failed: ");
      Serial.println(rwdh2.getLastAck(), HEX);
    }
    
    for (uint8_t i = 0; i < sizeof(tag_userdata); i++) {
      tag_userdata[i]++;
      if (tag_userdata[i] == 0xFF)
        tag_userdata[i] = 0x0;
    }
  } 
  delay(50);
}
