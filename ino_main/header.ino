#include <AccelStepper.h>
#include <SerialCommand_TS.h>   
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <avr/pgmspace.h>
#include <math.h>


char buffer[50];
char * bufptr = buffer;
unsigned long max_x_vel = 1200;  
unsigned long x_accel = 2000; 
unsigned long max_y_vel = 1200;
unsigned long y_accel = 2000;
unsigned int xbutton_pressed = 0;
unsigned int ybutton_pressed = 0;


#define YEnablePin 24
#define XEnablePin A8
#define Tpolling 200  
#define Tupdate 50
const double xsteps_per_mum = 3.2; 
const double ysteps_per_mum = 3.2;
#define y_flag 207
#define y_dir 208
#define x_flag 30
#define x_dir 31
#define xhome_switch 2
#define yhome_switch 3 //34


#define x_joystick A10
#define y_joystick A5


int x_read,y_read,x_pos,y_pos;
int EnabledX = 0;
int EnabledY = 0;
int XMoving = 0, XMoveAbs = 0;
int YMoving = 0, YMoveAbs = 0;
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
AccelStepper YStepper(1, 26, 28);
SerialCommand sCmd;

void checkLimit();

void connect_to_stage(){
  pinMode(26,OUTPUT);
  pinMode(28,OUTPUT);
  pinMode(46,OUTPUT);
  pinMode(48,OUTPUT);

  pinMode(XEnablePin, OUTPUT);
  pinMode(YEnablePin, OUTPUT);

  pinMode(xhome_switch, INPUT_PULLUP);
  pinMode(yhome_switch, INPUT_PULLUP);

  
  XStepper.setMaxSpeed(max_x_vel*32);
  XStepper.setAcceleration(x_accel*32);
  YStepper.setMaxSpeed(max_y_vel*32);
  YStepper.setAcceleration(y_accel*32);

  digitalWrite(XEnablePin, EnabledX);
  digitalWrite(YEnablePin, EnabledY);

  if (EEPROM.read(x_flag)) {    
    long x_steps;
    EEPROM_readAnything(x_dir, x_steps);
    XStepper.setCurrentPosition(x_steps);
    get_x();
    
  }
  if (EEPROM.read(y_flag)){
    long y_steps;
    EEPROM_readAnything(y_dir, y_steps);
    YStepper.setCurrentPosition(y_steps);
    get_y();
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
    ///joystick();                                      
    TUOld = TCurrent;                            
    if(!XStepper.distanceToGo() && !YStepper.distanceToGo()){                 
      motion_complete();}                         
  }
  XStepper.run();                                
  YStepper.run();
  
}

void help(){                                 
  for (int i = 0; i < 19; i++) {
    strcpy_P(bufptr, (char*)pgm_read_word(&(helpString[i])));
    Serial.println(buffer); }
}


long xposition_to_steps(float pos){         
  return (round(pos * xsteps_per_mum));
}

long yposition_to_steps(float pos){         
  return (round(pos * ysteps_per_mum));
}

float xsteps_to_position(long steps){      
  return ((float)(steps / (xsteps_per_mum))); 
}
float ysteps_to_position(long steps){      
  return ((float)(steps / (ysteps_per_mum))); 
}

void get_x(){        
  Serial.println(xsteps_to_position(XStepper.currentPosition()), 2);
}

void get_y(){        
  Serial.println(ysteps_to_position(YStepper.currentPosition()), 2);
}

void get_x_y(){        
  Serial.println(xsteps_to_position(XStepper.currentPosition()), 2);
  Serial.println(ysteps_to_position(YStepper.currentPosition()), 2);
}          

