#include "HX711.h"
HX711 scale(4,5);

float calibration_factor = -900; // This must be tuned for each load cell
float units; // Variable for storing current measurement
float MOSFET = 9; // this is output pin for MOSFET
int timer = 250; // delay timing (reduced from 1300 for faster response)
int range = 10; // +/- range in which weight can bounce around
int nominal = 0;
int lowerLimit = 0;
int history = 3; // reduced from 5 for faster response
int lastReadings[] = {
  0,0,0
}; // Array for last 3 measurements
int scorecard[] = {
  0,0,0
}; // Tracks cycles where weight decreased - 3 consecutive cycles results in music cut off


void setup() {
  Serial.begin(9600);
  scale.set_scale();
  scale.tare();
  pinMode(MOSFET, OUTPUT);
  digitalWrite(MOSFET, HIGH); // close switch to turn on speakers 
}

void loop() {
  scale.set_scale(calibration_factor);
  units = scale.get_units(), 10;
   
  Serial.print(units);
  Serial.print(" grams"); 

  // bump array to add latest measurement and remove oldest
  for (int n = 0; n < (history-1); n++) {
    lastReadings[n] = lastReadings[n+1];
  }
  lastReadings[(history-1)] = units;

  // calculate the rolling average
  int total = 0;
  int average = 0;
  for (int i=0; i<=(history-1); i++){
    total = total + lastReadings[i];
  }
  average = (total/history);
  Serial.print(" average: ");
  Serial.print(average);
  Serial.println();

  /* Update scorecard */
  for (int j = 0; j < (history-1); j++) {
    scorecard[j] = scorecard[j+1];
  }

  /* If latest average value is greater than nominal value, set new nominal value */
  if (average > nominal) {
    nominal = average;
    lowerLimit = nominal - range;
    scorecard[(history-1)] = 0;
  }
  else if (units > lowerLimit) {
    scorecard[(history-1)] = 0;
  }
  else if (average < lowerLimit) {
    scorecard[(history-1)] = 1;
  }
  else {
    scorecard[(history-1)] = 0;
  }

  int score = 0; // score sum, reset each cycle
  // Calculate score
  for (int k=0; k<=(history-1); k++){
    score = score + scorecard[k];
  }
  
  if(score == history) {
    digitalWrite(MOSFET, LOW);
    nominal = 0;  // Reset so it can detect phone return
    lowerLimit = 0;
    Serial.print("OFF");
    Serial.println();
  }
  else{
    digitalWrite(MOSFET,HIGH);
    Serial.print("ON");
    Serial.println();
  }  
  Serial.print(score);
  Serial.println();
  delay(timer); // delay between measurements
}
