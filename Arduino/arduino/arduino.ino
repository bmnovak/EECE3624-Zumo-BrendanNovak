#include <Wire.h>
#include <ZumoShield.h>

#define LED 13

// Tuned parameters
#define QTR_THRESHOLD      1500 // ms
#define PROXIMITY_THRESHOLD 250 // (higher = closer)

#define REVERSE_SPEED       200 
#define TURN_SPEED          200 
#define FORWARD_SPEED       200 
#define REVERSE_DURATION    200 // ms
#define TURN_DURATION       300 // ms

#define DISTANCE_SENSOR_PIN A1

ZumoBuzzer buzzer;
ZumoMotors motors;

#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];
ZumoReflectanceSensorArray sensors(QTR_NO_EMITTER_PIN);

char currentCmd = 'S';

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(DISTANCE_SENSOR_PIN, INPUT);
  Serial.begin(9600);

  digitalWrite(LED, HIGH);
  for (int i = 0; i < 3; i++)
  {
    delay(1000);
    buzzer.playNote(NOTE_G(3), 200, 15);
  }
  delay(1000);
  buzzer.playNote(NOTE_G(4), 500, 15);
  delay(1000);
  digitalWrite(LED, LOW);
}

void loop()
{
  sensors.read(sensor_values);

  int opponentDistance = analogRead(DISTANCE_SENSOR_PIN);

  // PRIORITY 1: Border Safety 
  if (sensor_values[0] > QTR_THRESHOLD)
  {
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(TURN_DURATION);
    currentCmd = 'S';
  }
  else if (sensor_values[5] > QTR_THRESHOLD)
  {
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(TURN_DURATION);
    currentCmd = 'S';
  }
  else
  {
    // PRIORITY 2: Proximity Warning Sound Check
    if (opponentDistance > PROXIMITY_THRESHOLD)
    {
      if (!buzzer.isPlaying())
      {
        buzzer.playNote(NOTE_A(5), 50, 15);
      }
    }

    // PRIORITY 3: Joystick Teleoperation
    if (Serial.available() > 0)
    {
      char incoming = Serial.read();
      if (incoming == 'F' || incoming == 'B' || incoming == 'L' || incoming == 'R' || incoming == 'S')
      {
        currentCmd = incoming;
      }
    }

    switch (currentCmd)
    {
      case 'F':
        motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
        break;
      case 'B':
        motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
        break;
      case 'L':
        motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
        break;
      case 'R':
        motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
        break;
      case 'S':
      default:
        motors.setSpeeds(0, 0);
        break;
    }
  }
}
