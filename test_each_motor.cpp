// KONFIGURASI YANG BAWAH
// #define MOTOR_CW LOW 
// #define MOTOR_CCW HIGH

// TOTAL HORIZONTAL PATCH = 2484 ; 2519 ; 2529 ; 
// 2480 ; 2480; ; 2480


#define DIR_PIN 5
#define STEP_PIN 18
#define LIMIT_SWITCH_PIN 22

#define MOTOR_CW HIGH
#define MOTOR_CCW LOW


long targetTicks = 0;
long currentTicks = 0;

const long totalRevSteps = 200L * 8L;        // 1600 microsteps/revolution
const double gearRatio = 49.0 / 80.0;
const double totalRevStepsWithRatio = totalRevSteps / gearRatio;

bool isHomed = true;

// Change these if your switch logic is opposite.
// With INPUT_PULLUP:
// LOW  = switch connected to GND / activated
// HIGH = switch released / not activated
const int LIMIT_ACTIVATED = HIGH;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);

  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  Serial.println();
  Serial.println(F("Starting homing..."));

  // isHomed = resetPos(STEP_PIN, 3000);

  if (isHomed) {
    Serial.println(F("Homing complete."));
    Serial.println(F("Commands:"));
    Serial.println(F("  CW  - run motor clockwise"));
    Serial.println(F("  CCW - run motor counterclockwise"));
  } else {
    Serial.println(F("Homing failed. The motor will not run."));
  }
}

void loop() {
  receiveCommand();

  // Keep trying to home until the limit switch is activated.
  // if (!isHomed) {
  //   Serial.println(F("Homing retry..."));

  //   isHomed = resetPos(STEP_PIN, 3000);

  //   if (isHomed) {
  //     Serial.println(F("Homing complete."));
  //     Serial.println(F("Ready. Type CW or CCW."));
  //   }

  //   delay(10);
  // }
}

void receiveCommand() {
  if (Serial.available() == 0) {
    return;
  }

  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toUpperCase();

  if (command == "CW") {
    if (!isHomed) {
      Serial.println(F("Cannot run: motor is not homed."));
      return;
    }

    Serial.println(F("Running clockwise..."));

    runMotor(STEP_PIN, 250, currentTicks, 0);

    Serial.println(F("CW movement complete."));
  }
  else if (command == "CCW") {
    if (!isHomed) {
      Serial.println(F("Cannot run: motor is not homed."));
      return;
    }

    Serial.println(F("Running counterclockwise..."));

    runMotor(STEP_PIN, 250, currentTicks, 1);

    Serial.println(F("CCW movement complete."));
  }
  else if (command.length() > 0) {
    Serial.print(F("Unknown command: "));
    Serial.println(command);
    Serial.println(F("Use CW or CCW."));
  }
}

bool limitActivated() {
  return digitalRead(LIMIT_SWITCH_PIN) == LIMIT_ACTIVATED;
}

bool resetPos(int pin, long maxSteps) {
  // Move toward the home switch.
  digitalWrite(DIR_PIN, LOW);

  if (limitActivated()) {
    currentTicks = 0;
    targetTicks = 0;
    return true;
  }

  for (long i = 0; i < maxSteps; i++) {
    if (limitActivated()) {
      currentTicks = 0;
      targetTicks = 0;
      return true;
    }

    digitalWrite(pin, HIGH);
    delayMicroseconds(300);

    digitalWrite(pin, LOW);
    delayMicroseconds(100);
  }

  return false;
}

void runMotor(int pin, long steps, long current, int direction) {
  if (steps <= 0) {
    return;
  }

  // direction == 1 gives HIGH; direction == 0 gives LOW.
  digitalWrite(DIR_PIN, direction == 1 ? HIGH : LOW);
  delayMicroseconds(20);

  for (long i = 0; i < steps; i++) {
    // Stop immediately if the limit switch is activated.
    if (limitActivated()) {
      Serial.println(F("Limit switch activated. Motor stopped."));
      return;
    }

    digitalWrite(pin, HIGH);
    delayMicroseconds(400);

    digitalWrite(pin, LOW);
    delayMicroseconds(200);
  }

  if (direction == 1) {
    currentTicks += steps;
  } else {
    currentTicks -= steps;
  }

  targetTicks = currentTicks;
}
