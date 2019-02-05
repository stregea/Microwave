/*
 Name: Samuel Tregea
 Professor: George Fazekas
 Date: May 16, 2018
 Description: Final Project for CSC-202
 
 This project is a simulation of a microwave that implements the following:
 
 1.) Thunderbird
 2.) LCD display
 3.) Servo Motor
 4.) Step Motor
 5.) 7-Segmented Display
 6.) A/D Converter (Thermometer)
 7.) Hex Keypad
 8.) Passive Buzzer
 9.) 2 LED's
 10.) Hardware Interrupt
 11.) Software Interrupt
 
 Most of the code has been placed into void methods which are called in the interrupt when a
 "flag" is raised, allowing the multiplexing of the 7-Segmented Disaplay to occur without raising any errors.
 
 Flags:
 0 = pause
 1 = countdown()
 2 = rotate()
 3 = lockServo()
 
 LEDS:
 
 Red - means microwave is locked and cannot be used
 Green - microwave is open and is ready for input.
 
 */

#include <hidef.h>      /* common defines and macros */
#include <mc9s12dg256.h>     /* derivative information */
#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"

#include "main_asm.h" /* interface to the assembly module */

/*
  Function Declarations
*/
void countDown(void);
void lockServo(void);
void unlockServo(void);
void displayTemp(void);
void beep(void);
void alarm(void);
void rotate(void);
void redLEDOn(void);
void redLEDOff(void);
void greenLEDOn(void);
void greenLEDOff(void);

//100 ticks ~ 1 second, may change due to several millisecond delays
  int ticks = 0;
  
  int i, j; //Used for for-loops
  int flag1; //Flag that's to be raised and used in the software interrupt
  int temperature; //Used to display the temperature
  int width; //used for the Servo methods

  char c;
  char *message; //String variable
  int digits_size = 3;

  int digits[] = {
     0,0,0,0      
  };
  
  unsigned int arry[] = {0x9,0x3,0x6,0xC};



void interrupt 7 handle(){ //real-time interrupt
     
  if(flag1 == 1){
    countDown();
  }
  if(flag1 == 2){
    rotate(); 
  }
  if(flag1 == 3){
    lockServo();
  }
  
  clear_RTI_flag(); 
}

 

void main(void){

  PLL_init();     //set clock frequency to 24 MHz
  lcd_init();     //enable LCD
  SCI0_int_init(9600);  //initialize SCI0 at 9600 baud  
  ad0_enable();     //enable a a/d converter 0 
  keypad_enable();//enable keypad

  DDRB = 0xff;  //Enabling PortB for 7 segmented display
  DDRP = 0xff;  //Enabling Port P for 7 segmented display
  PTP = 0xff;    //Port P, enabling one segment, use '0' to use all leds (uses negative logic)
  DDRT = 0xf;    //Enabling Port T for use of alarm
  RTI_init();
  servo76_init();       //enable pwm1 for servo
 
  unlockServo();    //"opening" the microwave, allowing it to be ready for use
  greenLEDOn();
  clear_lcd();
  set_lcd_addr(0x00);     //display on first row LCD
  message = "Ready for Input";
  type_lcd(message);
     
  while(1){
   
    //Preset time to 30 seconds
    if(keyscan() == 10){
      wait_keyup();
      clear_lcd();
      set_lcd_addr(0x00);//display on first row LCD
      message = "Preset Mode";
      type_lcd(message);
      beep();

      unlockServo();
      greenLEDOn();      
      
      digits[0] = 0;
      digits[1] = 0;
      digits[2] = 3;
      digits[3] = 0;    }
    //Preset time to 01:00 minutes
    if(keyscan() == 11){
      wait_keyup();
      clear_lcd();
      set_lcd_addr(0x00);//display on first row LCD
      message = "Preset Mode";
      type_lcd(message);
      beep();

      unlockServo();
      greenLEDOn();     
     
      digits[0] = 0;
      digits[1] = 1;
      digits[2] = 0;
      digits[3] = 0; 
    }
    //Preset time to 05:00 minutes
    if(keyscan() == 12)
    {
      wait_keyup();
      clear_lcd();
      set_lcd_addr(0x00);//display on first row LCD
      message = "Preset Mode";
      type_lcd(message);
      beep();

      unlockServo();
      greenLEDOn();  
    
      digits[0] = 0;
      digits[1] = 5;
      digits[2] = 0;
      digits[3] = 0; 
    }
    //Preset time to 10:00 minutes
    if(keyscan() == 13)
    {
      wait_keyup();
      clear_lcd();
      set_lcd_addr(0x00);     //display on first row LCD
      message = "Preset Mode";
      type_lcd(message);
      beep();

      unlockServo();
      greenLEDOn();    
    
      digits[0] = 1;
      digits[1] = 0;
      digits[2] = 0;
      digits[3] = 0; 
    } 
    //raising a flag for when the user presses"*"
    //locks servo, then counts down 
    if(keyscan() == 14)
    {
       flag1 = 3; //got to lockServo();
    }
    //resetting the flag
    if(keyscan() != 16 && keyscan() != 10 && keyscan() != 11 && keyscan() != 12 && keyscan() != 13 && keyscan() != 14 && keyscan() != 15)
    {
      flag1 = 0;//pause
      
      c = getkey();
      wait_keyup();
      clear_lcd();
      set_lcd_addr(0x00);//display on first row LCD
      message = "Input Mode";
      type_lcd(message);
      beep();

      unlockServo();
      greenLEDOn();
     
      //moving the digits to the left
      //using negative logic
      for(j = 0; j < 3; j++)
      {
        digits[j] = digits[j+1]; 
      }
       
      //setting the rightmost display to the keyscan
      digits[3] = c;
              
    }
   
    /*
      multiplexing
    */
    for(i = 0; i<4; i++)
    {
        seg7dec(digits[i],i);
        ms_delay(1);
    }

}//while
     
  asm swi;//returns to the controller and resets it
  
}//main


