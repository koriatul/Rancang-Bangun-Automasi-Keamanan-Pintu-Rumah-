#define pin_mc38 5
void setup() {
  pinMode(pin_mc38, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  Serial.println(digitalRead(pin_mc38));
  delay(100);
}
