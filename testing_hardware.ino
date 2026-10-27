
// CPS607 Assignment 2
// Ultrasonic Sensor + FSM + 3-Second Motor Test

// Ultrasonic sensor
const int trigPin = 11;
const int echoPin = 12;

// IR sensors
const int obstaclePin = 2;
const int linePin = 3;

// Motor pins (Assignment 1)
const byte A_pin1 = 5;
const byte A_pin2 = 6;
const byte B_pin1 = 10;
const byte B_pin2 = 9;

// Distance threshold
const float detectionDistance = 20.0;

// Robot states
enum State {
  SEARCH,
  APPROACH
};

State currentState = SEARCH;

// Stop both motors
void stopMotors() {
  analogWrite(A_pin1, 0);
  analogWrite(A_pin2, 0);
  analogWrite(B_pin1, 0);
  analogWrite(B_pin2, 0);
}

void searchMovement() {
  analogWrite(A_pin1, 255);
  analogWrite(A_pin2, 0);

  analogWrite(B_pin1, 255);
  analogWrite(B_pin2, 0);
}

void setup() {
  Serial.begin(9600);

  // Sensor setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(obstaclePin, INPUT);
  pinMode(linePin, INPUT);

  // Motor setup
  pinMode(A_pin1, OUTPUT);
  pinMode(A_pin2, OUTPUT);
  pinMode(B_pin1, OUTPUT);
  pinMode(B_pin2, OUTPUT);

  stopMotors();

  // Run motors for 3 seconds
  Serial.println("Testing motors...");
  searchMovement();
  delay(3000);
  stopMotors();
  Serial.println("Motor test complete.");
}

void loop() {

  // Read ultrasonic distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);
  float distance = duration * 0.0343 / 2;

  // FSM decision-making
  if (currentState == SEARCH) {

    if (distance > 0 && distance < detectionDistance) {
      currentState = APPROACH;
    }

  } else if (currentState == APPROACH) {

    if (distance == 0 || distance >= detectionDistance) {
      currentState = SEARCH;
    }
  }

  // Display current state
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | State: ");

  if (currentState == SEARCH) {
    Serial.println("SEARCH");
  } else {
    Serial.println("APPROACH");
  }

  delay(200);
}

