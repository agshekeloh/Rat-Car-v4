/*
   @Author: ELEGOO
   @Date: 2019-10-22 11:59:09
   @LastEditTime: 2020-12-18 14:14:35
   @LastEditors: Changhua
   @Description: Smart Robot Car V4.0
   @FilePath:
*/

/*hacking this up to update
 * https://www.instructables.com/Rat-Operated-Vehicle/
 * to work on ELegoo Smart Car v4 chassis, instead of the unavailable v3 in instructions
 * rat car code first stolen from 
 * https://github.com/LElizabethCrawford/RatCarCode/blob/master/Rat_Car_Code.ino
 * and integrated into Elegoo sketch to get driver support, etc.
 * 
 * my particular car uses 
 * TB6612 and MPU6050 chipsets
 * 
 */
 
#include <avr/wdt.h>
#include "ApplicationFunctionSet_xxx0.h"

//Rat Car Functions

volatile int state = LOW;
int EN_LEFT = 6; //enable pin, ENB
int EN_RIGHT = 5; //ENA

int IN1 = 10; //Direction pins left
int IN2 = 9;
int IN3 = 8; //Direction pins right
int IN4 = 7;

int ABS_LEFT = 150; //Use an even number.
int ABS_RIGHT = 150; //Use an even number.
int CURRENT_LEFT = 0;
int CURRENT_RIGHT = 0;
int TARGET_LEFT = 0;
int TARGET_RIGHT = 0;
int TURN_OFFSET = 50;  // Adjust how sharp it turns.
// 50 makes for just a slight turn.  Much more, and the differences between left
// and right motors makes one turn direction sharper than the other.

int ACCELERATION = 10;  // 1 feels laggy, 50 feels lurchy.
bool LEFT_FORWARD = true;
bool RIGHT_FORWARD = true;

const byte forwardInputPin = A0;     // the forward touch sensor
const byte leftInputPin = A1;     // the left touch sensor
const byte rightInputPin = A2;     // the right touch sensor

int forwardInputVal = 0;
int leftInputVal = 0;
int rightInputVal = 0;

int threshold = 500;  // minimum ADC reading for touch sensor.

int moveState = 0;    // 0:stop, 1: forward, 2: turn left, 3: rotate left, 4: turn right, 5: rotate right.
int oldMoveState = 0;

int checkInputs()
{
  // 0:stop, 1: forward, 2: turn left, 3: rotate left, 4: turn right, 5: rotate right.
  leftInputVal = analogRead(leftInputPin);
  delay(10);
  leftInputVal = analogRead(leftInputPin);

  forwardInputVal = analogRead(forwardInputPin);
  delay(10);
  forwardInputVal = analogRead(forwardInputPin);

  rightInputVal = analogRead(rightInputPin);
  delay(10);
  rightInputVal = analogRead(rightInputPin);

  // Serial.print("forward: ");
  // Serial.println(forwardInputVal);

  // Serial.print("left: ");
  // Serial.println(leftInputVal);

  // Serial.print("right: ");
  // Serial.println(rightInputVal);
  delay(500);

  if (leftInputVal > threshold && forwardInputVal > threshold) {
    moveState = 2;
  } else if (rightInputVal > threshold && forwardInputVal > threshold) {
    moveState = 4;
  } else if (leftInputVal > threshold) {
    moveState = 3;
  } else if (rightInputVal > threshold) {
    moveState = 5;
  } else if (forwardInputVal > threshold) {
    moveState = 1;
  } else {
    moveState = 0;  // Touching none is stop.
  }
  //moveState = 4;
  return (moveState);
}

void setTarget(int) // Set target speed for the wheels.
{
  // 0:stop, 1: forward, 2: turn left, 3: rotate left, 4: turn right, 5: rotate right.

  switch (moveState) {
    case 0:
      TARGET_LEFT = 0;
      TARGET_RIGHT = 0;
      break;

    case 1:
      //Serial.println("forward");
      TARGET_LEFT = ABS_LEFT;
      TARGET_RIGHT = ABS_RIGHT;
      break;

    case 2:
      TARGET_LEFT = ABS_LEFT - TURN_OFFSET;
      TARGET_RIGHT = ABS_RIGHT;
      break;

    case 3:
      TARGET_LEFT = ABS_LEFT * -1;
      TARGET_RIGHT = ABS_RIGHT;
      break;

    case 4:
      TARGET_LEFT = ABS_LEFT;
      TARGET_RIGHT = ABS_RIGHT - TURN_OFFSET;
      break;

    case 5:
      TARGET_LEFT = ABS_LEFT;
      TARGET_RIGHT = ABS_RIGHT * -1;
      break;
  }
}

