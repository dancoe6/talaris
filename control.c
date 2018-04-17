#include <math.h>
#define PRESSURE_SENSOR_PIN 1
#define VALVE_PIN 2
#define INCREMENT_BUTTON 3
#define DECREMENT_BUTTON 4
#define TICKS_PER_SECOND 100 //@Justen this is the clock rate. we can increase this if the processor isn't fast enough to get through instructions

#define ABSOLUTE_PRESSURE 100000 //assume 100kPa
#define SPECIFIC_GAS_CONSTANT 287.058 //this is for dry air in J/(kg·K)


#define VERY_LOW_PRESSURE -10.0
#define LOW_PRESSURE -5.0
#define SLIGHTLY_LOW_PRESSURE -2.0
#define VERY_LOW_PRESSURE_TIME 2.0
#define LOW_PRESSURE_TIME 1.0
#define SLIGHTLY_LOW_PRESSURE_TIME .5
#define MIN_LOW_PRESSURE_TIME .1

#define SCREEN_UPDATE_RATE TICKS_PER_SECOND/5 //screen updated every 200ms

#define COMPRESSOR_PSI 150.0 //estimated compressor PSI. need to test this
#define HARDWIRED_PSI 35.0 //psi is hardwired for testing purposes
#define VALVE_OPEN_COUNTER_MAX TICKS_PER_SECOND/50 //20ms delay to open valve
#define VALVE_CLOSE_COUNTER_MAX TICKS_PER_SECOND/50 //20ms delay to close valve
#define PRESSURE_READING_COUNTER_MAX TICKS_PER_SECOND/100 //10ms delay to read pressure (maybe we should read 10 values and average?)
#define FILL_TIME_COUNTER_CONVERSION 100
#define INPUT_TUBE_DIAMETER_INCHES .75 //crossectional area of tube going into device in inches
#define TUBE_DIAMETER_TO_VOLUME_MODIFIER 0.0003800306//volume (meters cubed) of one meter of tubing
#define AVERAGE_TIRE_VOLUME .01 //in cubic meters, this would be computed normally

double targetPressure = HARDWIRED_PSI;
bool controlEnabled = false;
int valveCounter = 0;
int pressureReadCounter = 0;
double currentPressure = 0;
int fillCounter = 0;
double fillTime = 0;
bool valveClosed = true;
double currentTemperature = 300.0; //temperature reading in Kelvin
double rho = 0;
double pressureReading = 0.0;

double readPressureSensor(){
  //@Justen set pressureReading equal to reading from sensor (convert to PSI if not already)
  return pressureReading;
};

/*
void computeRho(double temperature){
  rho = (ABSOLUTE_PRESSURE)/(SPECIFIC_GAS_CONSTANT*temperature);
}

//helper function to find air velocity based on tests
double airVelocity (double currentPressure, double targetPressure){
  double averagePressure = (targetPressure+currentPressure)/2;
  double velocity = sqrt((COMPRESSOR_PSI - averagePressure)/(rho/2));
  printf("velocity is %f\n", velocity);
  return velocity;
}
//volume of inner space of one meter of tubing is 0.00028502295 square meters based on 3/4 inch tubing
//mass (kg) in a cubic meter is rho, as calculated above
//helper function to determine mass (in kilograms) per second
double massPerSecond(double velocity, double area){
  double temp = area*TUBE_DIAMETER_TO_VOLUME_MODIFIER*velocity;
  printf("massPerSecond is %f\n", temp);
  return temp;
};

//helper function to get volume of tire
double tireVolume(double temperature, double pressureDifference){
  return AVERAGE_TIRE_VOLUME; //this is average in cubic meters
};


double findFillTime(double currentPressure, double targetPressure, double area, double temperature, double pressureDifference){
  double fillTime = (tireVolume(temperature, pressureDifference)) /massPerSecond(airVelocity(currentPressure, targetPressure),area);
}
*/

enum control_st_t{
  init_st,
  close_valve_st,
  check_pressure_st,
  open_valve_st,
  fill_st,
  done_st
} curControlState;

void control_init(){
  /*
  //read temperature
  computeRho(currentTemperature);
  */
  curControlState = init_st;
}

