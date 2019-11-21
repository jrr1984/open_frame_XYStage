#include <AccelStepper.h>
#include <SerialCommand_TS.h>       // Command library to handle serial commands. The _TS version is case insensitive.
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <avr/pgmspace.h>
#include <math.h>


char buffer[50];
char * bufptr = buffer;
unsigned long max_x_vel = 300;
unsigned long x_accel = 500;
unsigned long max_z_vel = 200;
unsigned long z_accel = 400;

#define XEnablePin 24
#define ZEnablePin A8
#define Tpolling 200
#define Tupdate 50
#define steps_per_mm 6400
#define EEPROM_reg 20
#define xeeprom_dir 21
#define zeeprom_dir 22

int EnabledX = 0;
int EnabledZ = 0;
int XMoving = 0, XMoveAbs = 0;
int ZMoving = 0, ZMoveAbs = 0;
unsigned long TCurrent, TPOld, TUOld;

const char string_0[] PROGMEM = "XZStage";
const char string_1[] PROGMEM = "help";
const char string_2[] PROGMEM = "XPOS? -> Get x";
const char string_3[] PROGMEM = "ZPOS? -> Get z";
const char string_4[] PROGMEM = "XZPOS? -> Get x & z";
const char string_5[] PROGMEM = "OFF -> Turn off";
const char string_6[] PROGMEM = "XACC -> Set x acceleration (steps/s^2)";
const char string_7[] PROGMEM = "ZACC -> Set z acceleration (steps/s^2)";
const char string_8[] PROGMEM = "XACC? -> Get x acceleration (steps/s^2)";
const char string_9[] PROGMEM = "ZACC? -> Get z acceleration (steps/s^2)";
const char string_10[] PROGMEM = "MAXXVEL -> Set x max vel (steps/s)";
const char string_11[] PROGMEM = "MAXZVEL -> Set z max vel (steps/s)";
const char string_12[] PROGMEM = "MAXXVEL? -> Get x max vel (steps/s)";
const char string_13[] PROGMEM = "MAXXVEL? -> Get z max vel (steps/s)";
const char string_14[] PROGMEM = "MOVX -> Move to x (micrometers)";
const char string_15[] PROGMEM = "MOVZ -> Move to z (micrometers)";
const char string_16[] PROGMEM = "MOVX -> Move to x (micrometers)";
const char string_17[] PROGMEM = "ISXMOV? -> Is x stepper moving?";
const char string_18[] PROGMEM = "ISZMOV? -> Is z stepper moving?";

PGM_P const helpString[] PROGMEM = { string_0, string_1, string_2, string_3, string_4, string_5,
                                     string_6, string_7, string_8, string_9, string_10, string_11,
                                     string_12, string_13, string_14, string_15, string_17, string_18};
AccelStepper ZStepper(1, 46,48);
AccelStepper XStepper(1, 26, 28);
SerialCommand sCmd;

void connect_to_stage(){
  pinMode(26,OUTPUT);
  pinMode(28,OUTPUT);
  pinMode(46,OUTPUT);
  pinMode(48,OUTPUT);

  pinMode(XEnablePin, OUTPUT);
  pinMode(ZEnablePin, OUTPUT);
  
  XStepper.setMaxSpeed(max_x_vel*32);
  XStepper.setAcceleration(x_accel*32);
  ZStepper.setMaxSpeed(max_z_vel*32);
  ZStepper.setAcceleration(z_accel*32);

  digitalWrite(XEnablePin, !EnabledX);
  digitalWrite(ZEnablePin, !EnabledZ);

  if (EEPROM.read(EEPROM_reg)) {    
    long x_steps, z_steps;

    EEPROM_readAnything(xeeprom_dir, x_steps);
    XStepper.setCurrentPosition(x_steps);

    EEPROM_readAnything(zeeprom_dir, z_steps);
    ZStepper.setCurrentPosition(z_steps);

    get_x();
    get_z();
  }
  TPOld = millis() - Tpolling/2;                  // Asynchronous polling 
  TUOld = TPOld - Tupdate/2;                      // and update
}


