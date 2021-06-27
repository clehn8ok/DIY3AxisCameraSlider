#include <Arduino.h>
#include "FastAccelStepper.h"
#include "Ps3Controller.h"
#include "algorithm"




#define PAN_MOTOR_STEP_PIN 23
#define PAN_MOTOR_DIRECTION_PIN 22
#define TILT_MOTOR_STEP_PIN 16
#define TILT_MOTOR_DIRECTION_PIN 4
#define SLIDE_MOTOR_STEP_PIN 21
#define SLIDE_MOTOR_DIRECTION_PIN 19
#define MOTOR_EN_PIN 18
#define MOTOR_MS1_PIN 5
#define MOTOR_MS2_PIN 17
#define PAN_HALL_PIN 36
#define TILT_HALL_PIN 39
#define SLIDE_HALL_PIN 34
#define PAN_MAX_SPEED 2500
#define PAN_MAX_ACCELERATION 8000
#define TILT_MAX_SPEED 1250
#define TILT_MAX_ACCELERATION 8000
#define SLIDE_MAX_SPEED 3500
#define SLIDE_MAX_ACCELERATION 8000
#define KEYFRAME_ARRAY_LENGTH 10
#define SPEEDADJUSTMENTFACTOR 0.483

unsigned long previousMillis = 0; 
const long debouncedelay = 100; 
float speedfactor=1;
float accelerationfactor=1;
float panspeed_current=0;
float tiltspeed_current=0;
float slidespeed_current=0;
int keyframe_elements = 0;
int current_keyframe_index = -1;
int last_movedto_keyframe_index=-1;
int cntr_moves=0;
int cntr_moves_continous=0;
bool pan_is_moving=true;
bool tilt_is_moving=true;
bool slide_is_moving=true;
long pan_steps_to_move=0;
long tilt_steps_to_move=0;
long slide_steps_to_move=0;
long steps_to_move[3]={0,0,0};
double pan_time_motion_complete=0;
double tilt_time_motion_complete=0;
double slide_time_motion_complete=0;
int pan_speed_motion_complete=0;
int tilt_speed_motion_complete=0;
int slide_speed_motion_complete=0;
bool continous_mode=false;
int delay_continous_mode=0;

struct KeyframeElement {
    long panStepCount = 0;
    float panSpeed = 0;
    long tiltStepCount = 0;
    float tiltSpeed = 0;
    long sliderStepCount = 0;
    float sliderSpeed = 0;
    int msDelay = 0;
};

KeyframeElement keyframe_array[KEYFRAME_ARRAY_LENGTH];

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *pan_stepper = NULL;
FastAccelStepper *tilt_stepper = NULL;
FastAccelStepper *slide_stepper = NULL;

class Stick {
    private:
        bool _state;
        bool _2ndstate;
        int _pin;
        int _value;

    public:
        Stick(int pin) : _pin(pin) {}
        
        int ps3stickread(int pin2read)
        {
          switch(pin2read)
          {
            case 0:
              _value=Ps3.data.analog.stick.lx;
              break;
            case 1:
              _value=Ps3.data.analog.stick.ly;
              break;
            case 2:
               _value=Ps3.data.analog.stick.rx;
               break;
            case 3:
               _value=Ps3.data.analog.stick.ry;
               break;                    
          }          

          if(_value<-1 || _value>1)
          {
            _2ndstate=true;
          }
          if(_value==0 || _value==-1 || _value==1)
          {
            _2ndstate=false;
          }
          return _2ndstate;         
        }

        void begin() 
        {            
            _state = ps3stickread(_pin);
        }

        bool isPressed() 
        {
           bool v = ps3stickread(_pin);
           return v;
           /*
            bool v = ps3stickread(_pin);
            if (v != _state) {
                _state = v;
                if (_state) {
                    return true;
                }
            }
            return false;
            */
        }

        int getValue()
        {
           return _value;

        }
};


class Button {
    private:
        bool _state;
        bool _2ndstate;
        int _pin;
        int _value;

    public:
        Button(int pin) : _pin(pin) {}
        
