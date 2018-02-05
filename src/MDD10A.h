#ifndef MDD10A_h
#define MDD10A_h

#include <Arduino.h>

class MDD10A {

public:

	MDD10A();

	void setMotorSpeed(int controller, int speed);
	void init();

	int dir = 0;


	//PinOuts

	//MC
	const int _mc0_dir0 = 22;
	const int _mc0_pwm0 = 2;
	const int _mc0_dir1 = 24;
	const int _mc0_pwm1 = 3;

	//MC3
	const int _mc1_dir0 = 40;
	const int _mc1_pwm0 = 11;
	const int _mc1_dir1 = 38;
	const int _mc1_pwm1 = 10;

	//MC
	const int _mc2_dir0 = 49;
	const int _mc2_pwm0 = 9;
	const int _mc2_dir1 = 47;
	const int _mc2_pwm1 = 8;

	//MC
	const int _mc3_dir0 = 40;
	const int _mc3_pwm0 = 33;
	const int _mc3_dir1 = 38;
	const int _mc3_pwm1 = 31;





};

#endif