void control_enable(){
  controlEnabled = true;
}

void control_disable(){
  controlEnabled = false;
}

void openValve(){
  //@Justen code to open valve here
  valveClosed = false;
};

void closeValve(){
  //@Justen code to close valve here
  valveClosed = true;
};

bool isClosed(){
  return valveClosed;
};

void displayCurrentPressure(){
  //@Justen code to print currentPressure variable to screen
}

void debugControlPrint() {
  static control_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != curControlState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = curControlState;     // keep track of the last state that you were in.
    switch(curControlState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf("init_st\n\r");
        break;
      case close_valve_st:
        printf("close_valve_st\n\r");
        break;
      case check_pressure_st:
        printf("check_pressure_st\n\r");
        break;
      case open_valve_st:
        printf("open_valve_st\n\r");
        break;
      case fill_st:
        printf("fill_st\n\r");
        break;
      case done_st:
        printf("done_st\n\r");
        break;
      default:
        printf("debugControlPrint hit default\n");
        break;
     }
  }
}


void controlTick(){
  debugControlPrint();
  switch(curControlState){
    case init_st:
      if(controlEnabled){
        valveCounter = 0;
        curControlState = close_valve_st;
      }else{
        curControlState = init_st;
      }
      break;
    case close_valve_st:
      if(controlEnabled){
        closeValve();
        if(valveCounter > VALVE_CLOSE_COUNTER_MAX){
          valveCounter = 0;
          curControlState = check_pressure_st;
        }else{
          valveCounter++;
        }
      }else{
        curControlState = init_st;
      }
      break;
    case check_pressure_st:
      if(controlEnabled){
        if(pressureReadCounter == 0){
          currentPressure = readPressureSensor();
          pressureReadCounter++;
        }else if(pressureReadCounter > PRESSURE_READING_COUNTER_MAX){
          pressureReadCounter = 0;
          if(currentPressure >= targetPressure){
            printf("currentPressure is %f, which is above target value: %f\n", currentPressure, targetPressure);
            curControlState = done_st;
          }else{
            double currentPressureDifference = currentPressure - targetPressure;
            if(currentPressureDifference < VERY_LOW_PRESSURE){
              fillTime = VERY_LOW_PRESSURE_TIME;
            }else if(currentPressureDifference < LOW_PRESSURE){
              fillTime = LOW_PRESSURE_TIME;
            }else if(currentPressureDifference < SLIGHTLY_LOW_PRESSURE){
              fillTime = SLIGHTLY_LOW_PRESSURE_TIME;
            }else{
              fillTime = MIN_LOW_PRESSURE_TIME;
            }
            displayCurrentPressure();
            printf("currentPressure is %f, which is below target value: %f\n", currentPressure, targetPressure);
            // fillTime = findFillTime(currentPressure, targetPressure, INPUT_TUBE_DIAMETER_INCHES, currentTemperature, 0.0)*FILL_TIME_COUNTER_CONVERSION;
            printf("fillTime is %f seconds\n", fillTime);
            curControlState = open_valve_st;
          }
        }else{
          pressureReadCounter++;
        }
      }else{
        curControlState = init_st;
      }
      break;
    case open_valve_st:
      if(controlEnabled){
        openValve();
        if(valveCounter > VALVE_OPEN_COUNTER_MAX){
          valveCounter = 0;
          curControlState = fill_st;
        }else{
          valveCounter++;
        }
      }else{
        curControlState = init_st;
      }
      break;
    case fill_st:
      if(controlEnabled){
        if(fillCounter > (fillTime*TICKS_PER_SECOND)){
          fillCounter = 0;
          curControlState = close_valve_st; 
        }else{
          fillCounter++;
          if(fillCounter%SCREEN_UPDATE_RATE == 0){
            displayCurrentPressure();
          }
        }
      }else{
        curControlState = init_st;
      }
      break;
    case done_st:
      if(controlEnabled){
        displayCurrentPressure();
        printf("COMPLETE: tire filled\n");
        control_disable();
        curControlState = init_st;
      }else{
        curControlState = init_st;
      }
      break;
    default:
      printf("controlTick sm hit default\n");
      break;
  }
}