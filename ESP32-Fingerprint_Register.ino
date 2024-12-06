#include <Adafruit_Fingerprint.h>
#define mySerial Serial2 
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); 

uint8_t id; 

void setup()
{
  Serial.begin(115200);
  while (!Serial);  //loop koneksi serial 
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // mengatur laju data untuk port serial sensor
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); } 
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
 // menampilkan informasi pada fingerprint
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity); //jumlah maksimal sidik jari yg disimpan oleh fingerprint
  Serial.print(F("Security level: ")); Serial.println(finger.security_level); // tingkat keamanan fingerprint
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX); // alamat komunikasi fingerprint dalam format desimal
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len); //panjang paket data yg digunakan dalam komunikasi esp32 dan fingerprint
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
}

//membaca input id
uint8_t readnumber(void) { 
  uint8_t num = 0;

  while (num == 0) { //menunggu input
    while (! Serial.available()); 
    num = Serial.parseInt(); 
  }
  return num;
}

void loop()    // run over and over again
{
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readnumber();
  if (id == 0) {// ID #0 tidak di izinkan coba lagi!
     return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (!  getFingerprintEnroll() );
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) { 
    case FINGERPRINT_OK:
      Serial.println("Image taken"); 
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted"); 
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy"); 
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features"); 
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) { // mengulangi jika tidak ada datanya
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");// tempel kembali sidik jari
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted"); 
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy"); 
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features"); 
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id); //membuat model sidik jari

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!"); // store data fingerprint
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash"); 
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}
