#define LIMIT_SWITCH_PIN 22

// 22 yang muter dibawah
// 23 yang lifter

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:


  if (digitalRead(LIMIT_SWITCH_PIN) == HIGH)
  {
    Serial.println("Activated!");
  }

  else
  {
    Serial.println("Not activated.");
  }
  
  delay(100);
}