        bool ps3readpin(int pin2read)
        {
          switch(pin2read)
          {
            case 0:
              _value=Ps3.data.analog.button.up;
              break;
            case 1:
              _value=Ps3.data.analog.button.down;
              break;
            case 2:
               _value=Ps3.data.analog.button.left;
               break;
            case 3:
               _value=Ps3.data.analog.button.right;
               break;
            case 4:
               _value=Ps3.data.analog.button.cross;
               break;
            case 5:
               _value=Ps3.data.analog.button.square;
               break;
            case 6:
               _value=Ps3.data.analog.button.triangle;
               break;
            case 7:
               _value=Ps3.data.analog.button.circle;
               break;
            case 8:
               _value=Ps3.data.analog.button.l1;
               break;
            case 9:
               _value=Ps3.data.analog.button.r1;
               break;
            case 10:
               _value=Ps3.data.button.l3;
               break;
            case 11:
               _value=Ps3.data.button.r3;
               break;
            case 12:
               _value=Ps3.data.button.select;
               break;
            case 13:
               _value=Ps3.data.button.start;
               break;
             case 14:
               _value=Ps3.data.button.l2;
               break;
            case 15:
               _value=Ps3.data.button.r2;
               break;
          }          

          if(_value!=0)
          {
            _2ndstate=true;
          }
          if(_value==0)
          {
            _2ndstate=false;
          }
          return _2ndstate;         
        }

        void begin() 
        {            
            _state = ps3readpin(_pin);
        }

        bool isReleased() 
        {
            bool v = ps3readpin(_pin);
            if (v != _state) {
                _state = v;
                if (_state) {
                    return true;
                }
            }
            return false;
        }
};

Button button_dPadUP(0);
Button button_dPadDOWN(1);
Button button_dPadLEFT(2);
Button button_dPadRIGHT(3);
Button button_CROSS(4);
Button button_SQUARE(5);
Button button_TRIANGLE(6);
Button button_CIRCLE(7);
Button button_L1(8);
Button button_R1(9);
Button button_L3(10);
Button button_R3(11);
Button button_SELCET(12);
Button button_START(13);
Button button_L2(14);
Button button_R2(15);

Stick stick_LX(0);
Stick stick_LY(1);
Stick stick_RX(2);
Stick stick_RY(3);


void onConnect() 
{
  Serial.println("Connected.");
}

int add_Position(void) {
  if (keyframe_elements >= 0 && keyframe_elements < KEYFRAME_ARRAY_LENGTH) {
    keyframe_array[keyframe_elements].panStepCount = pan_stepper->getCurrentPosition();
    keyframe_array[keyframe_elements].tiltStepCount = tilt_stepper->getCurrentPosition();
    keyframe_array[keyframe_elements].sliderStepCount = slide_stepper->getCurrentPosition();
    keyframe_array[keyframe_elements].panSpeed = PAN_MAX_SPEED;
    keyframe_array[keyframe_elements].tiltSpeed = TILT_MAX_SPEED;
    keyframe_array[keyframe_elements].sliderSpeed = SLIDE_MAX_SPEED;
    keyframe_array[keyframe_elements].msDelay = 0;

    Serial.print("Pan: "); 
    Serial.println(keyframe_array[keyframe_elements].panStepCount);
    Serial.print("Tilt: "); 
    Serial.println(keyframe_array[keyframe_elements].tiltStepCount);
    Serial.print("Slide: "); 
    Serial.println(keyframe_array[keyframe_elements].sliderStepCount);

    current_keyframe_index = keyframe_elements;
    keyframe_elements++;    
    Serial.print("Added at index: ");    
    Serial.println(current_keyframe_index);
    return 0;
  }
  else {
    Serial.println("Max number of keyframes reached");    
  }
  return -1;
}

