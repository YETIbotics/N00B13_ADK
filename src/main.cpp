#include "XBOXRECV.h"
#include "MDD10A.h"
#include "SimpleTimer.h"

USB Usb;
XBOXRECV Xbox(&Usb);
MDD10A mc;
SimpleTimer t;

float LeftJoystickY;
float LeftJoystickX;
float RightJoystickY;
float RightJoystickX;
float TriggerAggregate;

float DriveRightSpeed;
float DriveLeftSpeed;
float MOGOSpeed;
float ArmSpeed;
float ClawSpeed;

float isArcadeDrive;

void timerLoop();
void ReadController();
void Task();
void WriteRobot();
void Drive(int r, int l);
void MOGO(int spd);
void Lift(int spd);
void Claw(int spd);
void MatchAuton();

void setup()
{

	LeftJoystickY = 0.0;
	LeftJoystickX = 0.0;
	RightJoystickY = 0.0;
	RightJoystickX = 0.0;
	TriggerAggregate = 0.0;

	DriveRightSpeed = 0.0;
	DriveLeftSpeed = 0.0;
	MOGOSpeed = 0.0;
	ArmSpeed = 0.0;
	ClawSpeed = 0.0;

	isArcadeDrive = false;

	Serial.begin(115200);

	if (Usb.Init() == -1)
	{
		Serial.print(F("\r\nOSC did not start"));
		while (1)
			; //halt
	}
	Serial.print(F("\r\nXbox Wireless Receiver Library Started"));

	t.setInterval(20, timerLoop);
}

void loop()
{
	Usb.Task();
	t.run();
}

void timerLoop()
{
	ReadController();
	Task();
	WriteRobot();
}

void WriteRobot()
{
	mc.setMotorSpeed(4, DriveRightSpeed);
	mc.setMotorSpeed(5, DriveLeftSpeed);
	mc.setMotorSpeed(0, MOGOSpeed);

	mc.setMotorSpeed(2, -ArmSpeed);
	mc.setMotorSpeed(3, ClawSpeed);
	Serial.print("Drive(");
	Serial.print(DriveLeftSpeed);
	Serial.print(",");
	Serial.print(DriveRightSpeed);
	Serial.print("); MOGO(");
	Serial.print(MOGOSpeed);
	Serial.print("); delay(20);");
	Serial.println("");
}

void Task()
{

	if (!isArcadeDrive)
	{
		DriveRightSpeed = RightJoystickY;
		DriveLeftSpeed = LeftJoystickY;
	}
	else
	{
		DriveLeftSpeed = (RightJoystickY + LeftJoystickY) + (RightJoystickX + LeftJoystickX);
		DriveRightSpeed = (RightJoystickY + LeftJoystickY) - (RightJoystickX + LeftJoystickX);

		if (DriveLeftSpeed > 255)
			DriveLeftSpeed = 255;

		if (DriveLeftSpeed < -255)
			DriveLeftSpeed = -255;

		if (DriveRightSpeed > 255)
			DriveRightSpeed = 255;

		if (DriveRightSpeed < -255)
			DriveRightSpeed = -255;
	}

	MOGOSpeed = TriggerAggregate;

	if (true)
	{
		//do exponential

		DriveRightSpeed = map((DriveRightSpeed * abs(DriveRightSpeed)), -65025, 65025, -255, 255);
		DriveLeftSpeed = map((DriveLeftSpeed * abs(DriveLeftSpeed)), -65025, 65025, -255, 255);
	}

	if (false)
	{
		int maxDiff = 60;
		if (DriveLeftSpeed > 0 && DriveRightSpeed > 0)
		{
			if (abs(DriveLeftSpeed - DriveRightSpeed) > maxDiff)
			{
				if (DriveLeftSpeed > DriveRightSpeed)
				{
					DriveRightSpeed = DriveLeftSpeed - maxDiff;
				}
				else
				{
					DriveLeftSpeed = DriveRightSpeed - maxDiff;
				}
			}
		}
		else if (DriveLeftSpeed < 0 && DriveRightSpeed < 0)
		{
			if (abs(DriveLeftSpeed - DriveRightSpeed) > maxDiff)
			{
				if (DriveLeftSpeed < DriveRightSpeed)
				{
					DriveRightSpeed = DriveLeftSpeed - maxDiff;
				}
				else
				{
					DriveLeftSpeed = DriveRightSpeed - maxDiff;
				}
			}
		}
	}
}

