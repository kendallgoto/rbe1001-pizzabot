/*
Team 1 - Kendall Goto, Suryansh Goyal, Derik Pignone
RBE 1001
Greg Lewin
Pizza Bot
*/

#include "main.h"
using namespace pros;
#include <array>

#define LEFT_WHEEL_PORT 1
#define RIGHT_WHEEL_PORT 2
#define BAR_1_PORT 3
#define BAR_2_PORT 4
#define CLAW_PORT 5

enum CurrentState { SETUP, READY, MANUAL, AUTO};
enum AutonomousState { IDLE, SEEKING, DELIVERING };

//pros::brain      Brain;
Motor      motorLeft (LEFT_WHEEL_PORT, E_MOTOR_GEARSET_18, true, E_MOTOR_ENCODER_DEGREES);
Motor      motorRight(RIGHT_WHEEL_PORT, E_MOTOR_GEARSET_18, false, E_MOTOR_ENCODER_DEGREES);
Motor      motor4Bar_1(BAR_1_PORT, E_MOTOR_GEARSET_18, false, E_MOTOR_ENCODER_DEGREES);
Motor      motor4Bar_2(BAR_2_PORT, E_MOTOR_GEARSET_18, false, E_MOTOR_ENCODER_DEGREES);
Motor      motorClaw(CLAW_PORT, E_MOTOR_GEARSET_18, true, E_MOTOR_ENCODER_DEGREES);
Controller ctrl(pros::E_CONTROLLER_MASTER);
static Motor fourbar[2]{
    motor4Bar_1,
    motor4Bar_2
};
enum BarState {
    INTAKE_GROUND,
    INTAKE_FLOOR2,
    INTAKE_FLOOR3,
    INTAKE_FLOOR4,
    INTAKE_FLOOR5,
    BARSTATE_NR_ITEMS
};
double intake_Positions[BARSTATE_NR_ITEMS];
BarState intakeCurrentPosition;


double clawOpenPos;
bool clawClosed = false;
void moveMotors(Motor motorArray[], int size, double position, double velocity, bool blocking);

void initialize() {
    //Assume four bar is reset to floor
    motor4Bar_1.tare_position();
    motor4Bar_2.tare_position();
    motorClaw.tare_position();

    //Limit voltage (or current ... set_current_limit)
    motor4Bar_1.set_current_limit(5000); //mA
    motor4Bar_2.set_current_limit(5000); //mA
    motorClaw.set_voltage_limit(10000);

//    motor4Bar_1.setTimeout(4, timeUnits::sec); //prevent overdriving
//    motor4Bar_2.setTimeout(4, timeUnits::sec);

    intake_Positions[INTAKE_GROUND] = motor4Bar_1.get_position() - 5;
    intake_Positions[INTAKE_FLOOR2] = motor4Bar_1.get_position() + 160;
    intake_Positions[INTAKE_FLOOR3] = motor4Bar_1.get_position() + 210;
    intake_Positions[INTAKE_FLOOR4] = motor4Bar_1.get_position() + 340;
    intake_Positions[INTAKE_FLOOR5] = motor4Bar_1.get_position() + 510;

    int init_height = 250;
    moveMotors(fourbar, 2, init_height, 80, true);
    delay(200);
    moveMotors(&motorClaw, 1, 150, 100, true);

    clawOpenPos = motorClaw.get_position();

    moveMotors(fourbar, 2, intake_Positions[INTAKE_GROUND], 80, true);
    intakeCurrentPosition = INTAKE_GROUND;

    cout << "Torque\tVelocity\tPower\t\tTorque\tVelocity\tPower" << endl;

    opcontrol();
}

void disabled() {}

void competition_initialize() {

}

void autonomous() {

}

void opcontrol() {
	while (true) {
	    /*
	     * Assign Drive Factor based on L/R Triggers
	     */
	    double driveFactor;
        if(ctrl.get_digital(E_CONTROLLER_DIGITAL_L1) && !ctrl.get_digital(E_CONTROLLER_DIGITAL_R1)) {
            driveFactor = 1.0;
        } else if(!ctrl.get_digital(E_CONTROLLER_DIGITAL_L1) && ctrl.get_digital(E_CONTROLLER_DIGITAL_R1)) {
            driveFactor = 0.25;
        } else {
            driveFactor = 0.5;
        }

        /*
         * Spin L/R Motor based on Joystick Position
         */
        int power = ctrl.get_analog(ANALOG_LEFT_Y);
        int turn = ctrl.get_analog(ANALOG_RIGHT_X);
        int left = power + turn;
        int right = power - turn;
        motorLeft.move(left * driveFactor);
        motorRight.move(right * driveFactor);
        /*
         * Toggle claw when pressing [A]
         */
        if(ctrl.get_digital(E_CONTROLLER_DIGITAL_A)) {
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_A)) {
                delay(10);
            }
            clawClosed = !clawClosed;
            if(clawClosed){
                motorClaw.move_velocity(100);
            } else {
                motorClaw.move_absolute(clawOpenPos, 100);
            }
        }
        /*
         * Move bar up / down when pressing up and down
         */
        int barDirection = 0;
        if(ctrl.get_digital(E_CONTROLLER_DIGITAL_UP)) {
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_UP)) delay(20);
            barDirection = 1;
        } else if(ctrl.get_digital(E_CONTROLLER_DIGITAL_DOWN)) {
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_DOWN)) delay(20);
            barDirection = -1;
        }
        if(barDirection != 0) {
            int resultingChange = (((int)intakeCurrentPosition) + barDirection);
            if(resultingChange >= 0 && resultingChange < BARSTATE_NR_ITEMS) {
                BarState newState = (BarState)resultingChange;
                double target = intake_Positions[newState];
                moveMotors(fourbar, 2, target, 80, false);
                intakeCurrentPosition = newState;
            }
        }
        cout << motor4Bar_1.get_torque() << "\t" << motor4Bar_1.get_actual_velocity() << "\t" << motor4Bar_1.get_power() << "\t\t" << motor4Bar_2.get_torque() << "\t" << motor4Bar_2.get_actual_velocity() << "\t" << motor4Bar_2.get_power() << endl;
        delay(20);
	}
}

void moveMotors(Motor motorArray[], int size, double position, double velocity, bool blocking) {
    for(int i = 0; i < size; i++) {
        motorArray[i].move_absolute(position, velocity);
    }
    if(blocking) {
        while (!((motorArray[0].get_position() < position + 5) && (motorArray[0].get_position() > position - 5))) {
            delay(2);
        }
    }
}