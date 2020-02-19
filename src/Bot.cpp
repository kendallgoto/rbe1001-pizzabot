/*
Team 1 - Kendall Goto, Suryansh Goyal, Derik Pignone
RBE 1001
Greg Lewin
Pizza Bot


// VEX V5 C++ Project
#include "vex.h"
using namespace vex;
#include <iostream>
using namespace std;
#include <math.h>
enum CurrentState { SETUP, READY, MANUAL, AUTO};
enum AutonomousState { IDLE, SEEKING, DELIVERING };
vex::brain      Brain;
vex::motor      motorLeft(vex::PORT1, vex::gearSetting::ratio18_1, true);
vex::motor      motorRight(vex::PORT2, vex::gearSetting::ratio18_1, false);
vex::motor      motor4Bar_1(vex::PORT3, vex::gearSetting::ratio18_1, false);
vex::motor      motor4Bar_2(vex::PORT4, vex::gearSetting::ratio18_1, false);
vex::motor      motorClaw(vex::PORT5, vex::gearSetting::ratio18_1, true);
vex::controller Controller1(vex::controllerType::primary);
//#endregion config_globals

//Declaration Of Variables
const int operationMode = 0;            //0 = Teleop, 1+ = Autonomous routines
CurrentState STATE = SETUP;
bool isConfigured = false;

double totalDriveFactor;

//Setpoints for FourBar
double intake_Floor;
double intake_Dorm1;
double intake_Dorm2;
double intake_Dorm3;
double intake_Dorm4;
int intakeCurrentHeight;

//Claw State
bool clawClosed = false;
double clawOpenPos;


int main(void) {
    //INITIALIZATION UPON ENABLE
    //zero height
    while(true) {
        switch(STATE) {
            SETUP:
                motor4Bar_1.resetRotation();
                motor4Bar_2.resetRotation();
                motorClaw.resetRotation();

                motor4Bar_1.setMaxTorque(75, percentUnits::pct);
                motor4Bar_2.setMaxTorque(75, percentUnits::pct);
                motor4Bar_1.setTimeout(4, timeUnits::sec); //prevent overdriving
                motor4Bar_2.setTimeout(4, timeUnits::sec);
                //establishing fourbar setpoints
                intake_Floor = motor4Bar_1.rotation(rotationUnits::deg);
                intake_Dorm1 = motor4Bar_1.rotation(rotationUnits::deg) + 210;
                intake_Dorm2 = motor4Bar_1.rotation(rotationUnits::deg) + 340;
                intake_Dorm3 = motor4Bar_1.rotation(rotationUnits::deg) + 510;
                intake_Dorm4 = motor4Bar_1.rotation(rotationUnits::deg) + 525;

                //Initial FourBar Lifting
                motor4Bar_1.startRotateTo(180, rotationUnits::deg);
                motor4Bar_2.rotateTo(180, rotationUnits::deg, true);
                //Initial Claw Reset
                motorClaw.rotateTo(135, rotationUnits::deg);
                clawOpenPos = motorClaw.rotation(rotationUnits::deg);

                //Replacing Fourbar Height
                motor4Bar_1.startRotateTo(intake_Floor, rotationUnits::deg);
                motor4Bar_2.rotateTo(intake_Floor, rotationUnits::deg, true);
                intakeCurrentHeight = 0;
                cout << "Torque\tVelocity\tPower\t\tTorque\tVelocity\tPower" << endl;
                STATE = READY;
                break;
            READY:
                // wait until state changes to autonomous / manual
                break;
            MANUAL:
                manual_loop();
                break;
            AUTO:
                auto_loop();
                break;
        }
    }
}

task autonomous() {
    operationMode = 1;
}
task usercontrol() {
    operationMode = 0;
}

void manual_loop() {
    //determines % power output based on if a button is pressed
    if(Controller1.ButtonL1.pressing() && !Controller1.ButtonR1.pressing()){
        //HIGH SPEED
        totalDriveFactor = 1;
    }else if(!Controller1.ButtonL1.pressing() && Controller1.ButtonR1.pressing()){
        //LOW SPEED
        totalDriveFactor = 0.25;
    }else{
        //NORMAL SPEED
        totalDriveFactor = 0.5;
    }

    motorLeft.spin(directionType::fwd, ((Controller1.Axis3.value() + Controller1.Axis1.value())*totalDriveFactor), velocityUnits::pct);
    motorRight.spin(directionType::fwd, ((Controller1.Axis3.value() - Controller1.Axis1.value())*totalDriveFactor), velocityUnits::pct);

    if(Controller1.ButtonA.pressing()){
        while(Controller1.ButtonA.pressing()){
            sleepMs(50);
        }
        clawClosed = !clawClosed;
    }

    if(clawClosed){
        motorClaw.spin(directionType::fwd, 100, velocityUnits::pct);
    }else{
        motorClaw.startRotateTo(clawOpenPos, rotationUnits::deg, 50, velocityUnits::pct);
    }

    if(Controller1.ButtonUp.pressing()){
        while(Controller1.ButtonUp.pressing()){
            sleepMs(50);    //Do nothing until button release, maybe not need sleep
        }
        switch(intakeCurrentHeight){
            case 0:
                motor4Bar_1.startRotateTo(intake_Dorm1, rotationUnits::deg);
                motor4Bar_2.startRotateTo(intake_Dorm1, rotationUnits::deg);
                intakeCurrentHeight = 1;
                break;
            case 1:
                motor4Bar_1.startRotateTo(intake_Dorm2, rotationUnits::deg);
                motor4Bar_2.startRotateTo(intake_Dorm2, rotationUnits::deg);
                intakeCurrentHeight = 2;
                break;
            case 2:
                motor4Bar_1.startRotateTo(intake_Dorm3, rotationUnits::deg);
                motor4Bar_2.startRotateTo(intake_Dorm3, rotationUnits::deg);
                intakeCurrentHeight = 3;
                break;
            case 3:
                motor4Bar_1.startRotateTo(intake_Dorm4, rotationUnits::deg);
                motor4Bar_2.startRotateTo(intake_Dorm4, rotationUnits::deg);
                intakeCurrentHeight = 4;
                break;
        }
    }
    if(Controller1.ButtonDown.pressing()){
        while(Controller1.ButtonDown.pressing()){
            sleepMs(50);
        }
        switch(intakeCurrentHeight){
            case 1:
                motor4Bar_1.startRotateTo(intake_Floor, rotationUnits::deg);
                motor4Bar_2.startRotateTo(intake_Floor, rotationUnits::deg);
                intakeCurrentHeight = 0;
                break;
            case 2:
                motor4Bar_1.startRotateTo(intake_Dorm1, rotationUnits::deg);
                motor4Bar_2.startRotateTo(intake_Dorm1, rotationUnits::deg);
                intakeCurrentHeight = 1;
                break;
            case 3:
                motor4Bar_1.startRotateTo(intake_Dorm2, rotationUnits::deg);
                motor4Bar_2.startRotateTo(intake_Dorm2, rotationUnits::deg);
                intakeCurrentHeight = 2;
                break;
            case 4:
                motor4Bar_1.startRotateTo(intake_Dorm3, rotationUnits::deg);
                motor4Bar_2.startRotateTo(intake_Dorm3, rotationUnits::deg);
                intakeCurrentHeight = 3;
                break;
        }
    }

    //Debug Logging
    cout << motor4Bar_1.torque(torqueUnits::Nm) << "\t" << motor4Bar_1.velocity(velocityUnits::pct) << "\t" << motor4Bar_1.power(powerUnits::watt) << "\t" << "\t\t" << motor4Bar_2.torque(torqueUnits::Nm) << "\t" << motor4Bar_2.velocity(velocityUnits::pct) << "\t" << motor4Bar_2.power(powerUnits::watt) << endl;
    sleepMs(10);
}

 */