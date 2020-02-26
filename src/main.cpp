/*
Team 1 - Kendall Goto, Suryansh Goyal, Derik Pignone
RBE 1001
Greg Lewin
Pizza Bot
*/

// Libraries
#include "main.h"
using namespace pros;
#include <array>

// Port Numbers for Motors
#define LEFT_WHEEL_PORT 1
#define RIGHT_WHEEL_PORT 2
#define BAR_1_PORT 3
#define BAR_2_PORT 4
#define CLAW_PORT 5

// State enumerations
enum CurrentState { SETUP, READY, MANUAL, AUTO};
enum AutonomousState { IDLE, SEEKING, DELIVERING };
enum BarState {
    INTAKE_GROUND,
    INTAKE_FLOOR2,
    INTAKE_PIZZERIA,
    INTAKE_FLOOR3,
    INTAKE_FLOOR4,
    INTAKE_FLOOR5,
    BARSTATE_NR_ITEMS
};

// region config_globals
Motor      motorLeft (LEFT_WHEEL_PORT, E_MOTOR_GEARSET_18, true, E_MOTOR_ENCODER_DEGREES);
Motor      motorRight(RIGHT_WHEEL_PORT, E_MOTOR_GEARSET_18, false, E_MOTOR_ENCODER_DEGREES);
Motor      motor4Bar_1(BAR_1_PORT, E_MOTOR_GEARSET_18, false, E_MOTOR_ENCODER_DEGREES);
Motor      motor4Bar_2(BAR_2_PORT, E_MOTOR_GEARSET_18, false, E_MOTOR_ENCODER_DEGREES);
Motor      motorClaw(CLAW_PORT, E_MOTOR_GEARSET_18, true, E_MOTOR_ENCODER_DEGREES);
Controller ctrl(pros::E_CONTROLLER_MASTER);

// global variables

static Motor fourbar[2]{ // Array of fourbar Motors
    motor4Bar_1,
    motor4Bar_2
};

Motor driveMotors[2]= {motorLeft, motorRight}; // Array of drive motors


double intake_Positions[BARSTATE_NR_ITEMS]; // Array of BarStates for intake positions
BarState intakeCurrentPosition; // Variable to keep track of current BarState
double intake_adjustment;
double clawOpenPos; // Variable to store the position of claw when it is open
bool clawClosed = false; // Variable to keep track of claw's state, true for closed, false otherwise

// Function headers
void moveMotors(Motor motorArray[], int size, double position, double velocity, bool blocking, bool isDrive, bool isTurn);
void drive(double distance, bool forward);
void turn(double angle);

/*
 * Main Function
 */
void initialize() {
    //Assume four bar is reset to floor and set absolute zero for both 4 bar and claw motors 
    motor4Bar_1.tare_position(); 
    motor4Bar_2.tare_position();
    motorClaw.tare_position();

    //Limit voltage (or current ... set_current_limit)
    motor4Bar_1.set_current_limit(5000); //mA
    motor4Bar_2.set_current_limit(5000); //mA
    motorClaw.set_voltage_limit(8000); //mV

//    motor4Bar_1.setTimeout(4, timeUnits::sec); //prevent overdriving
//    motor4Bar_2.setTimeout(4, timeUnits::sec);

    // setting encoder values with respect to motor4Bar_1 for intake positions
    intake_Positions[INTAKE_GROUND] = motor4Bar_1.get_position() - 5;
    intake_Positions[INTAKE_FLOOR2] = motor4Bar_1.get_position() + 210;
    intake_Positions[INTAKE_PIZZERIA] = motor4Bar_1.get_position() + 335;
    intake_Positions[INTAKE_FLOOR3] = motor4Bar_1.get_position() + 355;
    intake_Positions[INTAKE_FLOOR4] = motor4Bar_1.get_position() + 470;
    intake_Positions[INTAKE_FLOOR5] = motor4Bar_1.get_position() + 640;

    // Move 4Bar up, bring out the claw and reset the 4Bar to back to ground
    int init_height = 250;
    moveMotors(fourbar, 2, init_height, 80, true, false, false);
    delay(200);
    moveMotors(&motorClaw, 1, 150, 100, true, false, false);

    clawOpenPos = motorClaw.get_position();

    moveMotors(fourbar, 2, intake_Positions[INTAKE_GROUND], 80, true, false, false); 
    intakeCurrentPosition = INTAKE_GROUND;

   

    opcontrol();
}

/*
 *
 */
void disabled() {}

/*
 *
 */
void competition_initialize() {
}

/*
 * Function to operate on Autonomous Mode
 */
void autonomous() {
    /* Strategy -- CDR */
    /* Start from Pizzeria, Check for starting side (Left/Right), make adjustments and Pick up Pizza */
    /* Drive to Faraday */
    /* Place Pizza on Floor 2 */
    /* Drive back to Pizzeria */
    /* Pick up Pizza */
    /* Drive to Faraday */
    /* Place Pizza on Floor 3 */
    /* Drive back to Pizzeria */
    /* Pick up Pizza */
    /* Drive to Faraday */
    /* Place Pizza on Floor 4 */

    /* STRATEGY -- OED */
    /* Start from Pizzeria, Check for starting side (Left/Right), make adjustments and Pick up Pizza */
    /* Drive to Faraday */
    /* Place Pizza on Floor 2 */
    /* Drive back to Pizzeria */
    /* Pick up Pizza */
    /* Drive to Faraday */
    /* Place Pizza on Floor 3 */
    /* Move to construction zone */
    /* Go over the speed bump */
    //motorLeft.set_pos_pid_full();
}

