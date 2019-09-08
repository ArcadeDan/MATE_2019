#include <stdlib.h>
#include <iostream>
#include <string>

#include ".\Headers\Gamepad.h"
#include ".\Headers\SerialPort.h"

using namespace std;

char output[MAX_DATA_LENGTH];
char incomingData[MAX_DATA_LENGTH];

// Change the name of the port with the port name of your computer
// Must remember that the backslashes are essential so do not remove them
const char* port = "\\\\.\\COM7";
SerialPort arduino(port);
Gamepad gamepad = Gamepad(1);

void sendData(string data)
{
  char* charArray = new char[data.size()];
  copy(data.begin(), data.end(), charArray);

  arduino.writeSerialPort(charArray, data.size() - 1);
  Sleep(180);
  arduino.readSerialPort(output, data.size() - 1);

  cout << ">>       " << output << endl << endl;

  delete[] charArray;
}

double convertRange(double oldMin, double oldMax, double newMin, double newMax,
                    double oldValue)
{
  double oldRange = (oldMax - oldMin);
  double newRange = (newMax - newMin);

  return (((oldValue - oldMin) * newRange) / oldRange) + newMin;
}

void drive()
{
  string data;

  double FWD = gamepad.leftStick_Y();
  double STR = gamepad.leftStick_X();
  double RCCW = gamepad.rightStick_X();

  // Will not reach full power diagonally because of controller input (depending
  // on controller)
  // Add IMU input to rad45 for field orientation
  const double rad45 = 45.0 * 3.14159 / 180.0;
  double FR = (-STR * sin(rad45) + FWD * cos(rad45) + RCCW);
  double BR = (STR * cos(rad45) + FWD * sin(rad45) + RCCW);
  double BL = (-STR * sin(rad45) + FWD * cos(rad45) - RCCW);
  double FL = (STR * cos(rad45) + FWD * sin(rad45) - RCCW);

  // Add IMU input here for tilt correction
  double UL = gamepad.rightTrigger() - gamepad.leftTrigger();
  double UR = gamepad.rightTrigger() - gamepad.leftTrigger();
  double UB = gamepad.rightTrigger() - gamepad.leftTrigger();

  double* vals[] = {&FR, &BR, &BL, &FL, &UL, &UR, &UB};

  double max = 1.0;

  // Normalize the motor powers if calculation goes above 100%
  // Ignores up/down motors
  for (int i = 0; i < 4; ++i )
  {
    if (abs(*vals[i]) > max)
    {
      max = abs(*vals[i]);
    }
  }

  for (int i = 0; i < 4; ++i)
  {
    *vals[i] /= max;
  }

  // Convert the values to something the motors can read
  for (double* num : vals)
  {
    *num = convertRange(-1.0, 1.0, 1100.0, 1900.0, *num);
    data.append(to_string((int)*num) + ";");
  }

  data.pop_back();
  data.append("\n");

  if (gamepad.getButtonDown(xButtons.A) || gamepad.getButtonPressed(xButtons.B))
  {
    cout << "Sending: " << data << endl;
    sendData(data);
  }
}

int main()
{
  if (arduino.isConnected())
  {
    cout << " Arduino connection made" << endl << endl;
  }
  else
  {
    cout << " Error in Arduino port name" << endl << endl;
  }

  if (gamepad.connected())
  {
    cout << " Gamepad 1 connected" << endl;
  }
  else
  {
    cout << " Gamepad 1 NOT connected" << endl;
  }

  while (!gamepad.getButtonPressed(xButtons.Back))
  {
    gamepad.update();
    drive();
    gamepad.refresh();
  }

  cout << " Exiting" << endl;

  return 0;
}
