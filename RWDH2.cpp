/**
 *
 * @file        RWDH2.cpp
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

#include "RWDH2.h"

/**
 * @brief Instantiate a RWDH2 class
 *
 * @param serial  HardwareSerial or SoftwareSerial object
 * @param cts_pin Pin used for CTS flow control
 */
RWDH2::RWDH2(Stream &serial, int cts_pin) {
  _serial = &serial;
  _cts_pin = cts_pin;
  _last_ack = RWDH2_OKAY;
}

/**
 * @brief Sets up the hardware
 *
 * @returns True if commands could be sent to device
 */
bool RWDH2::begin() {
  return begin(false);
}

/**
 * @brief Sets up the hardware
 *
 * @param open_access If true, sets the RWDH2 up for all tags to be authorised
 * @returns True if commands could be sent to device
 */
bool RWDH2::begin(bool enable_uid) {
  pinMode(_cts_pin, INPUT);

  return (enable_uid ? enableUID() : true);
}

/**
 * @brief Get the last return code from the RWDH2
 *
 * @returns The byte returned when the last command was executed
 */
uint8_t RWDH2::getLastAck() {
  return _last_ack;
}

/**
 * @brief Authorise a tag UID with the RWD
 *
 * @param uid The UID of the tag to authorise
 * @param page The memory slot to use 0-59
 * @returns True if commands could be sent to device
 */
bool RWDH2::authoriseTag(uint8_t* uid, uint8_t page) {
  return writeEEPROM(12+4*page, uid, 4);
}

/**
 * @brief Deauthorise all tags by clearing the EEPROM memory
 *        Warning: This is slow at 9600 baud
 *
 * @returns True if commands could be sent to device
 */