/*
 * Function to operate on Teleoperation Mode
 */
void opcontrol() {
    cout << "Torque\tVelocity\tPower\t\tTorque\tVelocity\tPower" << endl;
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
         * Move the bar to pizzeria pickup window
         */
        if(ctrl.get_digital(E_CONTROLLER_DIGITAL_X)) {
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_X)) {
                delay(10);
            }
            moveMotors(fourbar, 2, intake_Positions[INTAKE_PIZZERIA], 80, false, false, false);
            intakeCurrentPosition = INTAKE_PIZZERIA;
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
        if(barDirection != 0) { // if the UP or DOWN button is pressed 
            int resultingChange = (((int)intakeCurrentPosition) + barDirection);
            if(resultingChange == INTAKE_PIZZERIA){
                resultingChange++;
            }
            if(resultingChange >= 0 && resultingChange < BARSTATE_NR_ITEMS) {
                intake_adjustment = 0;
                BarState newState = (BarState)resultingChange;
                double target = intake_Positions[newState];
                moveMotors(fourbar, 2, target, 80, false, false, false);
                intakeCurrentPosition = newState;
            }
        }

        /*
         * Just in case, allow for micro adjustments with the bumpers
         */
        if(ctrl.get_digital(E_CONTROLLER_DIGITAL_L2)) {
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_L2)) delay(20);
            double target = intake_adjustment += 5;
            moveMotors(fourbar, 2, target, 80, false, false, false);
            intake_adjustment = target;
        } else if(ctrl.get_digital(E_CONTROLLER_DIGITAL_R2)) {
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_R2)) delay(20);
            double target = intake_adjustment -= 5;
            moveMotors(fourbar, 2, target, 80, false, false, false);
            intake_adjustment = target;
        }

        cout << motor4Bar_1.get_torque() << "\t" << motor4Bar_1.get_actual_velocity() << "\t" << motor4Bar_1.get_power() << "\t\t" << motor4Bar_2.get_torque() << "\t" << motor4Bar_2.get_actual_velocity() << "\t" << motor4Bar_2.get_power() << endl;
        delay(20);
	}
}

/*
 * Function to move a collection of motors at a specified position and with a specified velocity
 */
void moveMotors(Motor motorArray[], int size, double position, double velocity, bool blocking, bool isDrive, bool isTurn) {
    if(isDrive){ // if we are driving
        double curPosition = motorArray[0].get_position();
        for(int i = 0; i < size; i++) { // move all individual motors in the array
            motorArray[i].move_relative(position, velocity);
        }
        while (!((motorArray[0].get_position() < position - curPosition + 5 ) && (motorArray[0].get_position() > position - curPosition - 5))) {
            delay(2);
        }
    }
    else if (isTurn){ // if we are turning
        double curPosition = motorArray[0].get_position();
        for(int i = 0; i < size; i++) { // move all individual motors in the array
            motorArray[i].move_relative(position, velocity);
        }
        while (!((motorArray[0].get_position() < position - curPosition + 5 ) && (motorArray[0].get_position() > position - curPosition - 5))) {
            delay(2);
        }

    }
    else{
        for(int i = 0; i < size; i++) { // move all individual motors in the array
            motorArray[i].move_absolute(position, velocity);
        }
        if(blocking) { 
            while (!((motorArray[0].get_position() < position + 5) && (motorArray[0].get_position() > position - 5))) {
                delay(2);
            }
        }
    }
    
}

/*
 * Routine to drive a particular distance
 */
void drive(double distance, bool forward) {
    //theta/360 * 2 * pi * r
    //wheel diameter: 10.16cm
    const double diameter = 4;
    const double gearRatio = 1;

    int directionMult;
    if(forward)
        directionMult = 1;
    else
        directionMult = -1;
       
    double delta = (360 * distance) / (diameter * M_PI);
    //motorLeft.startRotateTo(delta*gearRatio*directionMult, deg);
    //motorRight.rotateTo(delta*gearRatio*directionMult, deg);
    moveMotors(driveMotors,2,delta*gearRatio*directionMult,80, true,true, false);   
}

/*
 * Routine to turn the robot
 */
void turn(double angle) {
    //diameter * theta * pi / t
    //wheel track: 28cm
    //diameter: 10.16cm
    const double diameter = 4;
    const double track = 11.0236;
    const double gearRatio = 5;
   
    double rot = (angle * track) / diameter;
    
    //motorLeft.startRotateTo(rot*gearRatio, deg);
    //motorRight.rotateTo(-rot*gearRatio, deg);

    moveMotors(driveMotors,2, rot*gearRatio, 80, true, false, true);
}
