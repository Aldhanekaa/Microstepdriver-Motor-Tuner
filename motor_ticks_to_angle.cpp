#define DIR_PIN 5 // HORIZONTAL
#define STEP_PIN 18 // HORIZONTAL
#define LIMIT_SWITCH_PIN 22



// KONFIGURASI YANG BAWAH
#define MOTOR_CW LOW 
#define MOTOR_CCW HIGH

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

  if (isEnabled && !isHomed) {
    Serial.println(F("Homing retry..."));
    isHomed = resetPos(STEP_PIN, 3000);

    if (isHomed) {
      Serial.println(F("Homing complete."));
      Serial.println(F("Ready. Type CW or CCW."));
    }

    delay(10);
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
  if (steps <= 0) {
    return;
  }

  digitalWrite(DIR_PIN, direction);
  delayMicroseconds(20);

  long i = 0;
  do {
    if ((direction == CW && digitalRead(LIMIT_SWITCH_PIN) == HIGH)) {
      Serial.println(F("Limit switch activated. Motor stopped."));
      isHomed = true;
      currentTicks = 0;
      targetTicks = 0;
      return;
    }

    digitalWrite(pin, HIGH);
    delayMicroseconds(400);
    digitalWrite(pin, LOW);
    delayMicroseconds(200);

    i++;
  } while (i < steps);

  if (direction == MOTOR_CW) {
    currentTicks += steps;
  } else {
    currentTicks -= steps;
  }

  targetTicks = currentTicks;
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
    Serial.println(F("Motor enabled."));
  }
  else if (command == "D") {
    isEnabled = false;
    Serial.println(F("Motor disabled."));
  }
  else if (command == "CW" || command == "CCW") {
    if (!isEnabled) {
      Serial.println(F("Cannot run: motor is disabled. Send E first."));
      return;
    }

    if (!isHomed) {
      Serial.println(F("Cannot run: motor is not homed."));
      return;
    }

    bool clockwise = command == "CW";
    int direction = clockwise ? MOTOR_CW : MOTOR_CCW;
    Serial.println(clockwise ? F("Running clockwise...") : F("Running counterclockwise..."));

    runMotor(STEP_PIN, 250, currentTicks, direction);

    Serial.println(clockwise ? F("CW movement complete.") : F("CCW movement complete."));
    Serial.print(F("Current ticks: "));
    Serial.println(currentTicks);
    Serial.print(F("Current angle: "));
    Serial.print(ticksToAngle(currentTicks, totalRevStepsWithRatio), 2);
    Serial.println(F(" degrees"));
  }
  else if (command.length() > 0) {
    Serial.print(F("Unknown command: "));
    Serial.println(command);
    Serial.println(F("Use E, D, CW, or CCW."));
  }
}
