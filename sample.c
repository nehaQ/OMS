// Source code using the ViSi-Genie-RaspPi library
// 4D Systems - Office Messaging System
// Allows users to leave/ select a message from to leave for
// the owner of the door. These messages maybe accessed later
// by the owner. The basic setup of the screen is a keyboard 
// with a string object which displays the message typed 
// through the keyboard. 
// Form 0 - Leave a message/Read messages screen for the user
//	    Winbutton0, Winbutton1
// Form 1 - Set a message/Pick a message screen for the user
//	    Winbutton2, Winbutton3
// Form 2 - Pick a message screen. Uses a slider to choose the message 
//	    user wants to display
//	    Slider0, StaticText0 to StaticText5, Leddigits3
// Form 3 - Screen with a keyboard for the user to type out the message
//	    Keyboard0, Strings0, Winbutton4
// Form 4 - Screen to set countdown timer. Uses a slider and LED digits
//	    Slider1, StaticText6, StaticText7, Leddigits0, Winbutton5
// Form 5 - Screen which is displayed to the visitor
//	    Leddigits1, Leddigits2, StaticText8 to StaticText10, Strings1
//	    Strings2, Winbutton6, Winbutton7
// Form 6 - Pick a message/Set Message screen for the visitor
//	    Winbutton8, Winbutton9
// Form 7 - Keyboard for the visitor
//	    Keyboard1, Strings3, Winbutton10
// Form 8 - Preset messages for visitor with slider to choose
//	    Leddigits4, Slider2, StaticText11 to 13
// Form 9 - Acknowledgement screen for the visitor
//	    Image0, StaticText14
// Form 10- PIN entered access for the user
//	    Keyboard2, StaticText15, Strings4, Winbutton11
// Form 11- Read your messages screen for the user
//	    StaticText16, Strings5, Winbutton12

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#include <geniePi.h>  //the ViSi-Genie-RaspPi library

int display;
int count;
char buf [500]; // Buffer to hold string to print to screen
char message [500];
char userMsg[1000]; // Store the users messages
char password[20]; // Store the users password
int count = 0; // Count the number of characters being printed
int tortoise = 10;

//-----------------------------------------------------------------------

// Runs the countdown on the second form
static void *clockWork(void * data)
{
  struct sched_param sched;
  int pri = 10;

  time_t tt;
  struct tm timeData;
  //set to real time prioority 
  
  memset(&sched, 0,  sizeof(sched));
  if(pri>sched_get_priority_max (SCHED_RR)) pri = sched_get_priority_max(SCHED_RR);
  
  sched.sched_priority = pri;
  sched_setscheduler (0, SCHED_RR, &sched);

  sleep(1);
  tt = time(NULL);
    (void) localtime_r(&tt, &timeData);
  int startSec = tortoise*60;
  
  for(;;)
  {
    tt = time(NULL);
    while (time(NULL) == tt)  
      usleep(10000);
    
    tt = time(NULL);
    (void) localtime_r(&tt, &timeData);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 2, startSec/60); //minutes
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 1, startSec%60);
    startSec--;
    //tortoise--;
  }
  return (void *)NULL;
}

//-----------------------------------------------------------------------

//Update the display
void updateDisplay(int string)
{
  //sprintf(buf, "%s%c", buf, display);
  //printf("%s\n ", buf);
  genieWriteStr(string, buf); //strings
}

//-----------------------------------------------------------------------

