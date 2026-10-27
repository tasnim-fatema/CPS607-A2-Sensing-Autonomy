
// CPS607 Assignment 2 - Autonomous Push-the-Box Robot
// Based on Dr. Sven Lilge's L09 and L10 lectures
//
// States:
// SEARCH -> FORWARD -> IDENTIFY -> PUSH
// Boundary during movement -> RECOVER
// Boundary during PUSH -> DONE
//
// IMPORTANT: Calibrate motor directions and IR values
// before running the complete robot.

const byte trigPin = 11;
const byte echoPin = 12;
const byte obstaclePin = 2;
const byte linePin = 3;

// L9110 motor driver
const byte A_pin1 = 5;
const byte A_pin2 = 6;
const byte B_pin1 = 10;
const byte B_pin2 = 9;

// Motor settings
const int searchPWM = 120;
const int forwardPWM = 150;
const int pushPWM = 180;
const int recoverPWM = 150;

// IMPORTANT: Adjust after motor-direction testing
const bool reverseMotorA = false;
const bool reverseMotorB = true;

// Sensor thresholds
const float detectThreshold = 20.0;
const float pushThreshold = 6.0;

// IMPORTANT: Determine these experimentally
const int WHITE_DETECTED = LOW;
const int BLACK_LINE_DETECTED = LOW;

// Timings
const unsigned long sensorInterval = 60;
const unsigned long identifyTime = 400;
const unsigned long reverseTime = 500;
const unsigned long turnTime = 400;
const unsigned long pushTimeout = 5000;
const unsigned long lostTargetTime = 500;
const unsigned long ignoreBoxTime = 1000;

// FSM
enum State {
  SEARCH,
  FORWARD,
  IDENTIFY,
  PUSH,
  RECOVER,
  DONE
};

State currentState = SEARCH;

unsigned long stateStartTime = 0;
unsigned long lastSensorTime = 0;
unsigned long lastTargetSeen = 0;
unsigned long ignoreUntil = 0;

float distance = 0;
int obstacleReading = HIGH;
int lineReading = HIGH;

bool boxDetected = false;
bool boundaryDetected = false;

// Recovery phase
enum RecoveryPhase {
  BACKUP,
  TURN
};

RecoveryPhase recoveryPhase = BACKUP;

// ------------------------------------------------
// MOTOR FUNCTIONS
// ------------------------------------------------

void driveMotor(byte pin1, byte pin2,
                int speedValue, bool reversed) {

  speedValue = constrain(speedValue, -255, 255);

  if (reversed) {
    speedValue = -speedValue;
  }

  if (speedValue > 0) {
    analogWrite(pin1, speedValue);
    analogWrite(pin2, 0);
  }
  else if (speedValue < 0) {
    analogWrite(pin1, 0);
    analogWrite(pin2, -speedValue);
  }
  else {
    analogWrite(pin1, 0);
    analogWrite(pin2, 0);
  }
}

void setMotors(int speedA, int speedB) {
  driveMotor(A_pin1, A_pin2, speedA, reverseMotorA);
  driveMotor(B_pin1, B_pin2, speedB, reverseMotorB);
}

void stopMotors() {
  setMotors(0, 0);
}

void searchMovement() {
  // Rotate in place
  setMotors(searchPWM, -searchPWM);
}

void forwardMovement() {
  setMotors(forwardPWM, forwardPWM);
}

void pushMovement() {
  setMotors(pushPWM, pushPWM);
}

void reverseMovement() {
  setMotors(-recoverPWM, -recoverPWM);
}

void turnMovement() {
  setMotors(recoverPWM, -recoverPWM);
}

// ------------------------------------------------
// ULTRASONIC SENSOR
// ------------------------------------------------

float readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration =
      pulseIn(echoPin, HIGH, 25000);

  if (duration == 0) {
    return -1.0;
  }

  return duration * 0.0343 / 2.0;
}

// ------------------------------------------------
// STATE MANAGEMENT
// ------------------------------------------------

