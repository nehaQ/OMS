// Source code using the ViSi-Genie-RaspPi library
// 4D Systems - Office Messaging System
// Allows users to leave/ select a message from to leave for
// the owner of the door. These messages maybe accessed later
// by the owner. The basic setup of the screen is a keyboard 
// with a string object which displays the message typed 
// through the keyboard. 
// Form 0 - Leave a message/Read messages screen for the user
//	    Winbutton0, Winbutton1, Winbutton16
// Form 1 - Set a message/Pick a message screen for the user
//	    Winbutton2, Winbutton3, Winbutton14
// Form 2 - Pick a message screen. Uses a slider to choose the message 
//	    user wants to display
//	    Slider0, StaticText0 to StaticText5, Leddigits3, Winbutton13
//	    Winbutton17
// Form 3 - Screen with a keyboard for the user to type out the message
//	    Keyboard0, Strings0, Winbutton4, Winbutton18
// Form 4 - Screen to set countdown timer. Uses a slider and LED digits
//	    Slider1, StaticText6, StaticText7, Leddigits0, Winbutton5
//	    Winbutton19
// Form 5 - Screen which is displayed to the visitor
//	    Leddigits1, Leddigits2, Leddigits5, Leddigits 6
//	    StaticText8 to StaticText10, Strings1, Strings2
//	    Strings7, Winbutton6, Winbutton7, Statictext 19
// Form 6 - Pick a message/Set Message screen for the visitor
//	    Winbutton8, Winbutton9, Winbutton15
// Form 7 - Keyboard for the visitor
//	    Keyboard1, Strings3, Winbutton10, Winbutton25
// Form 8 - Preset messages for visitor with slider to choose
//	    Leddigits4, Slider2, StaticText11 to 13, Winbutton26
// 	    4Dbutton0, 4Dbutton1
// Form 9 - Acknowledgement screen for the visitor
//	    Image0, StaticText14
// Form 10- PIN entered access for the user
//	    Keyboard2, StaticText15, Strings4, Winbutton11, Winbutton29
// Form 11- Read your messages screen for the user
//	    StaticText16, Strings5, Winbutton12, Winbutton30
// Form 12- User chooses to set timer or set alarm
//	    Winbutton20 - 22
// Form 13- User sets the alarm
// 	    Winbutton23 - 24, StaticText7, Keyboard3, Leddigits4, Strings6-7
//	    Leddigits17
// Form 14- User changes PIN form
//	    Winbutton27, Winbutton28, Strings8, Leddigits25, Leddigits18
//	    Keyboard4, String 6, Statictext20
// Form 15- PIN change acknowledgement form
// 	    String9, Statictext

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
#include <signal.h>
#include <time.h>

#include <geniePi.h>  //the ViSi-Genie-RaspPi library
#define GENIE_OBJ_4DBUTTON 30
#define CLOCK 2
#define TIMER 3

int display;
int count;
char buf [500]; // Buffer to hold string to print to screen
char user_message [500]; // 
char msgs_for_user[100]; // Store the users messages
char password[] = "2957\0"; // Store the users password
int count = 0; // Count the number of characters being printed
volatile int tortoise = 1; //starting time value
int wrongPIN = 0;
int i_was_to_be_back = 0;
int message_picked = 0;
int msg_no = 0;
int startSec=0;
int *start;
int hour = 0; // Builds the hour entered by the user on form13
int hello = 0;
int set = 0;
char* preset_msg[] = {"", "Gone for lunch", "In a meeting", "Ba back soon", "Out for the day", "Playing golf"};

//-----------------------------------------------------------------------