void set_Speed(char _pts)
{
   switch(_pts)
   {
      case 'p':
      {
         pan_time_motion_complete=double(steps_to_move[2])/double(PAN_MAX_SPEED);
         Serial.print("time to complete pan movement: ");
         Serial.println(pan_time_motion_complete,5);

         pan_speed_motion_complete=PAN_MAX_SPEED;

         tilt_speed_motion_complete=abs(tilt_steps_to_move)/pan_time_motion_complete;
         Serial.print("tilt steps to move: ");
         Serial.println(tilt_steps_to_move);
         Serial.print("tilt speed according to pan: ");
         Serial.println(tilt_speed_motion_complete);

         slide_speed_motion_complete=abs(slide_steps_to_move)/pan_time_motion_complete;
         Serial.print("slide steps to move: ");
         Serial.println(slide_steps_to_move);
         Serial.print("slide speed according to pan: ");
         Serial.println(slide_speed_motion_complete);


         break;
      }
      case 't':
      {
         tilt_time_motion_complete=double(steps_to_move[2])/double(TILT_MAX_SPEED);
         Serial.print("time to complete tilt movement: ");
         Serial.println(tilt_time_motion_complete,5);

         tilt_speed_motion_complete=TILT_MAX_SPEED;

         pan_speed_motion_complete=abs(pan_steps_to_move)/tilt_time_motion_complete;
         Serial.print("pan steps to move: ");
         Serial.println(pan_steps_to_move);
         Serial.print("pan speed according to tilt: ");
         Serial.println(pan_speed_motion_complete);

         slide_speed_motion_complete=abs(slide_steps_to_move)/tilt_time_motion_complete;
         Serial.print("slide steps to move: ");
         Serial.println(slide_steps_to_move);
         Serial.print("slide speed according to tilt: ");
         Serial.println(slide_speed_motion_complete);

         break;
      }
      case 's':
      {
         slide_time_motion_complete=double(steps_to_move[2])/double(SLIDE_MAX_SPEED);
         Serial.print("time to complete slide movement: ");
         Serial.println(slide_time_motion_complete,5);

         slide_speed_motion_complete=SLIDE_MAX_SPEED;

         pan_speed_motion_complete=abs(pan_steps_to_move)/slide_time_motion_complete;
         Serial.print("pan steps to move: ");
         Serial.println(pan_steps_to_move);
         Serial.print("pan speed according to slide: ");
         Serial.println(pan_speed_motion_complete);

         tilt_speed_motion_complete=abs(tilt_steps_to_move)/slide_time_motion_complete;
         Serial.print("tilt steps to move: ");
         Serial.println(tilt_steps_to_move);
         Serial.print("tilt speed according to slide: ");
         Serial.println(tilt_speed_motion_complete);

         break;

      }
   }

}

void move_toPosition(int index)
{
   using namespace std;
   if(index >= 0 && index<KEYFRAME_ARRAY_LENGTH)
   {
      pan_steps_to_move=pan_stepper->getCurrentPosition()-keyframe_array[index].panStepCount;
      tilt_steps_to_move=tilt_stepper->getCurrentPosition()-keyframe_array[index].tiltStepCount;
      slide_steps_to_move=slide_stepper->getCurrentPosition()-keyframe_array[index].sliderStepCount;

      steps_to_move[0]=abs(pan_steps_to_move);
      steps_to_move[1]=abs(tilt_steps_to_move);
      steps_to_move[2]=abs(slide_steps_to_move);    

      sort(steps_to_move,steps_to_move+3);

      if(steps_to_move[2]==abs(pan_steps_to_move))
      {
         Serial.println("most steps 2 move on pan axis");
         set_Speed('p');
      }
      if(steps_to_move[2]==abs(tilt_steps_to_move))
      {
         Serial.println("most steps 2 move on tilt axis");
         set_Speed('t');
      }
      if(steps_to_move[2]==abs(slide_steps_to_move))
      {
         Serial.println("most steps 2 move on slide axis");
         set_Speed('s');
      }

      Serial.println(pan_speed_motion_complete*speedfactor);  
      Serial.println(tilt_speed_motion_complete*speedfactor);  
      Serial.println(slide_speed_motion_complete*speedfactor);   
      pan_stepper->setSpeedInHz(pan_speed_motion_complete*speedfactor);   
      tilt_stepper->setSpeedInHz(tilt_speed_motion_complete*speedfactor);
      slide_stepper->setSpeedInHz(slide_speed_motion_complete*speedfactor);


      

      pan_stepper->moveTo(keyframe_array[index].panStepCount);
      tilt_stepper->moveTo(keyframe_array[index].tiltStepCount);
      slide_stepper->moveTo(keyframe_array[index].sliderStepCount);
      

      Serial.print("moving to target index:");   
      Serial.println(index);
      current_keyframe_index = index;
      

   }
   else
   {
      Serial.println("cant move to invalid index");
   }


}

