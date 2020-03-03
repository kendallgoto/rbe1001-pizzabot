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
#include <string>

// Port Numbers for Motors
#define LEFT_WHEEL_PORT 1
#define RIGHT_WHEEL_PORT 2
#define BAR_1_PORT 3
#define BAR_2_PORT 4
#define CLAW_PORT 5
#define WINCH_PORT 6

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
Motor      motorLeft (LEFT_WHEEL_PORT, E_MOTOR_GEARSET_18, false, E_MOTOR_ENCODER_DEGREES);
Motor      motorRight(RIGHT_WHEEL_PORT, E_MOTOR_GEARSET_18, true, E_MOTOR_ENCODER_DEGREES);
Motor      motor4Bar_1(BAR_1_PORT, E_MOTOR_GEARSET_18, false, E_MOTOR_ENCODER_DEGREES);
Motor      motor4Bar_2(BAR_2_PORT, E_MOTOR_GEARSET_18, false, E_MOTOR_ENCODER_DEGREES);
Motor      motorClaw(CLAW_PORT, E_MOTOR_GEARSET_18, true, E_MOTOR_ENCODER_DEGREES);
Motor      motorWinch(WINCH_PORT, E_MOTOR_GEARSET_18, true, E_MOTOR_ENCODER_DEGREES);
Controller ctrl(pros::E_CONTROLLER_MASTER);

// global variables

Motor fourbar[2] = { // Array of fourbar Motors
    motor4Bar_1,
    motor4Bar_2
};

Motor driveMotors[2] = {motorLeft, motorRight}; // Array of drive motors


double intake_Positions[BARSTATE_NR_ITEMS]; // Array of BarStates for intake positions
BarState intakeCurrentPosition; // Variable to keep track of current BarState
double intake_adjustment;
double clawOpenPos; // Variable to store the position of claw when it is open
bool clawClosed = false; // Variable to keep track of claw's state, true for closed, false otherwise

int side=-1; // Assumes two vals: 1 for left side, -1 for right side
// Function headers
void moveMotors(Motor motorArray[], int size, double position, double velocity, bool shouldBlock, bool isRelative, bool isTurning);
void drive(double distance, bool forward);
void turn(double angle);


/*
 * Main Function
 */
void initialize() {
    //Assume four bar is reset to floor and set absolute zero for both 4 bar and claw motors
    pros::lcd::initialize();
    pros::lcd::set_text(1, "Initializing ...!");
    motor4Bar_1.tare_position(); 
    motor4Bar_2.tare_position();
    motorClaw.tare_position();

    //Limit voltage (or current ... set_current_limit)
    motor4Bar_1.set_current_limit(5000); //mA
    motor4Bar_2.set_current_limit(5000); //mA
    motorClaw.set_current_limit(3000); //mA

    // setting encoder values with respect to motor4Bar_1 for intake positions
    intake_Positions[INTAKE_GROUND] = motor4Bar_1.get_position() - 5;
    intake_Positions[INTAKE_FLOOR2] = motor4Bar_1.get_position() + 245;
    intake_Positions[INTAKE_PIZZERIA] = motor4Bar_1.get_position() + 340;
    intake_Positions[INTAKE_FLOOR3] = motor4Bar_1.get_position() + 365;
    intake_Positions[INTAKE_FLOOR4] = motor4Bar_1.get_position() + 475;
    intake_Positions[INTAKE_FLOOR5] = motor4Bar_1.get_position() + 645;

    // Move 4Bar up, bring out the claw and reset the 4Bar to back to ground
    int init_height = 250;
    drive(15, false);
    moveMotors(fourbar, 2, init_height, 80, true, false, false);
    delay(400);
    moveMotors(&motorClaw, 1, 150, 100, true, false, false);

    clawOpenPos = motorClaw.get_position();

    moveMotors(fourbar, 2, intake_Positions[INTAKE_GROUND], 80, true, false, false); 
    intakeCurrentPosition = INTAKE_GROUND;

    pros::lcd::set_text(1, "INITIALIZED!");
    
    delay(5000);
    autonomous(); 
    delay(1000);
    opcontrol();
}

/*
 *
 */
void disabled() {


}

/*
 *
 */
void competition_initialize() {

}

/*
 * Function to operate on Autonomous Mode in OED
 */