// Runs the clock on all forms
static void *clockWork(void * data)
{
  struct sched_param sched;
  int pri = 10;

  time_t tt;
  struct tm timeData;
  //set to real time priority 
  
  memset(&sched, 0,  sizeof(sched));
  if(pri>sched_get_priority_max (SCHED_RR)) pri = sched_get_priority_max(SCHED_RR);
  
  sched.sched_priority = pri;
  sched_setscheduler (0, SCHED_RR, &sched);

  sleep(1);
  tt = time(NULL);
    (void) localtime_r(&tt, &timeData);
  
  //start = (tortoise);
  //startSec = *start;
  //printf("%d", startSec);
  //startSec = (startSec)*(60);
  
  for(;;)
  {
    tt = time(NULL);
    while (time(NULL) == tt)  
      usleep(10000);
    
    tt = time(NULL);
    (void) localtime_r(&tt, &timeData);
    // Clock - hour
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 5, timeData.tm_hour);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 7, timeData.tm_hour);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 9, timeData.tm_hour);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 12, timeData.tm_hour);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 14, timeData.tm_hour);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 16, timeData.tm_hour);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 17, timeData.tm_hour);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 18, timeData.tm_hour);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 20, timeData.tm_hour);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 22, timeData.tm_hour);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 24, timeData.tm_hour);
    // Clock - minutes
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 4, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 6, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 8, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 10, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 11, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 13, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 15, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 17, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 19, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 21, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 23, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 25, timeData.tm_min);

    // Timer
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 2, abs(startSec)/60); //minutes
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 1, abs(startSec)%60);
    if(startSec == 0)
    	i_was_to_be_back = 1;
    startSec--;
  }
  return (void *)NULL;
}

//-----------------------------------------------------------------------

static void *timerHA(void * data)
{
  struct sched_param sched;
  int pri = 10;

  //set to real time prioority 
  
  memset(&sched, 0,  sizeof(sched));
  if(pri>sched_get_priority_max (SCHED_RR)) pri = sched_get_priority_max(SCHED_RR);
  
  sched.sched_priority = pri;
  sched_setscheduler (0, SCHED_RR, &sched);

  sleep(1);
  int tortoise = (int)data;
  startSec = tortoise*60;
  
  for(;;)
  {    
    // Timer
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 2, abs(startSec)/60); //minutes
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 1, abs(startSec)%60);
    if(startSec == 0)
    	i_was_to_be_back = 1;
    startSec--;
  }

  return (void *)NULL;
}

//-----------------------------------------------------------------------

//Update the display
void updateDisplay(int string)
{
  genieWriteStr(string, buf); //strings
}

//-----------------------------------------------------------------------

// Go to form5 (visitor screen) 
// Separate functions because strings 1,2 need to be rendered
void updateForm5()
{
  genieWriteObj(GENIE_OBJ_FORM, 5, 0);
  genieWriteStr(1, user_message);
  if(i_was_to_be_back)
    genieWriteStr(2, "I was supposed to be back:");
}
//-----------------------------------------------------------------------

void keyboard(int data, int string)
{
  if(data == 8)
  {
    count--;
    if(count < 0) count = 0;
    buf[count] = '\0';
    updateDisplay(string);
  }
  else
  {
    buf[count] = data;
    count++;
    updateDisplay(string);
  }
}

//-----------------------------------------------------------------------