//This is the event handler. Messages received from the display
//are processed here.
void handleGenieEvent(struct genieReplyStruct * reply)
{
  //check if the cmd byte is a report event
  if(reply->cmd == GENIE_REPORT_EVENT)    
  {
    //check if the object byte is that of a keyboard
    if(reply->object == GENIE_OBJ_KEYBOARD) 
    {
      // Check which keyboard it came from
      if(reply->index == 0)		  
      {
        //write to the data passed through the keyboard
	if(reply->data == 8)
	{
	  count--;
	  if(count < 0) count = 0;
	  buf[count] = '\0';
	  updateDisplay(0);
	}
	else
	{
	  buf[count] = reply->data;
	  count++;
	  updateDisplay(0);
	}
      }
      if(reply->index == 1)
      {
        //write to the data passed through the keyboard
	if(reply->data == 8)
	{
	  count--;
	  if(count < 0) count = 0;
	  buf[count] = '\0';
	  updateDisplay(3);
	}
	else
	{
	  buf[count] = reply->data;
	  count++;
	  updateDisplay(3);
	}
	
      }
      if(reply->index == 2)
      {
	//write to the data passed through the keyboard
	if(reply->data == 8)
	{
	  count--;
	  if(count < 0) count = 0;
	  buf[count] = '\0';
	  updateDisplay(4);
	}
	else
	{
	  buf[count] = '*';
	  count++;
	  updateDisplay(4);
	}
      }
    }

    if(reply->object == GENIE_OBJ_WINBUTTON)
    {
      if(reply->index == 0)
      {
	// Go to form1 (Pick/set msg for user)
	genieWriteObj(GENIE_OBJ_FORM, 1, 0);
	//updateDisplay();
      }
      if(reply->index == 1)
      {
	// Go to form11 (Read your msgs screen)
	genieWriteObj(GENIE_OBJ_FORM, 11, 0);
	//updateDisplay();
      }
      if(reply->index == 2)
      {
	// Go to form3 (Keybd for user)
	genieWriteObj(GENIE_OBJ_FORM, 3, 0);
	//updateDisplay();
      }
      if(reply->index == 3)
      {
	// Go to form2 (Pick msg for user)
	genieWriteObj(GENIE_OBJ_FORM, 2, 0);
	genieWriteObj(GENIE_OBJ_SLIDER, 0, 0);
	genieWriteObj(GENIE_OBJ_LED_DIGITS, 3, 0);
	//updateDisplay();
      }
      if(reply->index == 4)
      {
      	sprintf(message, "%s", buf);
        // Clear the buffer
        memset(&buf[0], 0, sizeof(buf));
        count = 0;
        
      	// Go to form4 (set timer screen for user)
	genieWriteObj(GENIE_OBJ_FORM, 4, 0);
	// Set slider to 0
	genieWriteObj(GENIE_OBJ_SLIDER, 1, 0);
	genieWriteObj(GENIE_OBJ_LED_DIGITS, 0, 0);
	//updateDisplay();
      }
      if(reply->index == 5)
      {
	// Go to form5 (Visitor screen)
	genieWriteObj(GENIE_OBJ_FORM, 5, 0);
	genieWriteStr(1, message);
	// Need to add code to start countdown
      }
      if(reply->index == 6)
      {
	// Go to form10 (the user is back screen)
	genieWriteObj(GENIE_OBJ_FORM, 10, 0);
      }
      if(reply->index == 7)
      {
	// Go to form6 (visitor leaves a msg screen)
	genieWriteObj(GENIE_OBJ_FORM, 6, 0);
      }
      if(reply->index == 8)
      {
	// Go to form7 (keyboard screen for vistor)
	genieWriteObj(GENIE_OBJ_FORM, 7, 0);
      }
      if(reply->index == 9)
      {
	// Go to form8 (Picks a msg screenfor visitor)
	genieWriteObj(GENIE_OBJ_FORM, 8, 0);
	genieWriteObj(GENIE_OBJ_SLIDER, 2, 0);
	genieWriteObj(GENIE_OBJ_LED_DIGITS, 4, 0);
      }
      if(reply->index == 10)
      {
        sprintf(userMsg, "%s||%s",userMsg, buf);
        // Clear the buffer
        memset(&buf[0], 0, sizeof(buf));
        count = 0;
	// Go to form9
	genieWriteObj(GENIE_OBJ_FORM, 9, 0);
	usleep(2000000);
	// Return to vistor screen
	genieWriteObj(GENIE_OBJ_FORM, 5, 0);
      }
      if(reply->index == 11)
      {
        sprintf(password, "%s", buf);
        // Clear the buffer
        memset(&buf[0], 0, sizeof(buf));
        count = 0;
        
	// Go to form0 (Back to user home screen)
	genieWriteObj(GENIE_OBJ_FORM, 0, 0);
	genieWriteStr(5, userMsg);
      }
      if(reply->index == 12)
      {
	// Go to form0 (back to user home screen)
	genieWriteObj(GENIE_OBJ_FORM, 0, 0);
	//updateDisplay();
      }
    }// End-if Winbutton
    
    if(reply->object == GENIE_OBJ_SLIDER)
    {
      if(reply->index == 0)
      {
        // Write to the LED digits the value of slider
        genieWriteObj(GENIE_OBJ_LED_DIGITS, 0x03, reply->data);
      }
      if(reply->index == 1)
      {
        // Write to the LED digits the value of slider
        genieWriteObj(GENIE_OBJ_LED_DIGITS, 0x00, reply->data);
        // Store the time to display later
        tortoise = reply->data;        
      }
      if(reply->index == 2)
      {
        // Write to the LED digits the value of slider
        genieWriteObj(GENIE_OBJ_LED_DIGITS, 0x04, reply->data);
      }
    }// End-if slider event
 
  }// End-if report event
  
  // Print error message
  else
    printf("Unhandled event: command: %2d, object: %2d, index: %d, data: %d \r\n", reply->cmd, reply->object, reply->index, reply->data);
}

//-----------------------------------------------------------------------

int main()
{
  pthread_t myThread;              //declare a thread
  //declare a genieReplyStruct type structure
  struct genieReplyStruct reply ;
  
  //print some information on the terminal window
  printf("\n\n");
  printf("Office Messaging System basic demo\n");
  printf("==================================\n");
  printf("Program is running. Press Ctrl + C to close.\n");

  //open the Raspberry Pi's onboard serial port, baud rate is 115200
  //make sure that the display module has the same baud rate
  genieSetup("/dev/ttyAMA0", 115200);  

  // Select form0
  genieWriteObj(GENIE_OBJ_FORM, 0, 0);
  
  // Start the clock thread
  (void)pthread_create(&myThread, NULL, clockWork, NULL);
  
  //start the thread for writing to the string
  //(void)pthread_create (&myThread,  NULL, handleString, NULL);

  // Big loop to wait for events to happen
  for(;;)                         
  {
    while(genieReplyAvail())      //check if a message is available
    {
      genieGetReply(&reply);      //take out a message from the events buffer
      handleGenieEvent(&reply);   //call the event handler to process the message
    }	
    usleep(10000);                //10-millisecond delay.Don't hog the 
  }	                          
  //CPU in case anything else is happening
  return 0;
}

