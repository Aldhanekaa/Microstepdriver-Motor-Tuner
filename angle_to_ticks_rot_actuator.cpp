 #define DIR_PIN 5
#define STEP_PIN 18
#define LIMIT_SWITCH_PIN 22

#define MOTOR_CW LOW
#define MOTOR_CCW HIGH

const long totalRevSteps = 200L * 8L;
const double gearRatio = 49.0 / 80.0;
const double stepsPerOutputRevolution = totalRevSteps / gearRatio;
const int LIMIT_ACTIVATED = HIGH;
const long movementStepCount = 250;

long double currentTicks = 0;
double currentAngle = 0.0;
bool isHomed = false;
bool isEnabled = false;

void setup() {
	Serial.begin(115200);
	Serial.setTimeout(100);

	pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
	pinMode(STEP_PIN, OUTPUT);
	pinMode(DIR_PIN, OUTPUT);

	digitalWrite(STEP_PIN, LOW);
	digitalWrite(DIR_PIN, MOTOR_CW);

	Serial.println(F("Ready. Send E, then H, then a target angle such as 90."));
}

void loop() {
	receiveCommand();
}

double ticksToAngle(long double ticks) {
	return ticks * (360.0 / stepsPerOutputRevolution);
}

long double angleToTicks(double angle) {
	return roundl(angle * (stepsPerOutputRevolution / 360.0));
}

bool limitActivated() {
	return digitalRead(LIMIT_SWITCH_PIN) == LIMIT_ACTIVATED;
}

bool isAngleCommand(const String& command) {
	bool hasDigit = false;
	bool hasDecimalPoint = false;

	for (unsigned int i = 0; i < command.length(); i++) {
		char character = command[i];

		if (i == 0 && (character == '-' || character == '+')) {
			continue;
		}

		if (character >= '0' && character <= '9') {
			hasDigit = true;
		} else if (character == '.' && !hasDecimalPoint) {
			hasDecimalPoint = true;
		} else {
			return false;
		}
	}

	return hasDigit;
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
	} else if (command == "D") {
		isEnabled = false;
		Serial.println(F("Motor disabled."));
	} else if (command == "H") {
		if (!isEnabled) {
			Serial.println(F("Cannot home: motor is disabled. Send E first."));
			return;
		}

		isHomed = homeMotor();
		if (isHomed) {
			currentTicks = 0;
			currentAngle = 0.0;
			printPosition();
		}
	} else if (isAngleCommand(command)) {
		if (!isEnabled) {
			Serial.println(F("Cannot move: motor is disabled. Send E first."));
			return;
		}

		if (!isHomed) {
			Serial.println(F("Cannot move: motor is not homed. Send H first."));
			return;
		}

		moveToAngle(command.toFloat());
	} else if (command.length() > 0) {
		Serial.println(F("Unknown command. Use E, D, H, or a target angle."));
	}
}

void moveToAngle(double targetAngle) {
	long double targetTicks = angleToTicks(-1 * targetAngle);
	long double tickDelta = targetTicks - currentTicks;

	Serial.print(F("Target angle: "));
	Serial.print(targetAngle, 2);
	Serial.print(F(" degrees ("));
	Serial.print((double)targetTicks, 0);
	Serial.println(F(" ticks)"));

	if (tickDelta == 0) {
		Serial.println(F("Already at target angle."));
		printPosition();
		return;
	}

	int direction = tickDelta > 0 ? MOTOR_CW : MOTOR_CCW;
    
    Serial.print(F("Motor direction: "));
	Serial.println(direction == MOTOR_CW ? F("CW") : F("CCW"));

	long steps = (long)roundl(fabsl(tickDelta));
	digitalWrite(DIR_PIN, direction);
	delayMicroseconds(20);

	long step = 0;
	do {


        if (direction == MOTOR_CW && limitActivated()) {
		    Serial.println(F("Limit switch activated. Motor stopped."));
            break;
		}


		digitalWrite(STEP_PIN, HIGH);
		delayMicroseconds(400);
		digitalWrite(STEP_PIN, LOW);
		delayMicroseconds(200);


		step++;
	} while (step < steps);

	currentTicks = targetTicks;
	currentAngle = ticksToAngle(currentTicks);
	printPosition();
}

bool homeMotor() {
	Serial.println(F("Homing..."));
	digitalWrite(DIR_PIN, MOTOR_CW);

	for (long step = 0; step < 3000; step++) {
		if (limitActivated()) {
			Serial.println(F("Homing complete."));
			return true;
		}

		digitalWrite(STEP_PIN, HIGH);
		delayMicroseconds(300);
		digitalWrite(STEP_PIN, LOW);
		delayMicroseconds(100);
	}

	Serial.println(F("Homing failed: limit switch was not reached."));
	return false;
}

void printPosition() {
	Serial.print(F("Current ticks: "));
	Serial.println((double)currentTicks, 0);
	Serial.print(F("Current angle: "));
	Serial.print(currentAngle, 2);
	Serial.println(F(" degrees"));
}