void autonomous(){
    /* STRATEGY -- OED */
    /* Start from Pizzeria, Check for starting side (Left/Right), make adjustments and Pick up Pizza */
    /* Drive to Faraday */
    /* Place Pizza on Floor 2 */
    /* Drive back to Pizzeria */
    /* Pick up Pizza */
    /* Drive to Faraday */
    /* Place Pizza on Floor 3 */
    
    //motorLeft.set_pos_pid_full();

    BarState targetStates[3] = {INTAKE_FLOOR2, INTAKE_FLOOR3, INTAKE_FLOOR4};
    int currentState = 0;
    for(int i = 0; i < 3; i++) {
        // starting at 40 cm away from pizza slot
        delay(500);
        moveMotors(fourbar, 2, intake_Positions[INTAKE_PIZZERIA], 80, true, false, false); // set 4 bar to Pizzeria slot
        delay(1000);
        motorClaw.move_velocity(100); // Grab pizza
        delay(1000);
        moveMotors(fourbar, 2, intake_Positions[INTAKE_GROUND] + 80, 80, true, false, false); // set 4 bar to ground
        /* Drive to Faraday */
        delay(100);

        drive(80, false);
        delay(100);
        turn(90*side);
        delay(100);
        drive(50, false);
        delay(100);
        drive(40, true);

        /* Place Pizza on Floor i + 2 */
        delay(100);
        moveMotors(fourbar, 2, intake_Positions[targetStates[currentState++]], 80, true, false, false); // set 4 bar to Floor 2
        delay(100);
        drive(20, true);
        delay(100);
        motorClaw.move_absolute(clawOpenPos, 100); // open claw and deliver pizza

        /* Drive back to Pizzeria */
        delay(100);

        drive(70, false);
        delay(100);
        moveMotors(fourbar, 2, intake_Positions[INTAKE_GROUND] + 80, 80, true, false, false); // set 4 bar to Ground 
        delay(100);
        drive(25, true);
        delay(100);
        turn(-95*side);
        delay(100);
        moveMotors(fourbar, 2, intake_Positions[INTAKE_PIZZERIA], 80, true, false, false); // set 4 bar to Pizzeria slot
        delay(100);
        drive(87, true);
    }
     
    //Speed Bump Code

    // moveMotors(fourbar, 2, intake_Positions[INTAKE_GROUND] + 80, 80, true, false, false); // set 4 bar to ground

    // /* Move to construction zone */
    // delay(100);
    // drive(10, false);
    // turn(45*side);
    // delay(100);
    // drive(15, true);
    // delay(100);
    // turn(45*side);
    // delay(100);
    // drive(50, false);
    // delay(100);
    // drive(180, true);
    // delay(100);
    // moveMotors(fourbar, 2, intake_Positions[INTAKE_FLOOR2], 80, true, false, false);
    // motorClaw.move_velocity(100); // Close the claw
    // drive(30,true);
    // delay(100);
    // turn(110*side);
    // delay(100);
    // drive(70, false);
    // delay(100);
    // //plow full speed to get front wheel on
    // driveMotors[0].move(100);
    // driveMotors[1].move(100);
    // delay(1500); //We SHOULD be up by now.
    // driveMotors[0].move(0);
    // driveMotors[1].move(0);
    // delay(1000);
    // //then, turn by backing up the left wheel
    // drive(8, false);
    // delay(250);
    // turn(-40*side);
    // delay(250);
    // driveMotors[0].move_velocity(127);
    // driveMotors[1].move_velocity(0);
    // delay(725);
    // driveMotors[0].move_velocity(0);
    // driveMotors[1].move_velocity(0);
    // delay(1000);
    // // moveMotors(fourbar, 2, intake_Positions[INTAKE_FLOOR5], 80, true, false, false);
    // // delay(500);
    // drive(10, false);
    // delay(500);
    // driveMotors[0].move_velocity(0);
    // driveMotors[1].move_velocity(127);
    // delay(500);
    // driveMotors[0].move_velocity(60);
    // driveMotors[1].move_velocity(60);
    // delay(2000);
    // driveMotors[0].move_velocity(0);
    // driveMotors[1].move_velocity(0);
    // turn(-90*side);

    // motorClaw.move_absolute(clawOpenPos, 100); // open claw

}

// /*
//  * Function to operate on Autonomous Mode in CDR
//  */
// void autonomous() {
//     /* Strategy -- CDR */
//     /* Start from Pizzeria, Check for starting side (Left/Right), make adjustments and Pick up Pizza */
//     BarState targetStates[3] = {INTAKE_FLOOR2, INTAKE_FLOOR3, INTAKE_FLOOR4};
//     int currentState = 0;
//     for(int i = 0; i < 3; i++) {
//         // starting at 40 cm away from pizza slot
//         delay(500);
//         moveMotors(fourbar, 2, intake_Positions[INTAKE_PIZZERIA], 80, true, false, false); // set 4 bar to Pizzeria slot
//         delay(1000);
//         motorClaw.move_velocity(100); // Grab pizza
//         delay(1000);
//         moveMotors(fourbar, 2, intake_Positions[INTAKE_GROUND] + 80, 80, true, false, false); // set 4 bar to ground
//         /* Drive to Faraday */
//         delay(100);

//         drive(80, false);
//         delay(100);
//         turn(90*side);
//         delay(100);
//         drive(40, false);
//         delay(100);
//         drive(45, true);

//         /* Place Pizza on Floor 2 */
//         delay(100);
//         moveMotors(fourbar, 2, intake_Positions[targetStates[currentState++]], 80, true, false, false); // set 4 bar to Floor 2
//         delay(100);
//         drive(15, true);
//         delay(100);
//         motorClaw.move_absolute(clawOpenPos, 100); // open claw and deliver pizza

//         /* Drive back to Pizzeria */
//         delay(100);