void run_in_loop(){

  TCurrent = millis();                           
  
  if (TCurrent - TPOld > Tpolling){             
    sCmd.readSerial();                          
    TPOld = TCurrent;                           
  }
  if (TCurrent - TUOld > Tupdate){                                      
    TUOld = TCurrent;                            
    if(!XStepper.distanceToGo() && !ZStepper.distanceToGo()){                 
      motion_complete();}                         
  }
  XStepper.run();                                
  ZStepper.run();                               
}

void help(){                                 
  for (int i = 0; i < 19; i++) {
    strcpy_P(bufptr, (char*)pgm_read_word(&(helpString[i])));
    Serial.println(buffer); }
}

void get_x(){        
  Serial.println(steps_to_position(XStepper.currentPosition()), 4);
}

void get_z(){        
  Serial.println(steps_to_position(ZStepper.currentPosition()), 4);
}

void get_x_z(){        
  Serial.println(steps_to_position(XStepper.currentPosition()), 4);
  Serial.println(steps_to_position(ZStepper.currentPosition()), 4);
}          

void motion_complete(){             
  if (XMoving){                                         // Recently moving?
      if (XMoveAbs){                                    // Check if hysteresis correction is pending
        XStepper.move(15*32);                    // ... correct hysteresis
        XMoveAbs = 0;                                   // ... and reset hysteresis variable
      }
      else {
        XMoving +=1;
        if(XMoving == 10){                              // 10 * TUpdate (0.5 sec.) after a move
          digitalWrite(XEnablePin, !(EnabledX=0));      // ... disable motor
          }
        else if(XMoving == 800){                        // 100 * (5 sec.) after a move
          EEPROM_writeAnything(EEPROM_reg +1, XStepper.currentPosition());   // Save position to EEPROM
          EEPROM.write(EEPROM_reg, true);              // ... and set EEPROM flag
          XMoving = 0; }                                // ... and reset the moving variable
      }
  }
  if (ZMoving){                                         // Recently moving?
      if (ZMoveAbs){                                    // Check if hysteresis correction is pending
        ZStepper.move(15*32);
        ZMoveAbs = 0;                                   // ... and reset hysteresis variable
      }
      else {
        ZMoving +=1;
        if(ZMoving == 10){                              // 10 * TUpdate (0.5 sec.) after a move
          digitalWrite(ZEnablePin, !(EnabledZ=0));      // ... disable motor
          }
        else if(ZMoving == 800){                        // 100 * (5 sec.) after a move
          EEPROM_writeAnything(EEPROM_reg +2, ZStepper.currentPosition());
          EEPROM.write(EEPROM_reg, true);              // ... and set EEPROM flag
          ZMoving = 0; }                                // ... and reset the moving variable
      }
  }
}

void turn_on(){
  digitalWrite(XEnablePin,LOW);
  digitalWrite(ZEnablePin,LOW);
}

void turn_off(){
  digitalWrite(XEnablePin,HIGH);
  digitalWrite(ZEnablePin,HIGH);
  if (!EEPROM.read(EEPROM_reg)){           
    EEPROM_writeAnything(xeeprom_dir, XStepper.currentPosition());
    EEPROM_writeAnything(zeeprom_dir, ZStepper.currentPosition());
    EEPROM.write(EEPROM_reg, true);}       
}

void start_movex(){
  EEPROM.write(EEPROM_reg, false);
  digitalWrite(XEnablePin,LOW);
  XMoving = 1;
}

void start_movez(){
  EEPROM.write(EEPROM_reg, false);
  digitalWrite(ZEnablePin,LOW);
  ZMoving = 1;
}
  


void get_x_target(){        
  Serial.println(steps_to_position(XStepper.targetPosition()), 4 );
}

void get_z_target(){        
  Serial.println(steps_to_position(ZStepper.targetPosition()), 4 );
}

