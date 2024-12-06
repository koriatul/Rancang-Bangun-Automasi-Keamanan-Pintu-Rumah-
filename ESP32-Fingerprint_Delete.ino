#include <Adafruit_Fingerprint.h>


#define mySerial Serial2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  Serial.begin(115200);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nDelete Finger");

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }
}


uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) { //menunggu input
    while (! Serial.available()); //menunggu hingga data tersedia
    num = Serial.parseInt(); // mengubah data yang diterima dari port serial menjadi bilangan bulat
  }
  return num;
}

void loop()                     // run over and over again
{
  Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
  uint8_t id = readnumber();
  if (id == 0) {// ID #0 tidak di izinkan coba lagi!
     return;
  }

  Serial.print("Deleting ID #");
  Serial.println(id);

  deleteFingerprint(id);
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location"); // Tidak dapat menghapus di lokasi tersebut
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash"); // Kesalahan penulisan ke flashdisk
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }

  return p;
}
