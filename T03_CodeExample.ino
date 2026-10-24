const int trigPin = 11;
const int echoPin = 12;

unsigned long previousMillis = 0; // Stores the last time the sensor was read
const long interval = 60;         // Interval at which to ping (milliseconds)

// Filter variables
const int window_size = 3;
float window[window_size]; // Array to store 10 samples
int window_index = 0;       // Keeps track of where we are in the array


void setup() {
  Serial.begin(9600);
  
  //ultrasonic
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  
}

void loop() {
  unsigned long currentMillis = millis(); // Grab the current time

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Save the last time you pinged

    float raw_distance = readSensorData();

    // Overwrite data at index
    window[window_index] = raw_distance;
    window_index = (window_index + 1) % window_size; // Advance index (0 --> 1 --> ... --> 9 --> 0)

    // Calculate the average of the current window
    float sum = 0;
    for (int i = 0; i < window_size; i++) {
      sum += window[i];
    }
    float filtered_distance = sum / window_size;

    
    Serial.print("Raw:");
    Serial.print(raw_distance * 100);
    Serial.print(",");
    Serial.print("Filtered:");
    Serial.println(filtered_distance * 100);
  }
  
}

float readSensorData() {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float distance = pulseIn(echoPin, HIGH, 30000) * 343 * 1e-6 * 0.5; //Equivalent to (340m/s*1us)/2
  return distance;
}