//counts down
void countDown(void){
    ticks++;
    redLEDOn();   
   if(ticks == 60){
    ticks = 0;  //resetting ticks
   if(digits[3] > 0)
   {
      digits[3]--;
   }//if digits[3]
   else{
      if(digits[2] > 0){
        digits [3] = 9;
        digits[2]--;     
      }
      else{
        if(digits[1] > 0){
          digits[3] = 9;
          digits[2] = 5;
          digits[1]--; 
        } else{
          if(digits[0] > 0){
           digits[3] = 9; 
           digits[2] = 5; 
           digits[1] = 9;
           digits[0]--; 
           //When timer hits 0
          } else{
             flag1 = 0;//pause flag when all segments are 0
             digits[0] = 0;
             digits[1] = 0;
             digits[2] = 0;
             digits[3] = 0;
    
             unlockServo();//opening the microwave
             clear_lcd();
             set_lcd_addr(0x00);//display on first row LCD
             message = "Cooking";
             type_lcd(message);
      
             set_lcd_addr(0x40);//display on second row LCD
             message = "Finished";
             type_lcd(message);   
             alarm();
             greenLEDOn();//green = microwave is ready to be used, turns off red LED
            
          }
          
        }
      }
   }
  }//if ticks
  
  //accesses when timer reaches 0
 /* if(digits[0] == 0 && digits[1] == 0 && digits[2] == 0 && digits[3] == 0){
            
             unlockServo();//opening the microwave
             clear_lcd();
             set_lcd_addr(0x00);//display on first row LCD
             message = "Cooking";
             type_lcd(message);
             set_lcd_addr(0x40);//display on second row LCD
             message = "Finished";
             type_lcd(message);   
             alarm();
             greenLEDOn();//green = microwave is ready to be used, turns off red LED
             flag1 = 0;//pause flag when all segments are 0


  }*/  
  else{//else, continue rotating motor

    /*
     multiplexing here prevents an issue in
     which the display would continuously blink.
     */
    for(i = 0; i<4; i++)
    {
        seg7dec(digits[i],i);
        ms_delay(1);
    }
    flag1 = 2; //go to rotate()

  }
}//countDown

/*
  Turns the servo motor to simulate a "lock" while the
  microwave is turned on.
*/
void lockServo(void){
         
    for(width = 3000; width <= 5500; width = width + 5){
     set_servo76(width);   //move servo from 3000 to 5500
    }
    
    displayTemp();
    flag1 = 1;//go to countDown()
}

/*
 "unlocks" the servo motor
*/
void unlockServo(void){
    
    for(width = 5500; width >= 3000; width = width - 5){
      set_servo76(width);   //move servo from 5500 to 3000
    }
     
     flag1 = 0;//do nothing 
}

/*
  Displays the current temperature of the microwave
*/
void displayTemp(void){

    temperature = ad0conv(7);       //read pot on channel 7
    //temperature = temperature >> 1;//divide by 2
    set_lcd_addr(0x00);     //display on first row LCD
    message = "Current Temp:";
    type_lcd(message);
    set_lcd_addr(0x40);     //display on 2nd row LCD    
    write_int_lcd(temperature);     //write value in field
    message = " degrees F";
    type_lcd(message);
 }

/*
    Used to create a beeping sound when a user hits a
    button on the hex keypad
 */
void beep(void){
 DDRE = 0xff;
 PORTE = 0xff;
 ms_delay(5);
 PORTE = 0x00;
  
}

/*
    called when countdown() finishes
 
    Beeps buzzer 3 times along with a flashing external LED
 */
void alarm(void){
 DDRE = 0xff;
 PORTE = 0xff;
 redLEDOn();
 ms_delay(450);//firstbeep
 PORTE = 0x00;
 redLEDOff();
 ms_delay(450); 
 PORTE = 0xff; //second beep
 redLEDOn();
 ms_delay(450);
 PORTE = 0x00;
 redLEDOff();
 ms_delay(450); 
 PORTE = 0xff;
 redLEDOn();
 ms_delay(450);//firstbeep
 PORTE = 0x00;
 redLEDOff();
 
}

/*
    Rotates the Step Motor
 */
void rotate(void){


    for(i = 0; i<4;i++){
    PTT = arry[i];
    ms_delay(2);
   }
   
    /*
     interrupt command
     */
   flag1 = 1;//go to countDown()

}

/*
    Turns on the red LED
 */
void redLEDOn(void){
 DDRM = 0x02; //m1
 PTM = 0x02;  
}

/*
    Turns off the red LED
 */
void redLEDOff(void){
 DDRM = 0x02;
 PTM = 0x00;
}

/*
    Turns on the green LED
 */
void greenLEDOn(void){
 DDRM = 0x04; //m2
 PTM = 0x04;  
}

/*
    Turns off the green LED
 */
void greenLEDOff(void){
 DDRM = 0x04;
 PTM = 0x00;
}