void set_x_acceleration(){          
  char *arg;
  arg = sCmd.next();                 
  if (arg != NULL) {                
    x_accel = atol(arg);            
    XStepper.setAcceleration(x_accel);
  }
  get_x_acceleration();                   
}

void set_z_acceleration(){
  char *arg;
  arg = sCmd.next();      
  if (arg != NULL) {      
    z_accel = atol(arg);  
    ZStepper.setAcceleration(z_accel);
  }
  get_z_acceleration();            
}

void get_x_acceleration(){         
  Serial.println(x_accel);         
}

void get_z_acceleration(){         
  Serial.println(z_accel);         
}


void get_max_x_vel(){
  Serial.println(max_x_vel);                        
}

void set_max_x_vel(){
  char *arg;
  arg = sCmd.next();      
  if (arg != NULL) {      
    max_x_vel = atol(arg);   
    XStepper.setMaxSpeed(max_x_vel);
  }
  get_max_x_vel();
}


void get_max_z_vel(){
  Serial.println(max_z_vel);                        
}

void set_max_z_vel(){
  char *arg;
  arg = sCmd.next();      
  if (arg != NULL) {      
    max_z_vel = atol(arg);   
    ZStepper.setMaxSpeed(max_z_vel);
  }
  get_max_z_vel();
}




void move_to_x(){             
  char *arg;
  arg = sCmd.next();            
  if (arg != NULL) {            
    float Pos = atof(arg);      
    long Steps = position_to_steps(Pos);
    start_movex();
    if (XStepper.currentPosition() > Steps){ 
      XStepper.moveTo(Steps - (15*32)); 
      XMoveAbs = 1;
    }
    else {
    XStepper.moveTo(Steps);             
    get_x_target();             
    }
  }
  else {get_x();}              
}

void move_to_z(){             
  char *arg;
  arg = sCmd.next();            
  if (arg != NULL) {            
    float Pos = atof(arg);      
    long Steps = position_to_steps(Pos);
    start_movez();                      
    if (ZStepper.currentPosition() > Steps){ 
      ZStepper.moveTo(Steps - (15*32)); 
      ZMoveAbs = 1;                     
    }
    else {
    ZStepper.moveTo(Steps);             
    get_z_target();
    }
  }
  else {get_z();}              
}

void is_x_moving(){          
  if (XStepper.distanceToGo()!=0){
    Serial.println(1);
  }
  else {
    Serial.println(0);
  }
}

void is_z_moving(){          
  if (ZStepper.distanceToGo()!=0){
    Serial.println(1);
  }
  else {
    Serial.println(0);
  }
}

long position_to_steps(float pos){         
  return (round(pos * steps_per_mm));
}

float steps_to_position(long steps){      
  return ((float)(steps / (steps_per_mm))); 
}

void unrecognized(const char * command) {   
  Serial.print("I don't understand the command '");
  Serial.print(command);
  Serial.println("'");
}

void comm_interface(){
  
  sCmd.addCommand("help", help);

  sCmd.addCommand("xx", get_x);

  sCmd.addCommand("zz", get_z);

  sCmd.addCommand("xz", get_x_z);

  sCmd.addCommand("OFF", turn_off);

  sCmd.addCommand("XACC", set_x_acceleration);

  sCmd.addCommand("ZACC", set_z_acceleration);

  sCmd.addCommand("XACC?", get_x_acceleration);

  sCmd.addCommand("ZACC?", get_z_acceleration);

  sCmd.addCommand("MAXXVEL", set_max_x_vel);

  sCmd.addCommand("MAXZVEL", set_max_z_vel);

  sCmd.addCommand("MAXXVEL?", get_max_x_vel);

  sCmd.addCommand("MAXZVEL?", get_max_z_vel);

  sCmd.addCommand("movx", move_to_x);

  sCmd.addCommand("movz", move_to_z);

  sCmd.addCommand("ISXMOV?", is_x_moving);

  sCmd.addCommand("ISZMOV?", is_z_moving);

  sCmd.setDefaultHandler(unrecognized);     
}                     

                                     
