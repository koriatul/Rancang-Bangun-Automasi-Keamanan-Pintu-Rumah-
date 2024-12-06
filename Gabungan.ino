//memanggil Library yang dibutuhkan
#include <WiFi.h>
#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_PCF8574.h>
#include <ESP32_Servo.h>
#include "CTBot.h" // library diakses directory local

//Inisialisasi Parameter Library
#define mySerial Serial2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_PCF8574 lcd(0x27); // alamat lcd 
CTBot myBot;
Servo myservo;

//Mencantunkan parameter yang dibutuhkan
String ssid   = "Galaxy A53"    ; // REPLACE mySSID WITH YOUR WIFI SSID
String pass   = "qfst2380"; // REPLACE myPassword YOUR WIFI PASSWORD, IF ANY
String token  = "7262134227:AAFWhPKm3aYNXk4_l7nw50VJnF0dHPmooCs"   ; // REPLACE myToken WITH YOUR TELEGRAM BOT TOKEN
long id_token = 341683059; //User ID // menggunakan tipe data long karena datanya bertipe number

//mendefinisikan Variabel Global
bool flag = true; // variabel untuk switch
bool akses; // variabel untuk bot telegram
bool flag_btn = true; //variabel untuk button

//Mendefinisikan nama-nama user yang terdaftar
String user_name[6] = {
  "Prodi1",
  "Prodi2",
  "User Name 3",
};

//Inisialisasi pin yang digunakan
#define pin_relay 18
#define pin_mc38 5
#define pin_servo 19
#define pin_led 2
#define pin_button 4
#define setup_servo 0
#define range_servo 180

void setup(){
  Serial.begin(115200);
  // set the data rate for the sensor serial port
  finger.begin(57600);
  myservo.attach(pin_servo); //menentukan pin servo
  myservo.write(setup_servo); //servo memulai dengan setup awal servo
  lcd.begin(16, 2);
  lcd.setBacklight(1); //Menyalakan backlight LCD
  lcd.clear();
  lcd.setCursor(2, 0); //kolom 2 baris 0
  lcd.print("System Start");
  Serial.println("System Start");
  //Mendefinisikan pin menjadi input atau output
  pinMode(pin_mc38,INPUT_PULLUP); // pullup mengaktifkan resistor internal
  pinMode(pin_relay,OUTPUT);
  pinMode(pin_led, OUTPUT);
  pinMode(pin_button, INPUT_PULLUP);
  
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); } //tidak akan diekseskusi sebelum fingerprint terdeteksi
  }

  Serial.println(F("Reading sensor parameters")); //fungsi F untuk menyimpan dimemori flash
  finger.getParameters();
// menampilkan informasi data fingerprint
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity); // jumlah maksimal sidik jari yg disimpan 
  Serial.print(F("Security level: ")); Serial.println(finger.security_level); //tingkat keamanan untuk mencocokan sidik jari
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX); //alamat komunikasi fingerprint dalam format heksadesimal
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len); // panjang paket data yg digunakan dalam komunikasi ESP32 Mikrokonroler dan Fingerprint
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate); // kecepatan komunikasi serial dalam satuan bps
  
  finger.getTemplateCount();
  //Mengecek apakah di sensor sudah terdaftar id atau belum
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll'.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates"); // menampilkan jumlah sidik jari yg terdaftar
  }
  //Menghubungkan ke Wi-Fi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { //jika tidak bisa terhubung selama sepuluh detik maka esp akan di restart
  Serial.print("•");
  digitalWrite(pin_led,HIGH); // menandakan led menyala mengedip
  delay(500);
  digitalWrite(pin_led,LOW);
  delay(500);
  }
  myBot.wifiConnect(ssid, pass); //Menyambungkan board ke wifi
  myBot.setTelegramToken(token); //Menghubungkan Board Ke token telegram bot
  if (myBot.testConnection())    //Cek apakah bisa terhubung ke bot telegram
    Serial.println("\ntestConnection OK");
  else
    Serial.println("\ntestConnection NOK");
  
  digitalWrite(pin_relay,HIGH); //Setup awal relay dinonaktifkan dengan memberikan sinyal HIGH
  
  //Menampilkan pesan awal pada LCD
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("System Ready");
  Serial.println("System Ready");
  delay(1000);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Tempelkan Jari");
  Serial.println("Tempelkan Jari");
}

