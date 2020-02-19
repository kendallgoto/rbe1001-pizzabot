/*
Team 1 - Kendall Goto, Suryansh Goyal, Derik Pignone
RBE 1001
Greg Lewin
Pizza Bot
*/

#include "main.h"
using namespace pros;
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
Motor      motorClaw(CLAW_PORT, E_MOTOR_GEARSET_18, false, E_MOTOR_ENCODER_DEGREES);
Controller ctrl(pros::E_CONTROLLER_MASTER);

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

void initialize() {
    //Assume four bar is reset to floor
    motor4Bar_1.tare_position();
    motor4Bar_2.tare_position();
    motorClaw.tare_position();

    //Limit voltage (or current ... set_current_limit)
    motor4Bar_1.set_voltage_limit(5000); //mv
    motor4Bar_2.set_voltage_limit(5000); //mv
    motorClaw.set_voltage_limit(5000); //mv

//    motor4Bar_1.setTimeout(4, timeUnits::sec); //prevent overdriving
//    motor4Bar_2.setTimeout(4, timeUnits::sec);

    intake_Positions[INTAKE_GROUND] = motor4Bar_1.get_position();
    intake_Positions[INTAKE_FLOOR2] = motor4Bar_1.get_position() + 160;
    intake_Positions[INTAKE_FLOOR3] = motor4Bar_1.get_position() + 210;
    intake_Positions[INTAKE_FLOOR4] = motor4Bar_1.get_position() + 340;
    intake_Positions[INTAKE_FLOOR5] = motor4Bar_1.get_position() + 510;

    int init_height = 180;
    motor4Bar_1.move_absolute(init_height, 80);
    motor4Bar_2.move_absolute(init_height, 80);
    while (!((motor4Bar_1.get_position() < init_height + 5) && (motor4Bar_1.get_position() > init_height - 5))) {
        cout << motor4Bar_1.get_position() << endl;
        delay(2);
    }
    motorClaw.move_absolute(135, 80);
    motorClaw.move_absolute(135, 80);
    clawOpenPos = motorClaw.get_position();

    double target = intake_Positions[INTAKE_GROUND];
    motor4Bar_1.move_absolute(target, 80);
    motor4Bar_2.move_absolute(target, 80);
    while (!((motor4Bar_1.get_position() < target + 5) && (motor4Bar_1.get_position() > target - 5))) {
        delay(2);
    }
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

        if(ctrl.get_digital(E_CONTROLLER_DIGITAL_A)) {
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_A)) {
                delay(10);
            }
            clawClosed = !clawClosed;
        }
        if(clawClosed){
            motorClaw.move_velocity(2);
        } else {
            motorClaw.move_absolute(clawOpenPos, 2);
        }
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
                motor4Bar_1.move_absolute(target, 80);
                motor4Bar_2.move_absolute(target, 80);
                intakeCurrentPosition = newState;
            }
        }

        cout << motor4Bar_1.get_torque() << "\t" << motor4Bar_1.get_actual_velocity() << "\t" << motor4Bar_1.get_power() << "\t\t" << motor4Bar_2.get_torque() << "\t" << motor4Bar_2.get_actual_velocity() << "\t" << motor4Bar_2.get_power() << endl;
        delay(20);
	}
}