bool RWDH2::deauthoriseAllTags() {
  uint8_t data[] = { 0x0, 0x0, 0x0, 0x0 };
  for (uint8_t page = 0; page < 60; page++) {
    if (!writeEEPROM(12+4*page, data, 4)) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Write a new RWD password to the RWD's EEPROM
 *
 * @param password The RWD 4 byte password
 * @returns True if commands could be sent to device
 */
bool RWDH2::writeEEPROMRWDH2Password(uint8_t* password) {
  return writeEEPROM(4, password, 4);
}

/**
 * @brief Write a new RWD password to the tag
 *        Warning: There is no way to reset a tag if you lock yourself out
 *
 * @param password The 4 byte RWD password
 * @returns True if commands could be sent to device
 */
bool RWDH2::writeTagRWDH2Password(uint8_t* password) {
  return writeTag(1, password);
}

/**
 * @brief Write a new tag password to the RWD's EEPROM
 *
 * @param password The 3 byte tag password
 * @returns True if commands could be sent to device
 */
bool RWDH2::writeEEPROMTagPassword(uint8_t* password) {
  return writeEEPROM(9, password, 3);
}

/**
 * @brief Write a new tag password to the tag
 *
 * @param password The 3 byte tag password
 * @returns True if commands could be sent to device
 */
bool RWDH2::writeTagTagPassword(uint8_t* password) {
  uint8_t data[] = { 0x06, password[0], password[1], password[2] };
  return writeTag(3, data);
}

/**
 * @brief Write the user data to the tag
 *
 * @param data Array of 16 bytes of user data
 * @returns True if commands could be sent to device
 */
bool RWDH2::writeTagUserData(uint8_t* data) {
  for (uint8_t i = 0; i < 4; i++) {
    if (!writeTag(4+i, &data[i*4])) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Read the tag user data
 *
 * @param data A 16 byte array to store the data
 * @returns True if commands could be sent to the device
 */
bool RWDH2::readTagUserData(uint8_t* data) {
  for (uint8_t i = 0; i < 4; i++) {
    if (!readTag(4+i, &data[i*4])) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Get the status of the RWD
 *
 * @returns True if commands could be sent to device
 */
bool RWDH2::getStatus() {
  return (write(RWDH2_CMD_STATUS, RWDH2_STATUS_MASK));
}

/**
 * @brief Get the UID of the tag in the reader
 *
 * @param uid Array to store 4 bytes of tag UID
 * @returns True if tag could be read
 */
bool RWDH2::getUID(uint8_t* uid) {
  return (write(RWDH2_CMD_UID, RWDH2_UID_MASK) ? read(uid, 4) : false);
}

/**
 * @brief Get the RWD identifier string
 *        e.g. “a IDE RWD H2 (SECx V1.xx) DD/MM/YY”
 *
 * @returns True if commands could be sent to device
 */
bool RWDH2::getIdentifier(uint8_t* identifier) {
  if (listening()) {
    write(RWDH2_CMD_MESSAGE);
    return (read(identifier));
  } else {
    return false;
  }
}

/*
 * Private methods
 */

/**
 * @brief Write data to a 32bit page of tag memory
 *        ** Be careful you don't trash your tag - [RTFM][data sheet] **
 *
 * @param page Page address 0-7
 * @param data 4 byte array of data to be written to page
 * @returns True if commands could be sent to device
 */
bool RWDH2::writeTag(uint8_t page, uint8_t* data) {
  uint8_t c[] = { RWDH2_CMD_WRITE, page, data[0], data[1], data[2], data[3] };
  return write(c, RWDH2_WRITE_MASK, sizeof(c));
}

/**
 * @brief Read data from a 32bit page of tag memory
 *
 * @param page Page address 0-7
 * @param data Array to store 4 bytes of data returned
 * @returns True if commands could be sent to device
 */
bool RWDH2::readTag(uint8_t page, uint8_t* data) {
  uint8_t c[] = { RWDH2_CMD_READ, page };
  return (write(c, RWDH2_READ_MASK, sizeof(c)) ? read(data, 4) : false);
}

/**
 * @brief Program the RWD's EEPROM with a byte of data
 *
 * @param address EEPROM memory location 0-255
 * @param data A byte of data to be written
 * @returns True if commands could be sent to device
 */
bool RWDH2::writeEEPROM(uint8_t address, uint8_t data) {
  return writeEEPROM(address, &data, 1);
}

/**
 * @brief Program the RWD's EEPROM with multiple bytes of data
 *
 * @param start_address Starting EEPROM memory location 0-255
 * @param data An array of data to be written
 * @param size Size of the data array to be written
 * @returns True if commands could be sent to device
 */
bool RWDH2::writeEEPROM(uint8_t start_address, uint8_t* data, size_t size) {
  if (listening()) {
    for (uint8_t i = 0; i < size; i++) {
      uint8_t c[] = { RWDH2_CMD_PROGEEPROM, start_address + i, data[i] };
      if (!write(c, RWDH2_PROGEEPROM_MASK, sizeof(c)))
        return false;
    }
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Fill EEPROM memory 0xC-0xF with 0xFF to indicate blank auth list and allow querying of UID with getUID()
 *
 * @returns True if commands could be sent to device
 */
bool RWDH2::enableUID() {
  uint8_t data[] = { 0xFF, 0xFF, 0xFF, 0xFF };
  return writeEEPROM(0xC, data, sizeof(data));
}

/**
 * @brief Send a command to the RWD
 *
 * @param cmd Command byte to be sent
 */
void RWDH2::write(uint8_t cmd) {
  _serial->write(cmd);
}

/**
 * @brief Write a command to the RWD and check the return is expected using a bit mask
 *
 * @param cmd  Command byte to be sent
 * @param mask Bit mask for the return to allow comparison with RWD_OKAY to indicate success
 * @returns True if commands executed successfully
 */
bool RWDH2::write(uint8_t cmd, uint8_t mask) {
  write(&cmd, mask, 1);
}

/**
 * @brief Write an array of commands to the RWD and check the return is expected using a bit mask
 *
 * @param cmd  Array of commands to be sent
 * @param mask Bit mask for the return to allow comparison with RWD_OKAY to indicate success
 * @param size Size of command array
 * @returns True if commands executed successfully
 */
bool RWDH2::write(uint8_t* cmd, uint8_t mask, size_t size) {
  if (listening()) {
    for (uint8_t i = 0; i < size; i++) {
      write(cmd[i]);
    }

    return (available() ? (((_last_ack = read()) & mask) == (RWDH2_OKAY & mask)) : false);
  }
}

/**
 * @brief Read a byte of data from the RWD serial connection
 *
 * @returns Byte of data read
 */
uint8_t RWDH2::read() {
  // The arduino is faster than the module - introduce some delay
  delay(1);
  return _serial->read();
}

/**
 * @brief Read several bytes of data from the RWD serial connection
 *
 * @param buffer The array to store the data
 * @returns True if data read
 */
bool RWDH2::read(uint8_t* buffer) {
  uint8_t i = 0;

  if (available()) {
    while (_serial->available()) {
      buffer[i++] = read();
    }
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Read size number of bytes of data from the RWD serial connection
 *
 * @param buffer The array to store the data
 * @param size   The number of bytes to read
 * @returns True if data read
 */
bool RWDH2::read(uint8_t* buffer, size_t size) {
  if (available()) {
    for (uint8_t i = 0; i < size; i++) {
      buffer[i] = read();
    }
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Wait until the RWD has some data to send through the serial connection
 *
 * @returns True when data is available to read
 */
bool RWDH2::available() {
  return available(RWDH2_DEFAULT_TIMEOUT);
}

/**
 * @brief Wait until the RWD has some data to send through the serial connection
 *
 * @param timeout Number of ms to wait
 * @returns True when data is available to read, false if no data was available within the timeout
 */
bool RWDH2::available(uint16_t timeout) {
  uint16_t timer = 0;

  while (!_serial->available()) {
    if (timeout > 0) {
      if (timer < timeout) {
        timer += 5;
      } else {
        Serial.println("RWDH2::available timeout");
        return false;
      }
    }
    delay(5);
  }
  return true;
}

/**
 * @brief Wait until CTS line goes HIGH indicating the RWD is ready to accept data
 *
 * @returns True when RWD is listening for commands
 */
bool RWDH2::listening() {
  return listening(RWDH2_DEFAULT_TIMEOUT);
}

/**
 * @brief Wait until CTS line goes HIGH indicating the RWD is ready to accept data
 *
 * @param timeout Number of ms to wait
 * @returns True when RWD is listening for commands, false if not listening within the timeout
 */
bool RWDH2::listening(uint16_t timeout) {
  uint16_t timer = 0;

  while (digitalRead(_cts_pin) == HIGH) {
    if (timeout > 0) {
      if (timer < timeout) {
        timer += 5;
      } else {
        Serial.println("RWDH2::listening timeout");
        return false;
      }
    }
    delay(5);
  }
  return true;
}