void loop(){
  handleBot();
  ReadFinger();
  delay(50);            
  bool sw = digitalRead(pin_mc38);   //Mengecek status sensor MC
  bool pb = digitalRead(pin_button); //Mengecek status Button
  if(sw != flag && akses) {
    digitalWrite(pin_led,HIGH);
    delay(500);
    digitalWrite(pin_led,LOW);
    flag = sw;
    String info;
    if(sw) info = "Pintu Terbuka "; else info = "Pintu Tertutup";
    myBot.sendMessage(id_token, info);
  }
  TBMessage msg;
  if(!pb && !flag_btn){ // Jika tombol ditekan
    flag_btn = true;
    //Menyalakan relay / solenoid
    digitalWrite(pin_relay,LOW);
    Serial.println("Pintu dibuka dari dalam");
    myBot.sendMessage(id_token, "Pintu dibuka dari dalam");
    //Membuka pintu dengan servo
    for(int i=setup_servo; i<range_servo; i++){
      myservo.write(i); 
      delay(10);
    }
        delay(8000);
        for(int i=range_servo; i>setup_servo; i--){
          myservo.write(i); 
          delay(30);
        }
        delay(1000);
        digitalWrite(pin_relay,HIGH);
        myBot.sendMessage(id_token, "Pintu telah ditutup kembali");
  }
  if(pb) flag_btn = false; // Memeriksa apakah tombol dilepaskan
}

//Fungsi untuk membaca fingerprint
void ReadFinger(){
  uint8_t p = finger.getImage();
  if(p == FINGERPRINT_OK){ //deteksi apakah finger terdeteksi
    Serial.println("Sensor detected");
    digitalWrite(pin_led,HIGH);
    delay(500);
    digitalWrite(pin_led,LOW);
    p = finger.image2Tz(); //konversi menjadi template
    if(p == FINGERPRINT_OK){
      p = finger.fingerSearch(); //mencari tempalte sidik jari
      if(p == FINGERPRINT_OK){ //Jika fingerprint terdeteksi dan terdaftar
        flag = true;
        int user_id = finger.fingerID;
        Serial.println("ID Terdeteksi: " + String(user_id));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Akses Diterima ");
        
        //Memberikan akses pintu dengan membuka solenoid selama dua detik
        myBot.sendMessage(id_token, "User " + String(user_name[user_id-1]) + " Login"); //Mengirim pesan ke bot telegram
        digitalWrite(pin_relay,LOW);        
        akses = true;
        delay(2000);
        //Membuka pintu
        Serial.println("Membuka Pintu");
        for(int i=setup_servo; i<range_servo; i++){
          myservo.write(i); 
          delay(10);
        }
        myBot.sendMessage(id_token, "Pintu Terbuka");
        Serial.println("Tunggu 8 Detik");
        delay(8000);
        
        //Menutup Pintu
        Serial.println("Menutup Pintu");
        for(int i=range_servo; i>setup_servo; i--){
          myservo.write(i); 
          delay(20);
        }
        delay(500);
        digitalWrite(pin_relay,HIGH);
        lcd.setCursor(1, 0);
        lcd.print("Tempelkan Jari  ");
      } else { //Jika fingerprint tidak terdaftar
        Serial.println("ID Tidak Terdaftar");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Akses Ditolak ");
        delay(2000);
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Tidak Terdaftar");
        myBot.sendMessage(id_token, "Percobaan Akses Ditolak");
        delay(1000);
        lcd.setCursor(1, 0);
        lcd.print("Tempelkan Jari  ");
      }
    }
  } 
}

void handleBot(){
  TBMessage msg;
  if (CTBotMessageText == myBot.getNewMessage(msg)){
    if (msg.text.equalsIgnoreCase("Buka Pintu")) { // Memeriksa apakah teks pesan yang diterima sama dengan "Buka Pintu" (case-insensitive).
      Serial.println("Membuka Pintu");
      digitalWrite(pin_relay,LOW);
        for(int i=setup_servo; i<range_servo; i++){
          myservo.write(i); 
          delay(10);
        }
        myBot.sendMessage(id_token, "Pintu Berhasil Terbuka");
        delay(8000);
        //Menutup Pintu
        Serial.println("Menutup Pintu");
        for(int i=range_servo; i>setup_servo; i--){
          myservo.write(i); 
          delay(20);
        }
        myBot.sendMessage(id_token, "Pintu Telah Tertutup");
        digitalWrite(pin_relay,HIGH);
      } else 
      {
      myBot.sendMessage(id_token, "Maaf, saya tidak diprogram untuk membalas pesan ini 🙏.");
    }
  }
}