void ReadController()
{
	LeftJoystickY = 0.0;
	LeftJoystickX = 0.0;
	RightJoystickY = 0.0;
	RightJoystickX = 0.0;
	TriggerAggregate = 0.0;
	ArmSpeed = 0.0;
	ClawSpeed = 0.0;

	if (Xbox.XboxReceiverConnected)
	{
		for (uint8_t i = 0; i < 4; i++)
		{
			if (Xbox.Xbox360Connected[i])
			{
				//L2 Trigger
				if (Xbox.getButtonPress(R2, i))
				{
					TriggerAggregate = 255.0 / 255 * Xbox.getButtonPress(R2, i) * 1;
				}
				//R2 Trigger
				else if (Xbox.getButtonPress(L2, i))
				{
					TriggerAggregate = 255.0 / 255 * Xbox.getButtonPress(L2, i) * -1;
				}

				const int joyThresh = 5500;
				if (Xbox.getAnalogHat(LeftHatX, i) > joyThresh || Xbox.getAnalogHat(LeftHatX, i) < -joyThresh || Xbox.getAnalogHat(LeftHatY, i) > joyThresh || Xbox.getAnalogHat(LeftHatY, i) < -joyThresh || Xbox.getAnalogHat(RightHatX, i) > joyThresh || Xbox.getAnalogHat(RightHatX, i) < -joyThresh || Xbox.getAnalogHat(RightHatY, i) > joyThresh || Xbox.getAnalogHat(RightHatY, i) < -joyThresh)
				{
					if (Xbox.getAnalogHat(LeftHatX, i) > joyThresh || Xbox.getAnalogHat(LeftHatX, i) < -joyThresh)
					{
						LeftJoystickX = 255.0 / 32767 * Xbox.getAnalogHat(LeftHatX, i);
					}
					if (Xbox.getAnalogHat(LeftHatY, i) > joyThresh)
					{
						//LeftJoystickY = 255.0 / 32767 * Xbox.getAnalogHat(LeftHatY, i);
						LeftJoystickY = map(Xbox.getAnalogHat(LeftHatY, i), 5500, 32767, 116, 255);
					}
					else if (Xbox.getAnalogHat(LeftHatY, i) < -joyThresh)
					{
						LeftJoystickY = map(Xbox.getAnalogHat(LeftHatY, i), -5500, -32767, -116, -255);
					}

					if (Xbox.getAnalogHat(RightHatX, i) > joyThresh || Xbox.getAnalogHat(RightHatX, i) < -joyThresh)
					{
						RightJoystickX = 255.0 / 32767 * Xbox.getAnalogHat(RightHatX, i);
					}
					if (Xbox.getAnalogHat(RightHatY, i) > joyThresh)
					{
						//RightJoystickY = 255.0 / 32767 * Xbox.getAnalogHat(RightHatY, i);
						RightJoystickY = map(Xbox.getAnalogHat(RightHatY, i), 5500, 32767, 116, 255);
					}
					else if (Xbox.getAnalogHat(RightHatY, i) < -joyThresh)
					{
						RightJoystickY = map(Xbox.getAnalogHat(RightHatY, i), -5500, -32767, -116, -255);
					}
				}

				//Serial.println(Xbox.getAnalogHat(RightHatY, i));

				if (Xbox.getButtonPress(X, i))
				{
					//DOWN
					ArmSpeed = -80;
				}
				else if (Xbox.getButtonPress(Y, i))
				{
					//UP
					ArmSpeed = 200;
				}

				if (Xbox.getButtonPress(L1, i))
				{
					//OPEN
					ClawSpeed = 225;
				}
				else if (Xbox.getButtonPress(R1, i))
				{
					//CLOSE
					ClawSpeed = -225;
				}

				//if (Xbox.getButtonPress(UP, i))
				{
					//LeftJoystickY = 200;
					//RightJoystickY = 200;
				}

				if (Xbox.getButtonClick(START, i))
				{
					isArcadeDrive = !isArcadeDrive;
				}

				if (Xbox.getButtonClick(BACK, i))
				{// parking match

					mc.setMotorSpeed(4, 255); //rt
					mc.setMotorSpeed(5, 255); //lt
					delay(100);

					mc.setMotorSpeed(4, 0); //rt
					mc.setMotorSpeed(5, 0); //lt
					delay(2000);

					mc.setMotorSpeed(4, 255); //rt
					mc.setMotorSpeed(5, 255); //lt
					delay(1000);
				}

				if (Xbox.getButtonClick(XBOX, i))
				{ // Skills Auton.
				Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-254.00); delay(20);
Drive(0.00,0.00); MOGO(-158.00); delay(20);
Drive(0.00,0.00); MOGO(-62.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(-23.00); delay(20);
Drive(0.00,0.00); MOGO(-150.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-73.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,75.00); MOGO(0.00); delay(20);
Drive(0.00,137.00); MOGO(0.00); delay(20);
Drive(53.00,209.00); MOGO(0.00); delay(20);
Drive(127.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,241.00); MOGO(0.00); delay(20);
Drive(255.00,169.00); MOGO(0.00); delay(20);
Drive(203.00,80.00); MOGO(0.00); delay(20);
Drive(108.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(99.00,0.00); MOGO(0.00); delay(20);
Drive(227.00,62.00); MOGO(0.00); delay(20);
Drive(255.00,91.00); MOGO(0.00); delay(20);
Drive(255.00,168.00); MOGO(0.00); delay(20);
Drive(255.00,222.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,181.00); MOGO(0.00); delay(20);
Drive(247.00,93.00); MOGO(0.00); delay(20);
Drive(56.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(90.00,0.00); MOGO(0.00); delay(20);
Drive(247.00,0.00); MOGO(0.00); delay(20);
Drive(255.00,0.00); MOGO(0.00); delay(20);
Drive(255.00,0.00); MOGO(0.00); delay(20);
Drive(255.00,0.00); MOGO(0.00); delay(20);
Drive(255.00,0.00); MOGO(0.00); delay(20);
Drive(255.00,0.00); MOGO(0.00); delay(20);
Drive(255.00,0.00); MOGO(0.00); delay(20);
Drive(198.00,0.00); MOGO(0.00); delay(20);
Drive(56.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(104.00,0.00); MOGO(0.00); delay(20);
Drive(255.00,69.00); MOGO(0.00); delay(20);
Drive(255.00,109.00); MOGO(0.00); delay(20);
Drive(255.00,224.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(53.00,255.00); MOGO(0.00); delay(20);
Drive(0.00,255.00); MOGO(0.00); delay(20);
Drive(0.00,163.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(19.00); delay(20);
Drive(0.00,0.00); MOGO(79.00); delay(20);
Drive(0.00,0.00); MOGO(150.00); delay(20);
Drive(0.00,0.00); MOGO(192.00); delay(20);
Drive(0.00,0.00); MOGO(225.00); delay(20);
Drive(0.00,0.00); MOGO(248.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(217.00); delay(20);
Drive(0.00,0.00); MOGO(137.00); delay(20);
Drive(0.00,0.00); MOGO(17.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(-54.00,0.00); MOGO(0.00); delay(20);
Drive(-81.00,0.00); MOGO(0.00); delay(20);
Drive(-132.00,0.00); MOGO(0.00); delay(20);
Drive(-167.00,0.00); MOGO(0.00); delay(20);
Drive(-225.00,-65.00); MOGO(0.00); delay(20);
Drive(-255.00,-91.00); MOGO(0.00); delay(20);
Drive(-255.00,-115.00); MOGO(0.00); delay(20);
Drive(-255.00,-139.00); MOGO(0.00); delay(20);
Drive(-255.00,-172.00); MOGO(0.00); delay(20);
Drive(-255.00,-182.00); MOGO(0.00); delay(20);
Drive(-255.00,-189.00); MOGO(0.00); delay(20);
Drive(-255.00,-192.00); MOGO(0.00); delay(20);
Drive(-255.00,-204.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-223.00); MOGO(0.00); delay(20);
Drive(-255.00,-208.00); MOGO(0.00); delay(20);
Drive(-255.00,-192.00); MOGO(0.00); delay(20);
Drive(-255.00,-151.00); MOGO(0.00); delay(20);
Drive(-255.00,-130.00); MOGO(0.00); delay(20);
Drive(-255.00,-83.00); MOGO(0.00); delay(20);
Drive(-255.00,-55.00); MOGO(0.00); delay(20);
Drive(-255.00,0.00); MOGO(0.00); delay(20);
Drive(-255.00,0.00); MOGO(0.00); delay(20);
Drive(-255.00,0.00); MOGO(0.00); delay(20);
Drive(-255.00,0.00); MOGO(0.00); delay(20);
Drive(-255.00,0.00); MOGO(0.00); delay(20);
Drive(-255.00,53.00); MOGO(0.00); delay(20);
Drive(-255.00,68.00); MOGO(0.00); delay(20);
Drive(-255.00,76.00); MOGO(0.00); delay(20);
Drive(-255.00,81.00); MOGO(0.00); delay(20);
Drive(-255.00,83.00); MOGO(0.00); delay(20);
Drive(-255.00,93.00); MOGO(0.00); delay(20);
Drive(-255.00,114.00); MOGO(0.00); delay(20);
Drive(-255.00,138.00); MOGO(0.00); delay(20);
Drive(-255.00,155.00); MOGO(0.00); delay(20);
Drive(-255.00,158.00); MOGO(0.00); delay(20);
Drive(-255.00,158.00); MOGO(0.00); delay(20);
Drive(-255.00,158.00); MOGO(0.00); delay(20);
Drive(-255.00,149.00); MOGO(0.00); delay(20);
Drive(-255.00,149.00); MOGO(0.00); delay(20);
Drive(-255.00,149.00); MOGO(0.00); delay(20);
Drive(-240.00,149.00); MOGO(0.00); delay(20);
Drive(-234.00,149.00); MOGO(0.00); delay(20);
Drive(-234.00,149.00); MOGO(0.00); delay(20);
Drive(-234.00,149.00); MOGO(0.00); delay(20);
Drive(-234.00,146.00); MOGO(0.00); delay(20);
Drive(-223.00,125.00); MOGO(0.00); delay(20);
Drive(-187.00,106.00); MOGO(0.00); delay(20);
Drive(-162.00,100.00); MOGO(0.00); delay(20);
Drive(-150.00,100.00); MOGO(0.00); delay(20);
Drive(-147.00,100.00); MOGO(0.00); delay(20);
Drive(-129.00,100.00); MOGO(0.00); delay(20);
Drive(-81.00,100.00); MOGO(0.00); delay(20);
Drive(-55.00,100.00); MOGO(0.00); delay(20);
Drive(0.00,100.00); MOGO(0.00); delay(20);
Drive(0.00,101.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,104.00); MOGO(0.00); delay(20);
Drive(0.00,140.00); MOGO(0.00); delay(20);
Drive(0.00,163.00); MOGO(0.00); delay(20);
Drive(-56.00,168.00); MOGO(0.00); delay(20);
Drive(-89.00,168.00); MOGO(0.00); delay(20);
Drive(-101.00,174.00); MOGO(0.00); delay(20);
Drive(-103.00,196.00); MOGO(0.00); delay(20);
Drive(-103.00,203.00); MOGO(0.00); delay(20);
Drive(-106.00,220.00); MOGO(0.00); delay(20);
Drive(-110.00,224.00); MOGO(0.00); delay(20);
Drive(-117.00,224.00); MOGO(0.00); delay(20);
Drive(-117.00,224.00); MOGO(0.00); delay(20);
Drive(-117.00,224.00); MOGO(0.00); delay(20);
Drive(-117.00,224.00); MOGO(0.00); delay(20);
Drive(-119.00,224.00); MOGO(0.00); delay(20);
Drive(-148.00,224.00); MOGO(0.00); delay(20);
Drive(-164.00,224.00); MOGO(0.00); delay(20);
Drive(-164.00,224.00); MOGO(0.00); delay(20);
Drive(-164.00,224.00); MOGO(0.00); delay(20);
Drive(-164.00,224.00); MOGO(0.00); delay(20);
Drive(-164.00,224.00); MOGO(0.00); delay(20);
Drive(-164.00,224.00); MOGO(0.00); delay(20);
Drive(-164.00,224.00); MOGO(0.00); delay(20);
Drive(-164.00,224.00); MOGO(0.00); delay(20);
Drive(-164.00,224.00); MOGO(0.00); delay(20);
Drive(-167.00,224.00); MOGO(0.00); delay(20);
Drive(-175.00,224.00); MOGO(0.00); delay(20);
Drive(-175.00,224.00); MOGO(0.00); delay(20);
Drive(-175.00,224.00); MOGO(0.00); delay(20);
Drive(-175.00,224.00); MOGO(0.00); delay(20);
Drive(-175.00,224.00); MOGO(0.00); delay(20);
Drive(-175.00,224.00); MOGO(0.00); delay(20);
Drive(-175.00,224.00); MOGO(0.00); delay(20);
Drive(-175.00,224.00); MOGO(0.00); delay(20);
Drive(-175.00,224.00); MOGO(0.00); delay(20);
Drive(-167.00,196.00); MOGO(0.00); delay(20);
Drive(-89.00,143.00); MOGO(0.00); delay(20);
Drive(0.00,102.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(-59.00,0.00); MOGO(0.00); delay(20);
Drive(-83.00,0.00); MOGO(0.00); delay(20);
Drive(-111.00,0.00); MOGO(0.00); delay(20);
Drive(-123.00,0.00); MOGO(0.00); delay(20);
Drive(-138.00,0.00); MOGO(0.00); delay(20);
Drive(-138.00,-53.00); MOGO(0.00); delay(20);
Drive(-138.00,-77.00); MOGO(0.00); delay(20);
Drive(-126.00,-94.00); MOGO(0.00); delay(20);
Drive(-72.00,-96.00); MOGO(0.00); delay(20);
Drive(-56.00,-96.00); MOGO(0.00); delay(20);
Drive(-56.00,-94.00); MOGO(0.00); delay(20);
Drive(-56.00,-70.00); MOGO(0.00); delay(20);
Drive(-56.00,0.00); MOGO(0.00); delay(20);
Drive(-56.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(-59.00,0.00); MOGO(0.00); delay(20);
Drive(-83.00,0.00); MOGO(0.00); delay(20);
Drive(-113.00,0.00); MOGO(0.00); delay(20);
Drive(-115.00,0.00); MOGO(0.00); delay(20);
Drive(-60.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,94.00); MOGO(0.00); delay(20);
Drive(0.00,205.00); MOGO(0.00); delay(20);
Drive(0.00,220.00); MOGO(0.00); delay(20);
Drive(0.00,237.00); MOGO(0.00); delay(20);
Drive(0.00,241.00); MOGO(0.00); delay(20);
Drive(0.00,241.00); MOGO(0.00); delay(20);
Drive(77.00,241.00); MOGO(0.00); delay(20);
Drive(97.00,241.00); MOGO(0.00); delay(20);
Drive(97.00,209.00); MOGO(0.00); delay(20);
Drive(97.00,143.00); MOGO(0.00); delay(20);
Drive(0.00,82.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,-76.00); MOGO(0.00); delay(20);
Drive(104.00,-110.00); MOGO(0.00); delay(20);
Drive(203.00,-111.00); MOGO(0.00); delay(20);
Drive(255.00,-118.00); MOGO(0.00); delay(20);
Drive(255.00,-118.00); MOGO(0.00); delay(20);
Drive(222.00,-118.00); MOGO(0.00); delay(20);
Drive(109.00,-115.00); MOGO(0.00); delay(20);
Drive(0.00,-67.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(96.00,-63.00); MOGO(0.00); delay(20);
Drive(138.00,-90.00); MOGO(0.00); delay(20);
Drive(198.00,-114.00); MOGO(0.00); delay(20);
Drive(233.00,-126.00); MOGO(0.00); delay(20);
Drive(233.00,-126.00); MOGO(0.00); delay(20);
Drive(233.00,-117.00); MOGO(0.00); delay(20);
Drive(143.00,-74.00); MOGO(0.00); delay(20);
Drive(68.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(-105.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-235.00); delay(20);
Drive(0.00,0.00); MOGO(-164.00); delay(20);
Drive(0.00,0.00); MOGO(-93.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(-71.00,0.00); MOGO(0.00); delay(20);
Drive(-94.00,-59.00); MOGO(0.00); delay(20);
Drive(-125.00,-109.00); MOGO(0.00); delay(20);
Drive(-180.00,-159.00); MOGO(0.00); delay(20);
Drive(-234.00,-226.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-190.00,-255.00); MOGO(0.00); delay(20);
Drive(-154.00,-244.00); MOGO(0.00); delay(20);
Drive(-151.00,-225.00); MOGO(0.00); delay(20);
Drive(-151.00,-201.00); MOGO(0.00); delay(20);
Drive(-151.00,-170.00); MOGO(0.00); delay(20);
Drive(-148.00,-114.00); MOGO(0.00); delay(20);
Drive(-128.00,0.00); MOGO(0.00); delay(20);
Drive(-128.00,0.00); MOGO(0.00); delay(20);
Drive(-125.00,0.00); MOGO(0.00); delay(20);
Drive(-125.00,0.00); MOGO(0.00); delay(20);
Drive(-125.00,0.00); MOGO(0.00); delay(20);
Drive(-125.00,0.00); MOGO(0.00); delay(20);
Drive(-117.00,0.00); MOGO(0.00); delay(20);
Drive(-94.00,0.00); MOGO(0.00); delay(20);
Drive(-88.00,0.00); MOGO(0.00); delay(20);
Drive(-85.00,0.00); MOGO(0.00); delay(20);
Drive(-85.00,82.00); MOGO(0.00); delay(20);
Drive(-88.00,140.00); MOGO(0.00); delay(20);
Drive(-111.00,203.00); MOGO(0.00); delay(20);
Drive(-111.00,255.00); MOGO(0.00); delay(20);
Drive(-114.00,255.00); MOGO(0.00); delay(20);
Drive(-114.00,255.00); MOGO(0.00); delay(20);
Drive(-133.00,255.00); MOGO(0.00); delay(20);
Drive(-154.00,255.00); MOGO(0.00); delay(20);
Drive(-159.00,255.00); MOGO(0.00); delay(20);
Drive(-159.00,255.00); MOGO(0.00); delay(20);
Drive(-159.00,255.00); MOGO(0.00); delay(20);
Drive(-159.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,255.00); MOGO(0.00); delay(20);
Drive(-161.00,233.00); MOGO(0.00); delay(20);
Drive(-161.00,168.00); MOGO(0.00); delay(20);
Drive(-110.00,0.00); MOGO(0.00); delay(20);
Drive(-62.00,0.00); MOGO(0.00); delay(20);
Drive(-56.00,0.00); MOGO(0.00); delay(20);
Drive(-54.00,0.00); MOGO(0.00); delay(20);
Drive(-54.00,0.00); MOGO(0.00); delay(20);
Drive(-54.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,70.00); MOGO(0.00); delay(20);
Drive(0.00,91.00); MOGO(0.00); delay(20);
Drive(63.00,158.00); MOGO(0.00); delay(20);
Drive(93.00,200.00); MOGO(0.00); delay(20);
Drive(109.00,255.00); MOGO(0.00); delay(20);
Drive(109.00,255.00); MOGO(0.00); delay(20);
Drive(109.00,255.00); MOGO(0.00); delay(20);
Drive(109.00,255.00); MOGO(0.00); delay(20);
Drive(60.00,255.00); MOGO(0.00); delay(20);
Drive(0.00,153.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(-9.00); delay(20);
Drive(0.00,0.00); MOGO(-116.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-255.00); delay(20);
Drive(0.00,0.00); MOGO(-190.00); delay(20);
Drive(0.00,0.00); MOGO(-40.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(95.00,0.00); MOGO(0.00); delay(20);
Drive(211.00,59.00); MOGO(0.00); delay(20);
Drive(255.00,95.00); MOGO(0.00); delay(20);
Drive(255.00,128.00); MOGO(0.00); delay(20);
Drive(255.00,134.00); MOGO(0.00); delay(20);
Drive(255.00,134.00); MOGO(0.00); delay(20);
Drive(255.00,134.00); MOGO(0.00); delay(20);
Drive(255.00,110.00); MOGO(0.00); delay(20);
Drive(255.00,84.00); MOGO(0.00); delay(20);
Drive(171.00,0.00); MOGO(0.00); delay(20);
Drive(77.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,-63.00); MOGO(0.00); delay(20);
Drive(0.00,-83.00); MOGO(0.00); delay(20);
Drive(0.00,-85.00); MOGO(0.00); delay(20);
Drive(0.00,-72.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(57.00,0.00); MOGO(0.00); delay(20);
Drive(203.00,0.00); MOGO(0.00); delay(20);
Drive(255.00,-73.00); MOGO(0.00); delay(20);
Drive(255.00,-136.00); MOGO(0.00); delay(20);
Drive(255.00,-139.00); MOGO(0.00); delay(20);
Drive(255.00,-78.00); MOGO(0.00); delay(20);
Drive(138.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(84.00,85.00); MOGO(0.00); delay(20);
Drive(172.00,160.00); MOGO(0.00); delay(20);
Drive(202.00,205.00); MOGO(0.00); delay(20);
Drive(209.00,255.00); MOGO(0.00); delay(20);
Drive(209.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(224.00,209.00); MOGO(0.00); delay(20);
Drive(135.00,164.00); MOGO(0.00); delay(20);
Drive(70.00,80.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(73.00,0.00); MOGO(0.00); delay(20);
Drive(127.00,76.00); MOGO(0.00); delay(20);
Drive(163.00,97.00); MOGO(0.00); delay(20);
Drive(182.00,153.00); MOGO(0.00); delay(20);
Drive(186.00,163.00); MOGO(0.00); delay(20);
Drive(181.00,168.00); MOGO(0.00); delay(20);
Drive(73.00,168.00); MOGO(0.00); delay(20);
Drive(0.00,147.00); MOGO(0.00); delay(20);
Drive(0.00,84.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(38.00); delay(20);
Drive(0.00,0.00); MOGO(112.00); delay(20);
Drive(0.00,0.00); MOGO(174.00); delay(20);
Drive(0.00,0.00); MOGO(225.00); delay(20);
Drive(0.00,0.00); MOGO(254.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(255.00); delay(20);
Drive(0.00,0.00); MOGO(135.00); delay(20);
Drive(-82.00,0.00); MOGO(0.00); delay(20);
Drive(-114.00,0.00); MOGO(0.00); delay(20);
Drive(-187.00,-88.00); MOGO(0.00); delay(20);
Drive(-223.00,-130.00); MOGO(0.00); delay(20);
Drive(-255.00,-180.00); MOGO(0.00); delay(20);
Drive(-255.00,-219.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-236.00); MOGO(0.00); delay(20);
Drive(-255.00,-151.00); MOGO(0.00); delay(20);
Drive(-255.00,-78.00); MOGO(0.00); delay(20);
Drive(-255.00,-57.00); MOGO(0.00); delay(20);
Drive(-169.00,-57.00); MOGO(0.00); delay(20);
Drive(-63.00,-75.00); MOGO(0.00); delay(20);
Drive(0.00,-192.00); MOGO(0.00); delay(20);
Drive(0.00,-255.00); MOGO(0.00); delay(20);
Drive(95.00,-255.00); MOGO(0.00); delay(20);
Drive(131.00,-255.00); MOGO(0.00); delay(20);
Drive(166.00,-255.00); MOGO(0.00); delay(20);
Drive(169.00,-255.00); MOGO(0.00); delay(20);
Drive(169.00,-255.00); MOGO(0.00); delay(20);
Drive(169.00,-255.00); MOGO(0.00); delay(20);
Drive(169.00,-255.00); MOGO(0.00); delay(20);
Drive(169.00,-255.00); MOGO(0.00); delay(20);
Drive(169.00,-246.00); MOGO(0.00); delay(20);
Drive(155.00,-194.00); MOGO(0.00); delay(20);
Drive(66.00,-91.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(55.00,0.00); MOGO(0.00); delay(20);
Drive(56.00,-92.00); MOGO(0.00); delay(20);
Drive(56.00,-156.00); MOGO(0.00); delay(20);
Drive(56.00,-183.00); MOGO(0.00); delay(20);
Drive(56.00,-238.00); MOGO(0.00); delay(20);
Drive(56.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(58.00,-255.00); MOGO(0.00); delay(20);
Drive(0.00,-255.00); MOGO(0.00); delay(20);
Drive(0.00,-255.00); MOGO(0.00); delay(20);
Drive(0.00,-255.00); MOGO(0.00); delay(20);
Drive(0.00,-255.00); MOGO(0.00); delay(20);
Drive(0.00,-255.00); MOGO(0.00); delay(20);
Drive(0.00,-255.00); MOGO(0.00); delay(20);
Drive(-80.00,-255.00); MOGO(0.00); delay(20);
Drive(-121.00,-255.00); MOGO(0.00); delay(20);
Drive(-157.00,-255.00); MOGO(0.00); delay(20);
Drive(-183.00,-255.00); MOGO(0.00); delay(20);
Drive(-206.00,-255.00); MOGO(0.00); delay(20);
Drive(-226.00,-255.00); MOGO(0.00); delay(20);
Drive(-244.00,-255.00); MOGO(0.00); delay(20);
Drive(-244.00,-255.00); MOGO(0.00); delay(20);
Drive(-244.00,-255.00); MOGO(0.00); delay(20);
Drive(-244.00,-255.00); MOGO(0.00); delay(20);
Drive(-244.00,-255.00); MOGO(0.00); delay(20);
Drive(-244.00,-255.00); MOGO(0.00); delay(20);
Drive(-244.00,-255.00); MOGO(0.00); delay(20);
Drive(-248.00,-255.00); MOGO(0.00); delay(20);
Drive(-248.00,-255.00); MOGO(0.00); delay(20);
Drive(-223.00,-255.00); MOGO(0.00); delay(20);
Drive(-201.00,-255.00); MOGO(0.00); delay(20);
Drive(-126.00,-255.00); MOGO(0.00); delay(20);
Drive(-69.00,-255.00); MOGO(0.00); delay(20);
Drive(0.00,-255.00); MOGO(0.00); delay(20);
Drive(0.00,-255.00); MOGO(0.00); delay(20);
Drive(70.00,-255.00); MOGO(0.00); delay(20);
Drive(106.00,-255.00); MOGO(0.00); delay(20);
Drive(141.00,-255.00); MOGO(0.00); delay(20);
Drive(158.00,-255.00); MOGO(0.00); delay(20);
Drive(163.00,-255.00); MOGO(0.00); delay(20);
Drive(163.00,-255.00); MOGO(0.00); delay(20);
Drive(163.00,-255.00); MOGO(0.00); delay(20);
Drive(163.00,-255.00); MOGO(0.00); delay(20);
Drive(163.00,-255.00); MOGO(0.00); delay(20);
Drive(163.00,-238.00); MOGO(0.00); delay(20);
Drive(147.00,-178.00); MOGO(0.00); delay(20);
Drive(128.00,-141.00); MOGO(0.00); delay(20);
Drive(61.00,-110.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(-57.00,0.00); MOGO(0.00); delay(20);
Drive(-57.00,0.00); MOGO(0.00); delay(20);
Drive(-57.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(69.00,0.00); MOGO(0.00); delay(20);
Drive(99.00,0.00); MOGO(0.00); delay(20);
Drive(108.00,0.00); MOGO(0.00); delay(20);
Drive(108.00,0.00); MOGO(0.00); delay(20);
Drive(108.00,0.00); MOGO(0.00); delay(20);
Drive(105.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(-75.00); delay(20);
Drive(0.00,0.00); MOGO(-147.00); delay(20);
Drive(0.00,0.00); MOGO(-197.00); delay(20);
Drive(0.00,0.00); MOGO(-221.00); delay(20);
Drive(0.00,0.00); MOGO(-216.00); delay(20);
Drive(0.00,0.00); MOGO(-157.00); delay(20);
Drive(0.00,0.00); MOGO(-56.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(-35.00); delay(20);
Drive(0.00,0.00); MOGO(-139.00); delay(20);
Drive(0.00,0.00); MOGO(-204.00); delay(20);
Drive(0.00,0.00); MOGO(-211.00); delay(20);
Drive(0.00,0.00); MOGO(-167.00); delay(20);
Drive(0.00,0.00); MOGO(-64.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(71.00,0.00); MOGO(0.00); delay(20);
Drive(134.00,0.00); MOGO(0.00); delay(20);
Drive(255.00,127.00); MOGO(0.00); delay(20);
Drive(255.00,205.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,255.00); MOGO(0.00); delay(20);
Drive(255.00,247.00); MOGO(0.00); delay(20);
Drive(255.00,138.00); MOGO(0.00); delay(20);
Drive(205.00,100.00); MOGO(0.00); delay(20);
Drive(193.00,62.00); MOGO(0.00); delay(20);
Drive(144.00,0.00); MOGO(0.00); delay(20);
Drive(93.00,0.00); MOGO(0.00); delay(20);
Drive(-61.00,-164.00); MOGO(0.00); delay(20);
Drive(-117.00,-255.00); MOGO(0.00); delay(20);
Drive(-232.00,-255.00); MOGO(0.00); delay(20);
Drive(-248.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-255.00); MOGO(0.00); delay(20);
Drive(-255.00,-189.00); MOGO(0.00); delay(20);
Drive(-255.00,0.00); MOGO(0.00); delay(20);
Drive(-223.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);
Drive(0.00,0.00); MOGO(0.00); delay(20);

					
					/*MOGO(-255);
					delay(600);
					MOGO(0);

					Drive(200,190);
					delay(800);
					Drive(0,0);

					MOGO(255);
					delay(200);
					MOGO(0);

					Drive(255,255);
					delay(1300);
					Drive(0,0);*/

					
					MOGO(-255);
					delay(600);
					MOGO(0);

					Drive(200,190);
					delay(800);
					Drive(0,0);

					MOGO(255);
					delay(500);
					MOGO(0);

					Drive(-175,0);
					delay(1300);
					Drive(0,0);

					Drive(0,175);
					delay(1100);
					Drive(0,0);

					MOGO(-255);
					delay(300);
					MOGO(0);

					Drive(-255,-255);
					delay(300);
					Drive(0,0);

					Drive(150,-175);
					delay(300);
					Drive(0,0);

					Drive(-255,-255);
					delay(700);
					Drive(0,0);



			

					//Drive(255,255);
					//delay(600);
					//Drive(-255,-255);
					//delay(500);
					//Drive(0,0);

			
					
					


				}

				if (Xbox.getButtonClick(A, i))
				{// up stack

					Lift(-255);
					Claw(255);
					delay(100);
					Claw(0);
					Lift(0);
					delay(900);
					Lift(0);

					Claw(-255);
					delay(300);
					Claw(0);

					Lift(150);
					delay(450);
					Lift(75);
					delay(200);
					Lift(50);
					delay(200);



					Lift(0);

				}

				if (Xbox.getButtonClick(B, i))
				{// down stack

					Lift(-255);
					Claw(-150);
					delay(100);
					Claw(0);
					Lift(0);
					delay(800);
					Lift(0);

					Claw(255);
					delay(200);
					Claw(0);

					Lift(125);
					Claw(150);
					delay(500);
					Claw(0);
					Lift(0);
					delay(800);
					Lift(0);

					


				}

				if (Xbox.getButtonClick(RIGHT, i))
				{	
					
					//break dance
					/* Drive(255,-255);
					delay(400);
					Drive(0,0);

					Drive(-255,255);
					delay(400);
					Drive(0,0);

					Drive(255,-255);
					delay(400);
					Drive(0,0);

					Drive(-255,255);
					delay(400);
					Drive(0,0);

					Drive(255,-255);
					delay(400);
					Drive(0,0);

					Drive(-255,255);
					delay(400);
					Drive(0,0);

					Drive(255,-255);
					delay(400);
					Drive(0,0); 
					
					Drive(-255,255);
					delay(4000);
					Drive(0,0);*/

				}





				if (Xbox.getButtonClick(LEFT, i))
				{//Match mogo score
					
					MatchAuton();

					Drive(150,-175);
					delay(300);
					Drive(0,0);

					Drive(-255,-255);
					delay(700);
					Drive(0,0);


				

				}

				if (Xbox.getButtonClick(DOWN, i))
				{//Match mogo score
					
					MatchAuton();
				
					//TURN ON DIME 180 CCW
					Drive(200,0);
					delay(2025);
					Drive(0,0);

					//GO FORWARD----p
					Drive(200,200);
					delay(400);
					Drive(0,0);

					//PICK UP A LITTLE

					//MOGO(255);
					//delay(200);
					//MOGO(0);
				

					//TURN ON A TIME AGAIN CW a little more than first time

					//Drive(200,-200);
					//delay(550);
					//Drive(0,0);

					//RACE FORWARD

					//Drive(255,255);
					//delay(800);
					//Drive(-255,-255);
					//delay(800);
					//Drive(0,0);

					//RACE BACK (up in race foward)*/

				}
				
				if (Xbox.getButtonClick(UP, i))
				{// 20 pt scoring
					//match park
					(600);
					MOGO(0);

					Drive(0,0);
					delay(500);
				
					Drive(200,200);
					delay(1400);
					Drive(0,0);

					
					
					//Drive(255,255);
					//delay(700);
					//Drive(0,0);
				

					//Drive(-255,-255);
					//delay(700);
					//Drive(0,0);
				}
			
			





				
			}
		}
	}
}

void MatchAuton()
{
MOGO(-255);
					delay(600);
					MOGO(0);

					Drive(0,0);
					delay(500);
				
					Drive(200,200);
					delay(1400);
					Drive(0,0);

					MOGO(255);
					delay(600);
					MOGO(0);

					Drive(-200+10,-200);
					delay(1455);
					Drive(0,0);

					Drive(-175,175);
					delay(1575);
					Drive(0,0);

					MOGO(-255);
					delay(600);
					MOGO(0);

					Drive(-255,-255);
					delay(275);
					Drive(0,0);


					
}

void Drive(int l, int r)
{
	mc.setMotorSpeed(4, r); //rt
	mc.setMotorSpeed(5, l); //lt
}
void MOGO(int spd)
{
	mc.setMotorSpeed(0, spd); //rt
}
void Lift(int spd)
{
	mc.setMotorSpeed(2, -spd); //rt
}
void Claw(int spd)
{
	mc.setMotorSpeed(3, spd); //rt
}