void move()
{
  if (CURRENT_RIGHT < TARGET_RIGHT) {
    CURRENT_RIGHT += ACCELERATION;
  }
  if (CURRENT_RIGHT > TARGET_RIGHT) {
    CURRENT_RIGHT -= ACCELERATION;
  }

  if (CURRENT_LEFT < TARGET_LEFT) {
    CURRENT_LEFT += ACCELERATION;
  }
  if (CURRENT_LEFT > TARGET_LEFT) {
    CURRENT_LEFT -= ACCELERATION;
  }

  // If necessary, switch the left wheels direction.
  if (CURRENT_LEFT < 0 && LEFT_FORWARD) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);        // left wheels go backward
    LEFT_FORWARD = false;
  }
  if (CURRENT_LEFT > 0 && !LEFT_FORWARD) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);        // left wheels go forward
    LEFT_FORWARD = true;
  }

  // If necessary, switch the right wheels direction.
  if (CURRENT_RIGHT < 0 && RIGHT_FORWARD) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);        // right wheels go backward
    RIGHT_FORWARD = false;
  }
  if (CURRENT_RIGHT > 0 && !RIGHT_FORWARD) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);        // right wheels go forward
    RIGHT_FORWARD = true;
  }

  analogWrite(EN_RIGHT, abs(CURRENT_RIGHT));
  analogWrite(EN_LEFT, abs(CURRENT_LEFT));

  //delay(2);  // enough delay from sensors.
  //    Serial.print("left");
  //    Serial.println(CURRENT_LEFT);
  //    Serial.print("right");
  //  Serial.println(CURRENT_RIGHT);
}


void setup()
{
  // put your setup code here, to run once:

  Serial.begin(9600);  // Serial monitor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(EN_LEFT, OUTPUT);
  pinMode(EN_RIGHT, OUTPUT);
  pinMode(forwardInputPin, INPUT);
  pinMode(leftInputPin, INPUT);
  pinMode(rightInputPin, INPUT);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);        // left wheels go forward
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);        // right wheels go forward
  
  Application_FunctionSet.ApplicationFunctionSet_Init();
  wdt_enable(WDTO_2S);
}

void loop()
{
  //put your main code here, to run repeatedly :
  moveState = checkInputs();
  if (oldMoveState != moveState)
    setTarget(moveState);

  move();

  oldMoveState = moveState;


  wdt_reset();
  Application_FunctionSet.ApplicationFunctionSet_SensorDataUpdate();
  Application_FunctionSet.ApplicationFunctionSet_KeyCommand();
  Application_FunctionSet.ApplicationFunctionSet_RGB();
  //  Application_FunctionSet.ApplicationFunctionSet_Follow();
  Application_FunctionSet.ApplicationFunctionSet_Obstacle();
  //  Application_FunctionSet.ApplicationFunctionSet_Tracking();
  Application_FunctionSet.ApplicationFunctionSet_Rocker();
  Application_FunctionSet.ApplicationFunctionSet_Standby();
  Application_FunctionSet.ApplicationFunctionSet_IRrecv();
  Application_FunctionSet.ApplicationFunctionSet_SerialPortDataAnalysis();

  Application_FunctionSet.CMD_ServoControl_xxx0();
  Application_FunctionSet.CMD_MotorControl_xxx0();
  Application_FunctionSet.CMD_CarControlTimeLimit_xxx0();
  Application_FunctionSet.CMD_CarControlNoTimeLimit_xxx0();
  Application_FunctionSet.CMD_MotorControlSpeed_xxx0();
  Application_FunctionSet.CMD_LightingControlTimeLimit_xxx0();
  Application_FunctionSet.CMD_LightingControlNoTimeLimit_xxx0();
  Application_FunctionSet.CMD_ClearAllFunctions_xxx0();
}
