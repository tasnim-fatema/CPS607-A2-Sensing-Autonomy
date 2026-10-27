
 // Ultrasonic sensor
const int trigPin = 11;
const int echoPin = 12;

// Binary IR obstacle sensor
const int obstaclePin = 2;

// Line-following IR sensor
const int linePin = 3;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(obstaclePin, INPUT);
  pinMode(linePin, INPUT);
}

void loop() {

  // Ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);
  float distance = duration * 0.0343 / 2;

  // IR sensors
  int obstacle = digitalRead(obstaclePin);
  int line = digitalRead(linePin);

  // Display readings
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm");

  Serial.print(" | Obstacle: ");
  Serial.print(obstacle);

  Serial.print(" | Line: ");
  Serial.println(line);

  delay(200);
}

