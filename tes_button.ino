void setup() {
  Serial.begin(115200);
  pinMode(4, INPUT_PULLUP); // restore internal agar datanya stabil 

}

void loop() {
  bool pb = digitalRead(4);
  if(!pb) Serial.println("Ok"); else Serial.println("No");
  delay(100);
}
