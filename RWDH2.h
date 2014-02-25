/**
 *
 * @file        RWDH2.h
 * @author      [Mark Cheverton][]
 * @copyright   MIT license
 *
 *  Arduino library to interface with [ibtechnology][]'s [RWD Hitag2][] RFID modules ([data sheet][])
 *
 *  Please note that this module only operates with tags in PASSWORD mode (ie it
 *  doesn't support crypto).
 *
 * [Mark Cheverton]: http://ecafe.org/
 * [ibtechnology]: http://www.ibtechnology.co.uk/
 * [RWD Hitag2]: http://www.ibtechnology.co.uk/products/hitag2-product.htm
 * [data sheet]: http://www.ibtechnology.co.uk/pdf/H2PROT.PDF
 *
*/

#ifndef RWDH2_h
#define RWDH2_h

#include <Arduino.h>
#include <Stream.h>

#define RWDH2_READ_MASK             B00111110
#define RWDH2_WRITE_MASK            B00111110
#define RWDH2_STATUS_MASK           B00101000
#define RWDH2_UID_MASK              B00111110
#define RWDH2_PROGEEPROM_MASK       B00000001

#define RWDH2_ANTENNA_FAULT         B00100000
#define RWDH2_RELAY_ENABLED         B00010000
#define RWDH2_RS232_ERROR           B00001000
#define RWDH2_RX_OKAY               B00000100
#define RWDH2_TAG_OKAY              B00000010
#define RWDH2_EEPROM_ERROR          B00000001

#define RWDH2_OKAY                  B11010110

#define RWDH2_CMD_READ              (0x52)
#define RWDH2_CMD_WRITE             (0x57)
#define RWDH2_CMD_STATUS            (0x53)
#define RWDH2_CMD_UID               (0x55)
#define RWDH2_CMD_MESSAGE           (0x7A)
#define RWDH2_CMD_PROGEEPROM        (0x50)

#define RWDH2_DEFAULT_TIMEOUT       1000
#define RWDH2_BAUD                  9600

class RWDH2 {
  public:
    RWDH2(Stream& serial, int ctsPin);

    bool begin();
    bool begin(bool enable_uid);
    uint8_t getLastAck();

    bool authoriseTag(uint8_t* uid, uint8_t page);
    bool deauthoriseAllTags();
    bool writeEEPROMRWDH2Password(uint8_t* password);
    bool writeTagRWDH2Password(uint8_t* password);
    bool writeEEPROMTagPassword(uint8_t* password);
    bool writeTagTagPassword(uint8_t* password);
    bool writeTagUserData(uint8_t* data);
    bool readTagUserData(uint8_t* data);

    bool getStatus();
    bool getUID(uint8_t* uid);
    bool getIdentifier(uint8_t* identifier);

  private:
    Stream* _serial;
    uint8_t _cts_pin;
    uint8_t _last_ack;

    bool writeTag(uint8_t page, uint8_t* data);
    bool readTag(uint8_t page, uint8_t* data);
    bool writeEEPROM(uint8_t address, uint8_t data);
    bool writeEEPROM(uint8_t address, uint8_t* data, size_t size);

    bool enableUID();
    bool available();
    bool available(uint16_t timeout);
    bool listening();
    bool listening(uint16_t timeout);
    void write(uint8_t cmd);
    bool write(uint8_t cmd, uint8_t mask);
    bool write(uint8_t* cmd, uint8_t mask, size_t size);
    uint8_t read();
    bool read(uint8_t* buffer);
    bool read(uint8_t* buffer, size_t size);
};
#endif
