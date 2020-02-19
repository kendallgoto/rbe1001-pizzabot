/*
Team 1 - Kendall Goto, Suryansh Goyal, Derik Pignone
RBE 1001
Greg Lewin
Pizza Bot
*/

#include "main.h"

#define LEFT_WHEEL_PORT 1
#define RIGHT_WHEEL_PORT 2
#define BAR_1_PORT 3
#define BAR_2_PORT 4
#define CLAW_PORT 5

enum CurrentState { SETUP, READY, MANUAL, AUTO};
enum AutonomousState { IDLE, SEEKING, DELIVERING };
//pros::brain      Brain;
pros::Motor      motorLeft (LEFT_WHEEL_PORT, true);
pros::Motor      motorRight(RIGHT_WHEEL_PORT);
pros::Motor      motor4Bar_1(BAR_1_PORT);
pros::Motor      motor4Bar_2(BAR_2_PORT);
pros::Motor      motorClaw(CLAW_PORT);
pros::Controller master(pros::E_CONTROLLER_MASTER);


void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");
}

void disabled() {}

void competition_initialize() {}

void autonomous() {}

void opcontrol() {
	while (true) {
        int power = master.get_analog(ANALOG_LEFT_Y);
        int turn = master.get_analog(ANALOG_RIGHT_X);
        int left = power + turn;
        int right = power - turn;

        motorLeft.move(left);
        motorRight.move(right);

		pros::delay(20);
	}
}
