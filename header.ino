#include <AccelStepper.h>
#include <SerialCommand_TS.h>       // Command library to handle serial commands. The _TS version is case insensitive.
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <avr/pgmspace.h>
#include <math.h>


char buffer[50];
char * bufptr = buffer;
unsigned long max_x_vel = 700;           
unsigned long x_accel = 1000;
unsigned long max_z_vel = 700;           
unsigned long z_accel = 1000;
int EnabledX = 0;
int EnabledZ = 0;
int Moving = 0;
int MoveAbs = 0;
int steps_per_mm = 6400;            
long TCurrent;
unsigned long EEPROM_reg = 20;
long Tpolling = 200;
long Tupdate = 50;
long TPOld, TUOld;

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
  pinMode(A8,OUTPUT);
  
  XStepper.setMaxSpeed(max_x_vel*32);
  XStepper.setAcceleration(x_accel*32);
  ZStepper.setMaxSpeed(max_z_vel*32);
  ZStepper.setAcceleration(z_accel*32);

   

  digitalWrite(A8, !EnabledX);
  digitalWrite(24, !EnabledZ);
  if (EEPROM.read(EEPROM_reg)) {    
    long x_steps, z_steps;
    EEPROM_readAnything(EEPROM_reg+1, x_steps);
    EEPROM_readAnything(EEPROM_reg+2, z_steps);  
    XStepper.setCurrentPosition(x_steps);
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
  Serial.print(steps_to_position(XStepper.currentPosition()), 4);
  Serial.print(" , ");
  Serial.println(steps_to_position(ZStepper.currentPosition()), 4);
}          

void motion_complete(){             
  if (Moving){                               
      if (MoveAbs){                          
        XStepper.move(15*32);
        ZStepper.move(15*32);                 
        MoveAbs = 0;                         
      }
      else {      
        Moving +=1;
        if(Moving == 10){                    
         // ... disable motors
          digitalWrite(A8, !(EnabledX=0));
          digitalWrite(24, !(EnabledZ=0));
          }       
        else if(Moving == 800){                        
          EEPROM_writeAnything(EEPROM_reg +1, XStepper.currentPosition());
          EEPROM_writeAnything(EEPROM_reg +2, ZStepper.currentPosition());      
          EEPROM.write(EEPROM_reg, true);              
          Moving = 0; }                                
      }
  }
}

void turn_on(){
  digitalWrite(A8,LOW);
  digitalWrite(24,LOW);
}

void turn_off(){
  digitalWrite(A8,HIGH);
  digitalWrite(24,HIGH);
  if (!EEPROM.read(EEPROM_reg)){           
    EEPROM_writeAnything(EEPROM_reg +1, XStepper.currentPosition());
    EEPROM_writeAnything(EEPROM_reg +2, ZStepper.currentPosition());
    EEPROM.write(EEPROM_reg, true);}       
}

void start_move(){ 
  EEPROM.write(EEPROM_reg, false); 
  turn_on();               
  Moving = 1;              
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
    start_move();                      
    if (XStepper.currentPosition() > Steps){ 
      XStepper.moveTo(Steps - (15*32)); 
      MoveAbs = 1;                     
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
    start_move();                      
    if (ZStepper.currentPosition() > Steps){ 
      ZStepper.moveTo(Steps - (15*32)); 
      MoveAbs = 1;                     
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

  sCmd.addCommand("XZPOS?", get_x_z);

  sCmd.addCommand("OFF", turn_off);

  sCmd.addCommand("XACC", set_x_acceleration);

  sCmd.addCommand("ZACC", set_z_acceleration);

  sCmd.addCommand("XACC?", get_x_acceleration);

  sCmd.addCommand("ZACC?", get_z_acceleration);

  sCmd.addCommand("MAXXVEL", set_max_x_vel);

  sCmd.addCommand("MAXZVEL", set_max_z_vel);

  sCmd.addCommand("MAXXVEL?", get_max_x_vel);

  sCmd.addCommand("MAXZVEL?", get_max_z_vel);

  sCmd.addCommand("mox", move_to_x);

  sCmd.addCommand("movz", move_to_z);

  sCmd.addCommand("ISXMOV?", is_x_moving);

  sCmd.addCommand("ISZMOV?", is_z_moving);

  sCmd.setDefaultHandler(unrecognized);     
}                     

                                     