void clearKeyframes(void) 
{
  keyframe_elements = 0;
  current_keyframe_index = -1;
  last_movedto_keyframe_index=-1;
  Serial.println("Keyframes cleared");
}


void setup()
{
   Serial.begin(112500);

   button_dPadUP.begin();
   button_dPadDOWN.begin();
   button_dPadLEFT.begin();
   button_dPadRIGHT.begin();
   button_CROSS.begin();
   button_SQUARE.begin();
   button_TRIANGLE.begin();
   button_CIRCLE.begin();
   button_L1.begin();
   button_R1.begin();
   button_L3.begin();
   button_R3.begin();
   button_SELCET.begin();
   button_START.begin();
   button_L2.begin();
   button_R2.begin();
   stick_LX.begin();
   stick_LY.begin();
   stick_RX.begin();
   stick_RY.begin();
 

  Ps3.attachOnConnect(onConnect);
  Ps3.begin("20:c9:d0:d5:92:3c");
  Serial.println("Ready."); 
     

   engine.init();
   pan_stepper = engine.stepperConnectToPin(PAN_MOTOR_STEP_PIN);
   tilt_stepper = engine.stepperConnectToPin(TILT_MOTOR_STEP_PIN);
   slide_stepper = engine.stepperConnectToPin(SLIDE_MOTOR_STEP_PIN);
   if (pan_stepper) 
   {
      pan_stepper->setDirectionPin(PAN_MOTOR_DIRECTION_PIN);
      pan_stepper->setEnablePin(MOTOR_EN_PIN);
      pan_stepper->setAutoEnable(true);

      pan_stepper->setSpeedInHz(PAN_MAX_SPEED);       // 500 steps/s
      pan_stepper->setAcceleration(PAN_MAX_ACCELERATION);    // 100 steps/s²
      
   }

      if (tilt_stepper) 
    {
      tilt_stepper->setDirectionPin(TILT_MOTOR_DIRECTION_PIN);
      tilt_stepper->setEnablePin(MOTOR_EN_PIN);
      tilt_stepper->setAutoEnable(true);

      tilt_stepper->setSpeedInHz(TILT_MAX_SPEED);       // 500 steps/s
      tilt_stepper->setAcceleration(TILT_MAX_ACCELERATION);    // 100 steps/s²
      
   }
      if (slide_stepper) 
    {
      slide_stepper->setDirectionPin(SLIDE_MOTOR_DIRECTION_PIN);
      slide_stepper->setEnablePin(MOTOR_EN_PIN);
      slide_stepper->setAutoEnable(true);

      slide_stepper->setSpeedInHz(SLIDE_MAX_SPEED);       // 500 steps/s
      slide_stepper->setAcceleration(SLIDE_MAX_ACCELERATION);    // 100 steps/s²
      
   }
}

