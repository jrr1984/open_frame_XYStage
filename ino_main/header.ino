#include <AccelStepper.h>
#include <SerialCommand_TS.h>   
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <avr/pgmspace.h>
#include <math.h>


char buffer[50];
char * bufptr = buffer;
unsigned long max_x_vel = 800;  
unsigned long x_accel = 1800; 
unsigned long max_z_vel = 800;
unsigned long z_accel = 1800;
unsigned int xbutton_pressed = 0;
unsigned int zbutton_pressed = 0;


#define ZEnablePin 24
#define XEnablePin A8
#define Tpolling 200  
#define Tupdate 50
const double xsteps_per_mum = 3.2; 
const double zsteps_per_mum = 3.2;
#define z_flag 207
#define z_dir 208
#define x_flag 30
#define x_dir 31
#define xhome_switch 2
#define zhome_switch 3 //34


#define x_joystick A0
#define z_joystick A1
#define button_joystick 10

int x_read,z_read,x_pos,z_pos,button_joystick_read;

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
AccelStepper XStepper(1, 46,48); //driver, step, direction
AccelStepper ZStepper(1, 26, 28);
SerialCommand sCmd;

void checkLimit();

void connect_to_stage(){
  pinMode(26,OUTPUT);
  pinMode(28,OUTPUT);
  pinMode(46,OUTPUT);
  pinMode(48,OUTPUT);

  pinMode(XEnablePin, OUTPUT);
  pinMode(ZEnablePin, OUTPUT);

  pinMode(xhome_switch, INPUT_PULLUP);
  pinMode(zhome_switch, INPUT_PULLUP);

  pinMode(button_joystick,INPUT_PULLUP);
  
  XStepper.setMaxSpeed(max_x_vel*32);
  XStepper.setAcceleration(x_accel*32);
  ZStepper.setMaxSpeed(max_z_vel*32);
  ZStepper.setAcceleration(z_accel*32);

  digitalWrite(XEnablePin, EnabledX);
  digitalWrite(ZEnablePin, EnabledZ);

  if (EEPROM.read(x_flag)) {    
    long x_steps;
    EEPROM_readAnything(x_dir, x_steps);
    XStepper.setCurrentPosition(x_steps);
    get_x();
    
  }
  if (EEPROM.read(z_flag)){
    long z_steps;
    EEPROM_readAnything(z_dir, z_steps);
    ZStepper.setCurrentPosition(z_steps);
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
    checkLimit();                                      
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


long xposition_to_steps(float pos){         
  return (round(pos * xsteps_per_mum));
}

long zposition_to_steps(float pos){         
  return (round(pos * zsteps_per_mum));
}

float xsteps_to_position(long steps){      
  return ((float)(steps / (xsteps_per_mum))); 
}
float zsteps_to_position(long steps){      
  return ((float)(steps / (zsteps_per_mum))); 
}

void get_x(){        
  Serial.println(xsteps_to_position(XStepper.currentPosition()), 2);
}

void get_z(){        
  Serial.println(zsteps_to_position(ZStepper.currentPosition()), 2);
}

void get_x_z(){        
  Serial.println(xsteps_to_position(XStepper.currentPosition()), 2);
  Serial.println(zsteps_to_position(ZStepper.currentPosition()), 2);
}          

void motion_complete(){             
  if (XMoving){                                         // Recently moving?
      if (XMoveAbs){                                    // Check if hysteresis correction is pending
        //XStepper.move(1);                    // ... correct hysteresis
        XMoveAbs = 0;                                   // ... and reset hysteresis variable
      }
      else {
        XMoving +=1;
        if(XMoving == 10){                              // 10 * TUpdate (0.5 sec.) after a move
          //digitalWrite(XEnablePin, !(EnabledX=0));      // ... disable motor
          }
        else if(XMoving == 800){                        // 100 * (5 sec.) after a move
          EEPROM_writeAnything(x_dir, XStepper.currentPosition());   // Save position to EEPROM
          EEPROM.write(x_flag, true);              // ... and set EEPROM flag
          XMoving = 0; }                                // ... and reset the moving variable
      }
  }
  if (ZMoving){                                         // Recently moving?
      if (ZMoveAbs){                                    // Check if hysteresis correction is pending
        //ZStepper.move(1);
        ZMoveAbs = 0;                                   // ... and reset hysteresis variable
      }
      else 
        ZMoving +=1;
        if(ZMoving == 10){                              // 10 * TUpdate (0.5 sec.) after a move
          //digitalWrite(ZEnablePin, !(EnabledZ=0));      // ... disable motor
          }
        else if(ZMoving == 800){                        // 100 * (5 sec.) after a move
          EEPROM_writeAnything(z_dir, ZStepper.currentPosition());
          EEPROM.write(z_flag, true);              // ... and set EEPROM flag
          ZMoving = 0; }                                // ... and reset the moving variable
      }
 }

void turn_on(){
  digitalWrite(XEnablePin,LOW);
  digitalWrite(ZEnablePin,LOW);
}

void turn_off(){
  digitalWrite(XEnablePin,HIGH);
  digitalWrite(ZEnablePin,HIGH);
  if (!EEPROM.read(x_flag)){           
    EEPROM_writeAnything(x_dir, XStepper.currentPosition());
    EEPROM.write(x_flag, true);
    }
   if (!EEPROM.read(z_flag)){
    EEPROM_writeAnything(z_dir, ZStepper.currentPosition());
    EEPROM.write(z_flag, true);
   }
}

void start_movex(){
  EEPROM.write(x_flag, false);
  //digitalWrite(XEnablePin,LOW);
  XMoving = 1;
}

void start_movez(){
  EEPROM.write(z_flag, false);
  //digitalWrite(ZEnablePin,LOW);
  ZMoving = 1;
}

void home_x(){
  start_movex();
  XStepper.moveTo(-10000);
}

void home_z(){
  start_movez();
  ZStepper.moveTo(-10000);
}


void checkLimit(){
   if(digitalRead(xhome_switch)){
            xbutton_pressed = 1;    
            ZStop();
            start_movez();
            ZStepper.moveTo(ZStepper.currentPosition() - 320); 
            ZMoveAbs = 1;
            }
    if (xbutton_pressed and (!digitalRead(xhome_switch))){
            ZStop();
            ZStepper.setCurrentPosition(0);
            xbutton_pressed = 0;
            }
    if(digitalRead(zhome_switch)){
            zbutton_pressed = 1;    
            ZStop() ;
            start_movez();
            ZStepper.moveTo(ZStepper.currentPosition() + 320); 
            ZMoveAbs = 1;
            }
    if (zbutton_pressed and (!digitalRead(zhome_switch))){
            ZStop();
            ZStepper.setCurrentPosition(0);
            zbutton_pressed = 0;
            } 

}

  


void get_x_target(){        
  Serial.println(xsteps_to_position(XStepper.targetPosition()), 2 );
}

void get_z_target(){        
  Serial.println(zsteps_to_position(ZStepper.targetPosition()), 2 );
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

void set_origin(){
  ZStepper.setCurrentPosition(0);
  XStepper.setCurrentPosition(0);
  EEPROM.write(x_flag, false);
  EEPROM.write(z_flag, false);
  if (!EEPROM.read(x_flag)){           
    EEPROM_writeAnything(x_dir, XStepper.currentPosition());
    EEPROM.write(x_flag, true);
    }
   if (!EEPROM.read(z_flag)){
    EEPROM_writeAnything(z_dir, ZStepper.currentPosition());
    EEPROM.write(z_flag, true);
   }
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
void move_steps(){

  
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
      long Steps = xposition_to_steps(Pos);
        start_movex();
        if (XStepper.currentPosition() > Steps){ 
          XStepper.moveTo(Steps); 
          XMoveAbs = 1;
        }
        else {
          XStepper.moveTo(Steps);                
        }}              
}



void move_to_z(){             
  char *arg;
  arg = sCmd.next();            
  if (arg != NULL) {            
    float Pos = atof(arg);      
    long Steps = zposition_to_steps(Pos);
    start_movez();                      
    if (ZStepper.currentPosition() > Steps){ 
      ZStepper.moveTo(Steps); 
      ZMoveAbs = 1;                     
    }
    else {
    ZStepper.moveTo(Steps);             
    }
  }
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



void unrecognized(const char * command) {   
  Serial.print("I don't understand the command '");
  Serial.print(command);
  Serial.println("'");
}



void XStop(){                     // --- STOP STEPPER MOTION --- //
  XStepper.stop();                          // Stop
  XMoveAbs = 0;                             // Stop does not require hysteresis correction
}

void ZStop(){                     // --- STOP STEPPER MOTION --- //
  ZStepper.stop();                          // Stop
  ZMoveAbs = 0;                             // Stop does not require hysteresis correction
}

void move_rel_in_x(int Pos){      
      start_movex();
      XStepper.move(Pos); 
      XMoveAbs = 1;              
}

void move_rel_in_z(int Pos){      
      start_movez();
      ZStepper.move(Pos); 
      ZMoveAbs = 1;              
}

void joystick(){  
  x_read = analogRead(x_joystick);
  z_read = analogRead(z_joystick);
  button_joystick_read = digitalRead(button_joystick);
  if(x_read >= 0 && x_read < 492){ //20 menos que la mitad de 1024
    x_pos = map(x_read,0,492,-1000,0);
    move_rel_in_x(x_pos);
  }
  else if(x_read >= 532 && x_read < 1023){
    x_pos = map(x_read,532,1023,0,1000);
    move_rel_in_x(x_pos);
  }
  else{
    XStop();
  }

  if(z_read >= 0 && z_read < 492){ //20 menos que la mitad de 1024
    z_pos = map(z_read,0,492,-1000,0);
    move_rel_in_z(z_pos);
  }
  else if(z_read >= 532 && z_read < 1023){
    z_pos = map(z_read,532,1023,0,1000);
    move_rel_in_z(z_pos);
  }
  else{
    ZStop();
  }
  if 

  
}

void comm_interface(){
  
  sCmd.addCommand("help", help);

  sCmd.addCommand("xx", get_x);

  sCmd.addCommand("zz", get_z);

  sCmd.addCommand("xz", get_x_z);

  sCmd.addCommand("off", turn_off);

  sCmd.addCommand("xacc", set_x_acceleration);

  sCmd.addCommand("zacc", set_z_acceleration);

  sCmd.addCommand("wxacc", get_x_acceleration);

  sCmd.addCommand("wzacc", get_z_acceleration);

  sCmd.addCommand("maxxvel", set_max_x_vel);  

  sCmd.addCommand("maxzvel", set_max_z_vel);

  sCmd.addCommand("wmaxxvel", get_max_x_vel);

  sCmd.addCommand("wmaxxvel", get_max_z_vel);

  sCmd.addCommand("movx", move_to_x);   

  sCmd.addCommand("movz", move_to_z);

  sCmd.addCommand("isxmov", is_x_moving);

  sCmd.addCommand("iszmov", is_z_moving);

  sCmd.addCommand("xstop", XStop);

  sCmd.addCommand("xhome", home_x);
  
  sCmd.addCommand("zhome", home_z);

  sCmd.addCommand("zstop", ZStop);
    
  sCmd.addCommand("set0",set_origin);

  sCmd.addCommand("help", help);
  sCmd.addCommand("xtarg", get_x_target);
  sCmd.addCommand("ztarg", get_z_target);

  sCmd.setDefaultHandler(unrecognized);     
}                     

                                     
