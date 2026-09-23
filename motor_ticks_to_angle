#define DIR_PIN 5 // HORIZONTAL
#define STEP_PIN 18 // HORIZONTAL
#define LIMIT_SWITCH_PIN 22


// KONFIGURASI LIFTER
#define MOTOR_CW LOW 
#define MOTOR_CCW HIGH

// KONFIGURASI YANG BAWAH
// #define MOTOR_CW LOW 
// #define MOTOR_CCW HIGH

// TOTAL HORIZONTAL PATCH = 2484 ; 2519 ; 2529 ; 
// 2480 ; 2480; ; 2480


long targetTicks = 0;
long currentTicks = 0;
long totalTicksToHome = 0;

long currentDIR = -1;

const long totalRevSteps = 200L * 8L;                  // 1600 microsteps/rev (motor side)
const double gearRatio = 49.0 / 80.0;                  // use floating-point division
const double totalRevStepsWithRatio = totalRevSteps / gearRatio;

uint8_t currentStep = 0;                               // 1..8
bool isHomed = false;

bool isEnabled = false;


void setup() {
  Serial.begin(115200);

  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(DIR_PIN, MOTOR_CW); 
  // isHomed = resetPos(STEP_PIN, 3000); // try initial homing

  Serial.println(F("Starting..."));
  Serial.print(F("isHomed..."));
  Serial.print(isHomed);


  
}

void loop() {

  receiveCommand();
  if (isEnabled) {

    // Serial.println(F("RUNNING..."));

    // Keep trying to reset position until homed
    if (!isHomed) {
      Serial.println(F("Homing retry..."));

      isHomed = resetPos(STEP_PIN, 3000); // retry homing in small batches
      delay(10);
      return; // skip normal motion until homed
    }

    if (isHomed) {
        Serial.println("ARRIVED AT HOME! ");
        Serial.print("Total Ticks to Home");
      Serial.println(totalTicksToHome);

    }else {
      Serial.print("Is Already Homed? ");
      Serial.println(digitalRead(LIMIT_SWITCH_PIN) == HIGH);

    }
  }else {
    Serial.println("Code is disabled");
  }

}

void receiveCommand() {
  if (Serial.available() == 0) {
    return;
  }

  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toUpperCase();

  if (command == "E") {
    isEnabled = true;
  }
  else if (command == "D") {
    isEnabled = false;
  }
  
}


double calculateCurrentAngle(long ticks, double stepsPerRev) {
  return (ticks / stepsPerRev) * 360.0;
}

long angleToTicks(double angle, double stepsPerRev) {
  return lround(angle * (stepsPerRev / 360.0));
}

double ticksToAngle(long ticks, double stepsPerRev) {
  return ticks * (360.0 / stepsPerRev);
}

// Returns true if homed, false if not yet homed after maxSteps tries
bool resetPos(int pin, long maxSteps) {
  digitalWrite(DIR_PIN, MOTOR_CW); // move toward home direction

  Serial.print("Is Already Homed? ");
  Serial.println(digitalRead(LIMIT_SWITCH_PIN) == HIGH);

  // Already homed
  if (digitalRead(LIMIT_SWITCH_PIN) == HIGH) {
    currentTicks = 0;
    return true;
  }

  for (long i = 0; i < maxSteps; i++) {
    if (digitalRead(LIMIT_SWITCH_PIN) == HIGH) {
      currentTicks = 0;
      totalTicksToHome = i;
      return true;
    }

    digitalWrite(pin, HIGH);
    delayMicroseconds(300);
    digitalWrite(pin, LOW);
    delayMicroseconds(100);
  }

  return false; // not homed yet, caller should retry
}

void runMotor(int pin, long steps, long current, int direction) {
  long delta = steps - current;
  if (delta == 0) return;

  // Set direction based on target relation
  digitalWrite(DIR_PIN, (direction == 1) ? HIGH : LOW);

  long count = labs(delta);
  for (long i = 0; i < count; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(400);
    digitalWrite(pin, LOW);
    delayMicroseconds(200);
  }
}