void changeState(State newState) {
  if (newState == currentState) {
    return;
  }

  stopMotors();

  currentState = newState;
  stateStartTime = millis();

  Serial.print("NEW STATE: ");

  switch (currentState) {
    case SEARCH:
      Serial.println("SEARCH");
      break;

    case FORWARD:
      Serial.println("FORWARD");
      lastTargetSeen = millis();
      break;

    case IDENTIFY:
      Serial.println("IDENTIFY");
      break;

    case PUSH:
      Serial.println("PUSH");
      break;

    case RECOVER:
      Serial.println("RECOVER");
      recoveryPhase = BACKUP;
      break;

    case DONE:
      Serial.println("DONE");
      break;
  }
}

// ------------------------------------------------
// SENSOR UPDATE
// ------------------------------------------------

void updateSensors() {
  distance = readDistance();

  obstacleReading = digitalRead(obstaclePin);
  lineReading = digitalRead(linePin);

  boxDetected =
      (distance > 0 &&
       distance < detectThreshold);

  boundaryDetected =
      (lineReading == BLACK_LINE_DETECTED);

  Serial.print("Distance: ");
  Serial.print(distance);

  Serial.print(" | IR: ");
  Serial.print(obstacleReading);

  Serial.print(" | Line: ");
  Serial.print(lineReading);

  Serial.print(" | State: ");
  Serial.println(currentState);
}

// ------------------------------------------------
// SETUP
// ------------------------------------------------

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(obstaclePin, INPUT);
  pinMode(linePin, INPUT);

  pinMode(A_pin1, OUTPUT);
  pinMode(A_pin2, OUTPUT);
  pinMode(B_pin1, OUTPUT);
  pinMode(B_pin2, OUTPUT);

  stopMotors();

  currentState = SEARCH;
  stateStartTime = millis();

  Serial.println("CPS607 Assignment 2");
  Serial.println("Autonomous Robot Initialized");
}

// ------------------------------------------------
// MAIN LOOP: SENSE -> PLAN -> ACT
// ------------------------------------------------

void loop() {

  unsigned long now = millis();

  // SENSE
  if (now - lastSensorTime >= sensorInterval) {
    lastSensorTime = now;
    updateSensors();
  }

  // Highest priority: boundary safety
  if (boundaryDetected &&
      currentState != RECOVER &&
      currentState != DONE) {

    if (currentState == PUSH) {
      changeState(DONE);
    }
    else {
      changeState(RECOVER);
    }
  }

  // PLAN AND ACT
  switch (currentState) {

    // ------------------------------------
    // SEARCH
    // ------------------------------------
    case SEARCH:

      if (now < ignoreUntil) {
        searchMovement();
        break;
      }

      if (boxDetected) {
        changeState(FORWARD);
      }
      else {
        searchMovement();
      }

      break;

    // ------------------------------------
    // FORWARD
    // ------------------------------------
    case FORWARD:

      if (boxDetected) {
        lastTargetSeen = now;
      }

      if (distance > 0 &&
          distance <= pushThreshold) {

        changeState(IDENTIFY);
      }
      else if (now - lastTargetSeen >
               lostTargetTime) {

        changeState(SEARCH);
      }
      else {
        forwardMovement();
      }

      break;

    // ------------------------------------
    // IDENTIFY
    // ------------------------------------
    case IDENTIFY:

      stopMotors();

      if (now - stateStartTime >= identifyTime) {

        if (obstacleReading == WHITE_DETECTED) {
          Serial.println("WHITE BOX DETECTED");
          changeState(PUSH);
        }
        else {
          Serial.println("BLACK BOX - IGNORE");
          ignoreUntil = now + ignoreBoxTime;
          changeState(RECOVER);
        }
      }

      break;

    // ------------------------------------
    // PUSH
    // ------------------------------------
    case PUSH:

      if (now - stateStartTime > pushTimeout) {
        changeState(RECOVER);
      }
      else {
        pushMovement();
      }

      break;

    // ------------------------------------
    // RECOVER
    // ------------------------------------
    case RECOVER:

      if (recoveryPhase == BACKUP) {

        if (now - stateStartTime < reverseTime) {
          reverseMovement();
        }
        else {
          stopMotors();
          recoveryPhase = TURN;
          stateStartTime = now;
        }
      }
      else {

        if (now - stateStartTime < turnTime) {
          turnMovement();
        }
        else {
          changeState(SEARCH);
        }
      }

      break;

    // ------------------------------------
    // DONE
    // ------------------------------------
    case DONE:

      stopMotors();
      break;
  }
}

