# arduino-RWDH2

Arduino library to interface with [ibtechnology](http://www.ibtechnology.co.uk)'s
[RWD Hitag2](http://www.ibtechnology.co.uk/products/hitag2-product.htm) RFID modules

## Installation

Clone as RWDH2 and put in your arduino libraries folder

## Notes

Please note that the ibtechnology modules only support PASSWORD mode, not crypto

This library is based on the [RWDH2 datasheet](http://www.ibtechnology.co.uk/pdf/H2PROT.PDF)

## Usage

See the examples included. Simple reading of tag UID's:

```c++
#include <RWDH2.h>
#include <SoftwareSerial.h>

uint8_t rx_pin = 11;
uint8_t tx_pin = 12;
uint8_t cts_pin = 10;

SoftwareSerial rwdh2_serial(rx_pin, tx_pin);

// To use hardware serial replace rwdh2_serial with Serial
RWDH2 rwdh2(rwdh2_serial, cts_pin);

void setup() {
  Serial.begin(115200);
  rwdh2_serial.begin(RWDH2_BAUD);
  
  Serial.println("Initialising RWDH2...");
  Serial.println("Enabling UID mode...");
  rwdh2.begin(true);

  uint8_t id[200];
  if (rwdh2.getIdentifier(id)) {
    Serial.println((char*) id);
  } else {
    Serial.print("RWDH2 initialisation failed: ");
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
  } 
  delay(50);
}
```