void loop() 
{
   // only runs code, if the controller is connected, otherwise everything stops
   if(Ps3.isConnected())
   {

       if(continous_mode)
         { 
            if(!(pan_stepper->isRunning()) && !(tilt_stepper->isRunning()) && !(slide_stepper->isRunning()) && keyframe_elements>1 )
            {
               delay(delay_continous_mode);
               move_toPosition(cntr_moves_continous);               
               if(keyframe_elements>cntr_moves_continous+1)
               {   
                  cntr_moves_continous++;
               }
               else
               {
                  cntr_moves_continous=0;
               } 
            }   
         }
   
      //debouncing of PS3 input
      unsigned long currentMillis = millis(); 
      if (currentMillis - previousMillis >= debouncedelay) 
      { 
         previousMillis = currentMillis; 

         //DPAD
         if(button_dPadUP.isReleased())
         {
            
            Serial.println("pressed button_dPadUP");  
            if(delay_continous_mode<1000)   
            {
               delay_continous_mode=delay_continous_mode+100; 
            }
            Serial.print("increased delay to"); 
            Serial.println(delay_continous_mode); 
               
         }
         if(button_dPadDOWN.isReleased())
         {
            Serial.println("pressed button_dPadDOWN");
            if(delay_continous_mode>=100)   
            {
               delay_continous_mode=delay_continous_mode-100; 
            }
            Serial.print("decreased delay to"); 
            Serial.println(delay_continous_mode); 
         }
         if(button_dPadLEFT.isReleased())
         {
            Serial.println("pressed button_dPadLEFT");         
         }
         if(button_dPadRIGHT.isReleased())
         {
            Serial.println("pressed button_dPadRIGHT");         
         }

         //Buttons
         if(button_CROSS.isReleased())
         {
            Serial.println("pressed button_CROSS");
            if(keyframe_elements!=0)
            {
               last_movedto_keyframe_index=cntr_moves;
               move_toPosition(last_movedto_keyframe_index);
               

               if(keyframe_elements>last_movedto_keyframe_index+1)
               {
                  cntr_moves++;                
               }
               else
               {
                  cntr_moves=0;
               }
            }        
            


           
            
                             
         }
         if(button_SQUARE.isReleased())
         {
            Serial.println("pressed button_SQUARE");
            add_Position();
            last_movedto_keyframe_index=current_keyframe_index;
            
         }
         if(button_TRIANGLE.isReleased())
         {
            
            Serial.println("pressed button_TRIANGLE");
            continous_mode=true;
            
            


         }
         if(button_CIRCLE.isReleased())
         {
            continous_mode=false;
            pan_stepper->stopMove();
            tilt_stepper->stopMove();
            slide_stepper->stopMove();         
            Serial.println("pressed button_CIRCLE");
         }

         //shoulder Buttons
         if(button_L1.isReleased())
         {
            Serial.println("pressed button_L1");
            if(speedfactor>0.000691)
            {
               speedfactor=speedfactor*SPEEDADJUSTMENTFACTOR;
            }
            Serial.println(speedfactor,5);
            /*
            pan_stepper->setSpeedInMilliHz(pan_stepper->getSpeedInMilliHz()*speedfactor);
            tilt_stepper->setSpeedInMilliHz(tilt_stepper->getSpeedInMilliHz()*speedfactor);
            slide_stepper->setSpeedInMilliHz(slide_stepper->getSpeedInMilliHz()*speedfactor);
            */
            
            
            

            
            
         }
         if(button_R1.isReleased())
         {
            Serial.println("pressed button_R1");
            if(speedfactor<1)
            {
               speedfactor=speedfactor/SPEEDADJUSTMENTFACTOR;
            }
            Serial.println(speedfactor,5);
            /*
            pan_stepper->setSpeedInMilliHz(pan_stepper->getSpeedInMilliHz()*speedfactor);
            tilt_stepper->setSpeedInMilliHz(tilt_stepper->getSpeedInMilliHz()*speedfactor);
            slide_stepper->setSpeedInMilliHz(slide_stepper->getSpeedInMilliHz()*speedfactor);
            */
         }
         if(button_L2.isReleased())
         {
            Serial.println("pressed stick_L2");  
            if(accelerationfactor>0.001)
            {
               accelerationfactor=accelerationfactor*0.5;
            }
            Serial.println(accelerationfactor);
            pan_stepper->setAcceleration(PAN_MAX_ACCELERATION*accelerationfactor);
            tilt_stepper->setAcceleration(TILT_MAX_ACCELERATION*accelerationfactor);    
            slide_stepper->setAcceleration(SLIDE_MAX_ACCELERATION*accelerationfactor);           
         }
         if(button_R2.isReleased())
         {
            Serial.println("pressed stick_R2");  
            if(accelerationfactor<1)
            {
               accelerationfactor=accelerationfactor/0.5;
            }
            Serial.println(accelerationfactor);
            pan_stepper->setAcceleration(PAN_MAX_ACCELERATION*accelerationfactor);
            tilt_stepper->setAcceleration(TILT_MAX_ACCELERATION*accelerationfactor);    
            slide_stepper->setAcceleration(SLIDE_MAX_ACCELERATION*accelerationfactor);      
         }
         if(button_L3.isReleased())
         {
            Serial.println("pressed button_L3");
            
         }
         if(button_R3.isReleased())
         {
            Serial.println("pressed button_R3");
            clearKeyframes();
            //Serial.println(slide_stepper->getCurrentPosition());
         }

         //start select
         if(button_SELCET.isReleased())
         {
            Serial.println("pressed button_SELCET");
         }
         if(button_START.isReleased())
         {
            Serial.println("pressed button_START");
         }


         //////////////////////////////////////////////////////////
         //STICK INPUT
         if(stick_LX.isPressed())
         {
            pan_is_moving=true;
            panspeed_current=speedfactor*PAN_MAX_SPEED*float(0.0078125)*abs(stick_LX.getValue());
            Serial.println("pressed stick_LX");
            Serial.println(stick_LX.getValue());
            pan_stepper->setSpeedInHz(panspeed_current);            
            Serial.println("maxspeed:");
            Serial.println(panspeed_current);

            if(stick_LX.getValue()>1)
            {
               pan_stepper->runForward();
            }
            else
            {
               pan_stepper->runBackward();
            }
         }
         else
         {
            if(pan_is_moving)
            {
               pan_stepper->stopMove();
               //pan_stepper->setSpeedInHz(0);
               pan_stepper->setDelayToDisable(65535);
               pan_is_moving=false;
            }
            
         }

         if(stick_LY.isPressed())
         {
            tilt_is_moving=true;
            tiltspeed_current=speedfactor*TILT_MAX_SPEED*float(0.0078125)*abs(stick_LY.getValue());
            Serial.println("pressed stick_LY");
            Serial.println(stick_LY.getValue());
            tilt_stepper->setSpeedInHz(tiltspeed_current);
            Serial.println("maxspeed:");
            Serial.println(tiltspeed_current);

            if(stick_LY.getValue()>1)
            {
               tilt_stepper->runForward();
            }
            else
            {
               tilt_stepper->runBackward();
            }
         }
         else
         {
            if(tilt_is_moving)
            {
               
               tilt_stepper->stopMove();
               tilt_stepper->setDelayToDisable(65535);
               //tilt_stepper->setSpeedInHz(0);
               tilt_is_moving=false;
            }
            
         }

         if(stick_RX.isPressed())
         {  
            slide_is_moving=true;
            slidespeed_current=speedfactor*SLIDE_MAX_SPEED*float(0.0078125)*abs(stick_RX.getValue());    
            Serial.println("pressed stick_RX");
            Serial.println(stick_RX.getValue());
            slide_stepper->setSpeedInHz(slidespeed_current);
            Serial.println("maxspeed:");
            Serial.println(slidespeed_current);

            if(stick_RX.getValue()>1)
            {
               slide_stepper->runBackward();
            }
            else
            {
               slide_stepper->runForward();
            }
         }
         else
         {
            if(slide_is_moving)
            {
               slide_stepper->stopMove();
               //slide_stepper->setSpeedInHz(0);
               slide_stepper->setDelayToDisable(65535);
               slide_is_moving=false;
            }
            
         }


         if(stick_RY.isPressed())
         {
            Serial.println("pressed stick_RY");
            Serial.println(stick_RY.getValue());
         }

        
      
      
      }
   }

   else
   {
      pan_stepper->stopMove();
      tilt_stepper->stopMove();
      slide_stepper->stopMove();

   }


   
}