//         drive(70, false);
//         delay(100);
//         moveMotors(fourbar, 2, intake_Positions[INTAKE_GROUND] + 80, 80, true, false,
//                    false); // set 4 bar to Ground 
//         delay(100);
//         drive(25, true);
//         delay(100);
//         turn(-105*side);
//         delay(100);
//         moveMotors(fourbar, 2, intake_Positions[INTAKE_PIZZERIA], 80, true, false, false); // set 4 bar to Pizzeria slot
//         delay(100);
//         drive(73, true);
//     }
// }

/*
 * Function to operate on Teleoperation Mode
 */
void opcontrol() {
    cout << "Torque\tVelocity\tPower\t\tTorque\tVelocity\tPower" << endl;
	while (true) {

        if(ctrl.get_digital(E_CONTROLLER_DIGITAL_B)) {
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_B)) {
                delay(10);
            }
            turn(90);
        }


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

        if(ctrl.get_digital(E_CONTROLLER_DIGITAL_Y)) {
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_Y)) {
                delay(10);
            }

            autonomous();
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
                resultingChange += barDirection;
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

        /*
         * Winch forward drive
         */
        if(ctrl.get_digital(E_CONTROLLER_DIGITAL_LEFT)) {
            //set motor mode
            motor4Bar_1.set_brake_mode(E_MOTOR_BRAKE_COAST);
            motor4Bar_2.set_brake_mode(E_MOTOR_BRAKE_COAST);
            motor4Bar_1.move_velocity(0);
            motor4Bar_2.move_velocity(0);
            motorWinch.move_velocity(100);
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_LEFT)) {
                delay(10);
            }
            motorWinch.move_velocity(0);
        }

        /*
         * Winch reverse drive
         */
        if(ctrl.get_digital(E_CONTROLLER_DIGITAL_RIGHT)) {
            //set motor mode
            motorWinch.move_velocity(-100);
            while(ctrl.get_digital(E_CONTROLLER_DIGITAL_RIGHT)) {
                delay(10);
            }
            motorWinch.move_velocity(0);
            motor4Bar_1.set_brake_mode(E_MOTOR_BRAKE_HOLD);
            motor4Bar_2.set_brake_mode(E_MOTOR_BRAKE_HOLD);
        }
        //Debugging Output
        pros::lcd::set_text(1, "Claw Temp\t\t" + std::string(to_string(motorClaw.get_temperature())));
        pros::lcd::set_text(2, "Claw THROTTLED\t\t" + std::string(to_string(motorClaw.is_over_temp())));
        pros::lcd::set_text(3, "4Bar1 THROTTLED\t\t" + std::string(to_string(motor4Bar_1.is_over_temp())));
        pros::lcd::set_text(4, "4Bar2 THROTTLED\t\t" + std::string(to_string(motor4Bar_2.is_over_temp())));
        pros::lcd::set_text(5, "BATTERY\t\t" + std::string(to_string(pros::battery::get_capacity())));
        delay(20);
	}
}

/*
 * Function to move a collection of motors at a specified position and with a specified velocity
 */
void moveMotors(Motor motorArray[], int size, double position, double velocity, bool shouldBlock, bool isRelative, bool isTurning) {
    if (isRelative) { // if we are driving
        double curPosition = motorArray[0].get_position();
        for(int i = 0; i < size; i++) { // move all individual motors in the array
            motorArray[i].move_relative(position, velocity);
        }
        while(motorArray[0].get_position() - curPosition < position - 5 || motorArray[0].get_position() - curPosition > position + 5) {
            delay(10);
        }
    }
    else if(isTurning) { // if we are turning
        double curPosition = motorArray[0].get_position();
        motorArray[0].move_relative(position, velocity);
        motorArray[1].move_relative(-1*position, velocity);
        while(motorArray[0].get_position() - curPosition < position - 5 || motorArray[0].get_position() - curPosition > position + 5) {
            delay(10);
        }
    }
    else {
        for(int i = 0; i < size; i++) { // move all individual motors in the array
            motorArray[i].move_absolute(position, velocity);
        }
        if(shouldBlock) {
            while(!((motorArray[0].get_position() < position + 5) && (motorArray[0].get_position() > position - 5))) {
                delay(10);
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
    const double diameter = 10.16;
    const double gearRatio = 1;

    int directionMult;
    if(forward)
        directionMult = 1;
    else
        directionMult = -1;
       
    double delta = (360 * distance) / (diameter * M_PI);
    //motorLeft.startRotateTo(delta*gearRatio*directionMult, deg);
    //motorRight.rotateTo(delta*gearRatio*directionMult, deg);
    moveMotors(driveMotors,2,delta*gearRatio*directionMult,60, true,true, false);
}

/*
 * Routine to turn the robot
 */
void turn(double angle) {
    //diameter * theta * pi / t
    //wheel track: 29cm
    //diameter: 10.16cm
    const double diameter = 10.16;
    const double track = 29;
    const double gearRatio = 1;
   
    double rot = (angle * track) / diameter;

    moveMotors(driveMotors,2, rot*gearRatio, 60, true, false, true);

}