void motion_complete(){             
  if (XMoving){                                         // Recently moving?
      if (XMoveAbs){                                    // Check if hysteresis correction is pending
        //XStepper.move(32*3);                    // ... correct hysteresis
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
  if (YMoving){                                         // Recently moving?
      if (YMoveAbs){                                    // Check if hysteresis correction is pending
        //YStepper.move(32*3);
        YMoveAbs = 0;                                   // ... and reset hysteresis variable
      }
      else{ 
        YMoving +=1;
        if(YMoving == 10){                              // 10 * TUpdate (0.5 sec.) after a move
          //digitalWrite(ZEnablePin, !(EnabledY=0));      // ... disable motor
          }
        else if(YMoving == 800){                        // 100 * (5 sec.) after a move
          EEPROM_writeAnything(y_dir, YStepper.currentPosition());
          EEPROM.write(y_flag, true);              // ... and set EEPROM flag
          YMoving = 0; }                                // ... and reset the moving variable
      }
 }}

void turn_on(){
  digitalWrite(XEnablePin,LOW);
  digitalWrite(YEnablePin,LOW);
}

void turn_off(){
  digitalWrite(XEnablePin,HIGH);
  digitalWrite(YEnablePin,HIGH);
  if (!EEPROM.read(x_flag)){           
    EEPROM_writeAnything(x_dir, XStepper.currentPosition());
    EEPROM.write(x_flag, true);
    }
   if (!EEPROM.read(y_flag)){
    EEPROM_writeAnything(y_dir, YStepper.currentPosition());
    EEPROM.write(y_flag, true);
   }
}

void start_movex(){
  EEPROM.write(x_flag, false);
  //digitalWrite(XEnablePin,LOW);
  XMoving = 1;
}

void start_movey(){
  EEPROM.write(y_flag, false);
  //digitalWrite(YEnablePin,LOW);
  YMoving = 1;
}

void home_x(){
  start_movex();
  XStepper.moveTo(-10000);
}

void home_y(){
  start_movey();
  YStepper.moveTo(-10000);
}


void checkLimit(){
   if(digitalRead(xhome_switch)){
            xbutton_pressed = 1;    
            XStop();
            start_movex();
            XStepper.moveTo(XStepper.currentPosition() + 320); 
            XMoveAbs = 1;
            }
    if (xbutton_pressed and (!digitalRead(xhome_switch))){
            XStop();
            XStepper.setCurrentPosition(0);
            xbutton_pressed = 0;
            }
    if(digitalRead(yhome_switch)){
            ybutton_pressed = 1;    
            XStop() ;
            start_movex();
            XStepper.moveTo(XStepper.currentPosition() - 320); 
            XMoveAbs = 1;
            }
    if (ybutton_pressed and (!digitalRead(yhome_switch))){
            XStop();
            XStepper.setCurrentPosition(0);
            ybutton_pressed = 0;
            } 

}

  


void get_x_target(){        
  Serial.println(xsteps_to_position(XStepper.targetPosition()), 2 );
}

void get_y_target(){        
  Serial.println(zsteps_to_position(YStepper.targetPosition()), 2 );
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

void set_y_acceleration(){
  char *arg;
  arg = sCmd.next();      
  if (arg != NULL) {      
    y_accel = atol(arg);  
    YStepper.setAcceleration(y_accel);
  }
  get_y_acceleration();            
}

void set_origin(){
  YStepper.setCurrentPosition(0);
  XStepper.setCurrentPosition(0);
  EEPROM.write(x_flag, false);
  EEPROM.write(y_flag, false);
  if (!EEPROM.read(x_flag)){           
    EEPROM_writeAnything(x_dir, XStepper.currentPosition());
    EEPROM.write(x_flag, true);
    }
   if (!EEPROM.read(y_flag)){
    EEPROM_writeAnything(y_dir, YStepper.currentPosition());
    EEPROM.write(y_flag, true);
   }
}


void get_x_acceleration(){         
  Serial.println(x_accel);         
}

void get_y_acceleration(){         
  Serial.println(y_accel);         
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


void get_max_y_vel(){
  Serial.println(max_y_vel);                        
}

void set_max_y_vel(){
  char *arg;
  arg = sCmd.next();      
  if (arg != NULL) {      
    max_y_vel = atol(arg);   
    YStepper.setMaxSpeed(max_y_vel);
  }
  get_max_y_vel();
}




void move_to_x(){             
  char *arg;
  arg = sCmd.next();            
  if (arg != NULL) {            
      float Pos = atof(arg);      
      long Steps = xposition_to_steps(Pos);
      start_movex();
      XStepper.moveTo(Steps); 
      XMoveAbs = 1;
  }              
}



void move_to_y(){             
  char *arg;
  arg = sCmd.next();            
  if (arg != NULL) {            
    float Pos = atof(arg);      
    long Steps = yposition_to_steps(Pos);
    start_movey();                      
    YStepper.moveTo(Steps); 
    YMoveAbs = 1;                    
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

void is_y_moving(){          
  if (YStepper.distanceToGo()!=0){
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

void YStop(){                     // --- STOP STEPPER MOTION --- //
  YStepper.stop();                          // Stop
  YMoveAbs = 0;                             // Stop does not require hysteresis correction
}

void move_rel_in_x(int Pos){      
      start_movex();
      XStepper.move(Pos); 
      XMoveAbs = 1;              
}

void move_rel_in_y(int Pos){      
      start_movey();
      YStepper.move(Pos); 
      YMoveAbs = 1;              
}

void joystick(){  
  x_read = analogRead(x_joystick);
  y_read = analogRead(y_joystick);
  if(x_read >= 0 && x_read < 300){
    x_pos = map(x_read,0,300,-10,0);
    move_rel_in_x(x_pos);
  }
  else if(x_read >= 700 && x_read < 1024){
    x_pos = map(x_read,700,1024,0,10);
    move_rel_in_x(x_pos);

  }
  else{
    XStop();
  }

  if(y_read >= 0 && y_read < 300){
    y_pos = map(y_read,0,300,10,0);
    move_rel_in_y(y_pos);
  }
  else if(y_read >= 700 && y_read < 1024){
    y_pos = map(y_read,700,1023,0,-10);
    move_rel_in_y(y_pos);
  }
  else{
    YStop();
  }
}


void comm_interface(){
  
  sCmd.addCommand("help", help);

  sCmd.addCommand("xx", get_x);

  sCmd.addCommand("zz", get_y);

  sCmd.addCommand("xz", get_x_y);

  sCmd.addCommand("off", turn_off);

  sCmd.addCommand("xacc", set_x_acceleration);

  sCmd.addCommand("yacc", set_y_acceleration);

  sCmd.addCommand("wxacc", get_x_acceleration);

  sCmd.addCommand("wyacc", get_y_acceleration);

  sCmd.addCommand("maxxvel", set_max_x_vel);  

  sCmd.addCommand("maxyvel", set_max_y_vel);

  sCmd.addCommand("wmaxxvel", get_max_x_vel);

  sCmd.addCommand("movx", move_to_x);   

  sCmd.addCommand("movy", move_to_y);

  sCmd.addCommand("isxmov", is_x_moving);

  sCmd.addCommand("iszmoy", is_y_moving);

  sCmd.addCommand("xstop", XStop);

  sCmd.addCommand("xhome", home_x);
  
  sCmd.addCommand("yhome", home_y);

  sCmd.addCommand("ystop", YStop);
    
  sCmd.addCommand("set0",set_origin);

  sCmd.addCommand("help", help);
  sCmd.addCommand("xtarg", get_x_target);
  sCmd.addCommand("ytarg", get_y_target);

  sCmd.setDefaultHandler(unrecognized);     
}                     

                                     