//This is the event handler. Messages received from the display
//are processed here.
void handleGenieEvent(struct genieReplyStruct * reply)
{
  //check if the cmd byte is a report event
  if(reply->cmd == GENIE_REPORT_EVENT)    
  {
    
    if(reply->object == GENIE_OBJ_4DBUTTON)
    {
      if(reply->index == 0)
      {
        // Visitor picked first msg
        msg_no++;
        sprintf(msgs_for_user, "%s\n%d. I was here.", msgs_for_user, msg_no);
        // Go to form& (Visitor keyboard)
        genieWriteObj(GENIE_OBJ_FORM, 7, 0);
        genieWriteStr(3, "Please leave your name");
      }
      if(reply->index == 1)
      {
        // Visitor picked second msg
        msg_no++;
        sprintf(msgs_for_user, "%s\n%d. Lets play golf!", msgs_for_user, msg_no);
        genieWriteObj(GENIE_OBJ_FORM, 9, 0);
        // Go to form& (Visitor keyboard)
        genieWriteObj(GENIE_OBJ_FORM, 7, 0);
        genieWriteStr(3, "Please leave your name");
      }
    }

    //check if the object byte is that of a keyboard
    if(reply->object == GENIE_OBJ_KEYBOARD) 
    {
      // Check which keyboard it came from
      if(reply->index == 0)		  
      {
        //write to the data passed through the keyboard
	keyboard(reply->data, 0);
      }
      if(reply->index == 1)
      {
        //write to the data passed through the keyboard
	keyboard(reply->data, 3);
      }
      if(reply->index == 2)
      {
	//write to the data passed through the keyboard
	keyboard(reply->data, 4);
      }
      if(reply->index == 3)
      {
      	// The set the clock form. Needs validation
      	if(reply->data == 8)
	{
	  count--;
	  if(count < 0) count = 0;
	  buf[count] = '\0';
	  updateDisplay(7);
	}
	else
	{
	  buf[count] = reply->data;
	  //hour = hour*10 + (int)reply->data;
	  count++;
	  updateDisplay(7);
  	}
      }
      if(reply->index == 4)
      {
	//write to the data passed through the keyboard
	keyboard(reply->data, 8);
      }
    } // End keyboard
    
    if(reply->object == GENIE_OBJ_WINBUTTON)
    {
      if(reply->index == 0)
      {
	// Go to form1 (Pick/set msg for user)
	genieWriteObj(GENIE_OBJ_FORM, 1, 0);
      }
      if(reply->index == 1)
      {
	// Go to form11 (Read your msgs screen)
	genieWriteObj(GENIE_OBJ_FORM, 11, 0);
	if(msg_no == 0)
	  genieWriteStr(5, "No messages at present");
	else
	  genieWriteStr(5, msgs_for_user);
      }
      if(reply->index == 2)
      {
	// Go to form3 (Keybd for user)
	genieWriteObj(GENIE_OBJ_FORM, 3, 0);
      }
      if(reply->index == 3)
      {
	// Go to form2 (Pick msg for user)
	genieWriteObj(GENIE_OBJ_FORM, 2, 0);
	genieWriteObj(GENIE_OBJ_SLIDER, 0, 0);
	genieWriteObj(GENIE_OBJ_LED_DIGITS, 3, 0);
      }
      if(reply->index == 4)
      {
      	sprintf(user_message, "%s", buf);
        // Clear the buffer
        memset(&buf[0], 0, sizeof(buf));
        count = 0;
        
      	// Go to form4 (set timer screen for user)
	genieWriteObj(GENIE_OBJ_FORM, 12, 0);
	
      }
      if(reply->index == 5)
      {
	// Go to form5 (Visitor screen)
	pthread_kill(&myThread, SIGUSR1);
	updateForm5();
      }
      if(reply->index == 6)
      {
	// Go to form10 (the PIN screen)
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
	// Go to form8 (Picks a msg screen for visitor)
	genieWriteObj(GENIE_OBJ_FORM, 8, 0);
	genieWriteObj(GENIE_OBJ_SLIDER, 2, 0);
	genieWriteObj(GENIE_OBJ_LED_DIGITS, 4, 0);
      }
      if(reply->index == 10)
      {
        msg_no++;
        sprintf(msgs_for_user, "%s\n%d%s", msgs_for_user, msg_no, buf);
        // Clear the buffer
        memset(&buf[0], 0, sizeof(buf));
        count = 0;
	// Go to form9
	genieWriteObj(GENIE_OBJ_FORM, 9, 0);
	usleep(2000000);
	// Return to vistor screen
	//genieWriteObj(GENIE_OBJ_FORM, 5, 0);
	updateForm5();
      }
      if(reply->index == 11)
      {
        if(strcmp(buf, password) == 0) // That is PIN is right
	{
	  // Reset the i_was_to_be_back
	  i_was_to_be_back = 0;
	  // Go to form0 (Back to user home screen)
 	  genieWriteObj(GENIE_OBJ_FORM, 0, 0);
	  if(set == TIMER)
	  {
	    //pthread_cancel(timer_);
	    set = 0;
 	  }
	}
	else
	{
	  if(wrongPIN == 2)
	  {
	    genieWriteObj(GENIE_OBJ_FORM, 5, 0);
	    wrongPIN = 0;
	  }
	  else
	  {
	    genieWriteStr(4, "Wrong PIN");
	    usleep(500000);
	    genieWriteStr(4, "");
	    wrongPIN++;
	  }
	}
	// Clear the buffer
	memset(&buf[0], 0, sizeof(buf));
        count = 0;
      }
      if(reply->index == 12)
      {
	// Go to form0 (back to user home screen)
	genieWriteObj(GENIE_OBJ_FORM, 0, 0);
	if(set == TIMER)
	{
	  //pthread_cancel(timer_);
	  set = 0;
	}
      }
      if(reply->index == 13)
      {
	// Set user_message
	sprintf(user_message, "%s", preset_msg[hello]);
      	// Go to form12 (User chooses to set clk/timer)
      	genieWriteObj(GENIE_OBJ_FORM, 12, 0);
      }
      if(reply->index == 14)
      {
        // Go to form0 (user home screen)
        genieWriteObj(GENIE_OBJ_FORM, 0, 0);
      }
      if(reply->index == 15)
      {
        // Go to form5 (Visitor screen)
        updateForm5();
      }
      if(reply->index == 16)
      {
      	// Go to form14 (change the PIN form)
      	genieWriteObj(GENIE_OBJ_FORM, 14, 0);
      	genieWriteStr(6, password);
      }
      if(reply->index == 17)
      {
      	// Go to form1 (User chooses to pick/set msg)
      	genieWriteObj(GENIE_OBJ_FORM, 1, 0);
      }
      if(reply->index == 18)
      {
  	// Clear the buffer
	memset(&buf[0], 0, sizeof(buf));
	count = 0;
      	// Go to form1 (User chooses to pic/set msg)
    	genieWriteObj(GENIE_OBJ_FORM, 1, 0);
      }
      if(reply->index == 19)
      {
        // Go to form12 (User chooses to set clk/timer)
      	genieWriteObj(GENIE_OBJ_FORM, 12, 0);
      }
      if(reply->index == 20)
      {
      	// Go to form4 (Set timer screen)
      	genieWriteObj(GENIE_OBJ_FORM, 4, 0);
      	// Set slider to 0
	genieWriteObj(GENIE_OBJ_SLIDER, 1, 0);
	genieWriteObj(GENIE_OBJ_LED_DIGITS, 0, 0);
      }
      if(reply->index == 21)
      {
        // Go to form13 (Set alarm screen)
        genieWriteObj(GENIE_OBJ_FORM, 13, 0);
        set = CLOCK;
      }
      if(reply->index == 22)
      {
        // Go to form1 (User chooses to set/pick msg)
        genieWriteObj(GENIE_OBJ_FORM, 1, 0); 
      }
      if(reply->index == 23)
      {
        // Go to form12 (User picks alarm/timer)
        genieWriteObj(GENIE_OBJ_FORM, 12, 0);
      }
      if(reply->index == 24)
      {
        hour = atoi(buf);
        if(hour >=1 && hour <=24)
	{
          // Go to form5 (Visitor screen)
	  set = CLOCK;
          updateForm5();
        }
        else
        {
          genieWriteStr(7, "Invalid hour");
          memset(&buf[0], 0, sizeof(buf));
	  count = 0;
	  hour = 0; // Clear the hour
	}
      }
      if(reply->index == 25)
      {
   	// Clear the buffer
	memset(&buf[0], 0, sizeof(buf));
	count = 0;
        // Go to form6 (Visitor pick/set msg)
        genieWriteObj(GENIE_OBJ_FORM, 6, 0);
      }
      if(reply->index == 26)
      {
        // Go to form6 (Visitor pick/set msg)
        genieWriteObj(GENIE_OBJ_FORM, 6, 0);
      }
      if(reply->index == 27)
      {
      	// Change the PIN
      	strcpy(password, buf);
      	// Clear the buffer
	memset(&buf[0], 0, sizeof(buf));
	count = 0;
	// Go to form15 (Ack form)
	genieWriteObj(GENIE_OBJ_FORM, 15, 0);
	genieWriteStr(9, password);
	usleep(1500000);
	// Go to form0
	genieWriteObj(GENIE_OBJ_FORM, 0, 0);
      }
      if(reply->index == 28)
      {
        // Clear the buffer
	memset(&buf[0], 0, sizeof(buf));
	count = 0;
	// Go to form0
	genieWriteObj(GENIE_OBJ_FORM, 0, 0);
      }
      if(reply->index == 29)
      {
      	// Clear the buffer
	memset(&buf[0], 0, sizeof(buf));
	count = 0;
	// Go to form5
	updateForm5();
      }
      if(reply->index == 30)
      {
      	// Clear the user's messages
      	memset(&msgs_for_user[0], 0, sizeof(msgs_for_user));
      	msg_no = 0;
      	genieWriteStr(5, "No messages at present");
      }
    }// End-if Winbutton
    
    if(reply->object == GENIE_OBJ_SLIDER)
    {
      // Slider for user picks a msg form
      if(reply->index == 0)
      {
        // Write to the LED digits the value of slider
        hello = reply->data;
        genieWriteObj(GENIE_OBJ_LED_DIGITS, 0x03, hello);
      }
      // Slider for set timer screen
      if(reply->index == 1)
      {
        // Store the time to display later
        tortoise = reply->data;
        // Write to the LED digits the value of slider
        genieWriteObj(GENIE_OBJ_LED_DIGITS, 0x00, tortoise);
                
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

