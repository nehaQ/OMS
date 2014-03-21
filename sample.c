//------------------------------------------------------------------------------
// Source code using the ViSi-Genie-RaspPi library
// 4D Systems - Office Messaging System
// Allows users to leave/ select a message from to leave for
// the owner of the door. These messages maybe accessed later
// by the owner. The basic setup of the screen is a keyboard 
// with a string object which displays the message typed 
// through the keyboard. 
// Form 0 - Leave a message/Read messages screen for the user
//	    Leddigit7, Leddigit8, Sounds0, Winbutton0, Winbutton1, Winbutton16,
//	    Winbutton32
// Form 1 - Set a message/Pick a message screen for the user
//	    Leddigits9, Leddigits10, Winbutton2, WInbutton3, Winbutton31
// Form 2 - Pick a message screen. Uses a slider to choose the message 
//	    user wants to display
//	    4Dbutton2-5, Leddigits11, Leddigits12, StaticText0-4, Winbutton13, 
//	    Winbutton17
// Form 3 - Screen with a keyboard for the user to type out the message
//	    Keyboard0, Strings0, Winbutton4, Winbutton18
// Form 4 - Screen to set countdown timer. Uses a slider and LED digits
//	    Leddigits0, Leddigits19, Leddigits20, Slider1, StaticText6, 
//	    StaticText7, Winbutton5, Winbutton19
// Form 5 - Screen which is displayed to the visitor
//	    Leddigits1, Leddigits2, Leddigits21, Leddigits22, Statictext8
//	    StaticText10, Strings1, Strings2, Strings10, Strings11, Winbutton6
//	    Winbutton7
// Form 6 - Pick a message/Set Message screen for the visitor
//	    Leddigits5, Leddigits6, Winbutton8, Winbutton9, Winbutton15
// Form 7 - Keyboard for the visitor
//	    Keyboard1, Strings3, Winbutton10, Winbutton25
// Form 8 - Preset messages for visitor with slider to choose
//	    4Dbutton0, 4Dbutton1, Leddigits23, Leddigits24, StaticText11-13, 
// 	    Winbutton26
// Form 9 - Acknowledgement screen for the visitor
//	    Image0, StaticText14
// Form 10- PIN entered access for the user
//	    Keyboard2, StaticText15, Strings4, Winbutton11, Winbutton29
// Form 11- Read your messages screen for the user
//	    Leddigits13, Leddigits14, StaticText16, Strings5, Winbutton12, 
//      Winbutton30
// Form 12- User chooses to set timer or set alarm
//	    Leddigits15, Leddigits16, Winbutton20-22
// Form 13- User sets the alarm
// 	    Keyboard3, Leddigits4, Leddigits17, StaticText7, Strings7, Winbutton23-24
// Form 14- User changes PIN form
//	    Keyboard4, Leddigits18, Leddigits25, Statictext18, Statictext20, 
//	    Strings6, Strings8, Winbutton27
// Form 15- PIN change acknowledgement form
// 	    Statictext21, Strings9
// Form 16- Visitor keyboard with name prompt
//      Keyboard5, Strings12, Winbutton33, Winbutton34

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
//#include "data_structures.h"
#include "db.c"	
#define GENIE_OBJ_SOUND    22
#define GENIE_OBJ_4DBUTTON 30
#define CLOCK 2
#define TIMER 3
#define HOUR  1
#define MINUTE 60

char buf [500]; // Buffer to hold string to print to screen
char user_message [500]; // The message that the user leaves
char msgs_for_user[500]; // Store the users messages
char password[] = "2957\0"; // Store the users password
int count = 0; // Count the number of characters being printed
int start_time = 1; //Starting time value
int wrongPIN = 0; // Count the number of tries with the PIN
int i_was_to_be_back = 0; // Conditional message when timer finishes
int msg_no = 0; // Keep count of the number of msgs left for the user
int hour = 0; // Builds the hour entered by the user on form13
int hello = 0; // The index of the msg that the user picks from the list
int set = 0; // Track whether Timer has been set or Clock
char* preset_msg[] = {"", "Gone for lunch", 
                          "In a meeting", 
                          "Be back soon", 
                          "Out for the day"};
pthread_t timer_; // Thread for running the countdown timer
int time_unit = 0; // Keep track of whether countdown will be in minutes/hours

//-----------------------------------------------------------------------------
// Go to form5 (visitor screen) 
// Function to render the visitor screen.
void updateForm5()
{
  genieWriteObj(GENIE_OBJ_FORM, 5, 0);
  // Render the message that user left
  genieWriteStr(1, user_message); 
  // If timer has finished, print this message
  if(i_was_to_be_back)
    genieWriteStr(2, "I was supposed to be back:");
  // Depending which mode was chosen, label timer accordingly
  if(set == TIMER)
  {
    genieWriteStr(10, "Minutes");
    genieWriteStr(11, "Seconds");
  }
  if(set == CLOCK)
  {
    genieWriteStr(10, "Hours");
    genieWriteStr(11, "Minutes");
  }
}

//-----------------------------------------------------------------------------
// Code follows from basicDemo.c - from the 4Dsystems libraries
// Runs the clock on all forms
static void *clockWork(void * data)
{
  struct sched_param sched;
  int pri = 10;

  time_t tt;
  struct tm timeData;
  
  // Set to real time priority 
  memset(&sched, 0,  sizeof(sched));
  if(pri>sched_get_priority_max (SCHED_RR)) 
    pri = sched_get_priority_max(SCHED_RR);
  
  sched.sched_priority = pri;
  sched_setscheduler (0, SCHED_RR, &sched);

  sleep(1);
  tt = time(NULL);
    (void) localtime_r(&tt, &timeData);
  
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
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 19, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 21, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 23, timeData.tm_min);
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 25, timeData.tm_min);
  }
  return (void *)NULL;
}

//-----------------------------------------------------------------------------
// Runs the countdown timer on the visitor screen
static void *timerHA(void * data)
{
  // Convert into appt unit of time hour/seconds
  int startSec = start_time*time_unit;
   
  for(;;)
  { 
    // Countdown timer
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 2, abs(startSec)/60); //minutes/hour
    genieWriteObj(GENIE_OBJ_LED_DIGITS, 1, abs(startSec)%60); //seconds/mins
    if(startSec == 0)
    	{i_was_to_be_back = 1; updateForm5();}
    startSec--;
    usleep(1000000*(60/time_unit)); // countdown every 1 second
  }

  return (void *)NULL;
}

//-----------------------------------------------------------------------------
// Update the string currently being rendered with keypress from keyboard
void updateDisplay(int string)
{
  genieWriteStr(string, buf); 
}

//-----------------------------------------------------------------------------
// Checks keypress for Backspace and then renders it to the respective string
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

//-----------------------------------------------------------------------------
// Convert 24 hours to minutes
int convertHours(int data)
{
  int hour = data/100;
  int min = hour*60;
  int more_min = data - hour*100;
  return (min + more_min);
}

//-----------------------------------------------------------------------------
//This is the event handler. Events received from display are processed here.
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
        // Go to form16 (Visitor keyboard with name prompt)
        genieWriteObj(GENIE_OBJ_FORM, 16, 0);
        genieWriteStr(12, "Please leave your name");
      }
      if(reply->index == 1)
      {
        // Visitor picked second msg
        msg_no++;
        sprintf(msgs_for_user, "%s\n%d. Lets play golf!", msgs_for_user, msg_no);
        genieWriteObj(GENIE_OBJ_FORM, 9, 0);
        // Go to form16 (Visitor keyboard with name prompt)
        genieWriteObj(GENIE_OBJ_FORM, 16, 0);
        genieWriteStr(12, "Please leave your name");
      }
      if(reply->index == 2)
      {
      	// User picked hello=1
      	hello = 1;
      	sprintf(user_message, "%s", preset_msg[hello]);
      	// Go to form12 to let user pick clock/timer
      	genieWriteObj(GENIE_OBJ_FORM, 12, 0);
      }
      if(reply->index == 3)
      {
        hello = 2;
        sprintf(user_message, "%s", preset_msg[hello]);
      	genieWriteObj(GENIE_OBJ_FORM, 12, 0);       
      }
      if(reply->index == 4)
      {
        hello = 3;
        sprintf(user_message, "%s", preset_msg[hello]);
       	genieWriteObj(GENIE_OBJ_FORM, 12, 0);        
      }
      if(reply->index == 5)
      {
        hello = 4;
        sprintf(user_message, "%s", preset_msg[hello]);
      	genieWriteObj(GENIE_OBJ_FORM, 12, 0);        
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
      	// The set the clock form. ADD: Needs validation
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
	  count++;
	  updateDisplay(7);
  	}
      }
      if(reply->index == 4)
      {
	//write to the data passed through the keyboard
	keyboard(reply->data, 8);
      }
      if(reply->index == 5)
      {
        keyboard(reply->data, 12);
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
	{
	  /*db_connect();
  	  char*** msg = db_read_user();
	  int i;
	  for (i=0; i<2; i++)
	    sprintf(msgs_for_user,"%s\n%s. %s [From %s]", msgs_for_user, msg[i][0], msg[i][1], msg[i][2]);
 	  db_disconnect();
 	  printf("%s", msgs_for_user);*/
 	  genieWriteStr(5, msgs_for_user);
 	}
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
      }
      if(reply->index == 4)
      {
      	sprintf(user_message, "%s", buf);
        // Clear the buffer
        memset(&buf[0], 0, sizeof(buf));
        count = 0;
        
      	// Go to form12 (time screen for user)
	genieWriteObj(GENIE_OBJ_FORM, 12, 0);
	
      }
      if(reply->index == 5)
      {
	// Go to form5 (Visitor screen)
	time_unit = MINUTE;
	pthread_create(&timer_, NULL, timerHA, &set);
	set = TIMER;
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
      }
      if(reply->index == 10)
      {
        msg_no++;
        sprintf(msgs_for_user, "%s\n%d. %s", msgs_for_user, msg_no, buf);
        // Clear the buffer
        memset(&buf[0], 0, sizeof(buf));
        count = 0;
	// Go to form16 (visitor keyboard with prompt)
	genieWriteObj(GENIE_OBJ_FORM, 16, 0);
	genieWriteStr(12, "Please leave your name");
      }
      if(reply->index == 11)
      {
        if(strcmp(buf, password) == 0) // That is PIN is right
	{
	  // Reset the i_was_to_be_back
	  i_was_to_be_back = 0;
	  pthread_cancel(timer_);
	  // Go to form0 (Back to user home screen)
 	  genieWriteObj(GENIE_OBJ_FORM, 0, 0);
	  set = 0; // Turn off timer/alarm
	  // Clear user msg
	  memset(&user_message[0], 0, sizeof(user_message));
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
	if(set == TIMER) set = 0;
      }
      if(reply->index == 13)
      {
	// Set user_message
	//sprintf(user_message, "%s", preset_msg[hello]);
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
        time_t tt;
	struct tm timeData;
	tt = time(NULL);
	(void)localtime_r(&tt, &timeData);
        if(hour <2400 && hour%100<=59 && hour>(timeData.tm_hour*100+timeData.tm_min))
	{
    // The difference in times in minutes
    start_time = convertHours(hour) - convertHours((timeData.tm_hour*100+timeData.tm_min)); 
	  if(start_time >= 60)
	  {
	    // Gone for more than an hour
	    time_unit = HOUR;
	  }
	  else
	  {  
	    time_unit = MINUTE;
	  }
          // Go to form5 (Visitor screen)
	  set = CLOCK;
	  // Append return time to user_message
	  sprintf(user_message, "%s\nI'll be back at %d:%02d", user_message,hour/100, hour%100);
	  pthread_create(&timer_, NULL, timerHA, &set);
	  memset(&buf[0], 0, sizeof(buf));
	  count = 0;
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
      if(reply->index == 31)
      {
        // Web message
      }
      if(reply->index == 33)
      {
        sprintf(msgs_for_user, "%s. [From %s]", msgs_for_user, buf);
        // Clear the buffer
	memset(&buf[0], 0, sizeof(buf));
	count = 0;
        // Go to form9
	genieWriteObj(GENIE_OBJ_FORM, 9, 0);
	usleep(2000000);
	// Return to vistor screen
	updateForm5();
        
      }
      if(reply->index == 34)
      {
        // Clear the buffer
	memset(&buf[0], 0, sizeof(buf));
	count = 0;
      }
    }// End-if Winbutton
    
    if(reply->object == GENIE_OBJ_SLIDER)
    {
      // Slider for set timer screen
      if(reply->index == 1)
      {
        // Store the time to display later
        start_time = reply->data;
        // Write to the LED digits the value of slider
        genieWriteObj(GENIE_OBJ_LED_DIGITS, 0x00, start_time);
                
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

//-----------------------------------------------------------------------------

int main()
{
  pthread_t myThread;              //declare a thread
  //pthread_t timer_;
  //declare a genieReplyStruct type structure
  struct genieReplyStruct reply ;
  
  //print some information on the terminal window
  printf("\n\n");
  printf("Office Messaging System basic demo\n");
  printf("==================================\n");
  printf("Program is running. Press Ctrl + C to close.\n");
  
  version();
  db_connect();
  char*** msg = db_read_user();
  int i;
  for (i=0; i<2; i++)
    printf("\n%s", msg[i][0]);
  db_disconnect();
  //db_read_visitor();;
  //db_disconnect();  
  //open the Raspberry Pi's onboard serial port, baud rate is 115200
  //make sure that the display module has the same baud rate
  genieSetup("/dev/ttyAMA0", 115200);  

  // Select form0    
  genieWriteObj(GENIE_OBJ_FORM, 0, 0); 
  
  
  // Start the clock  thread
  int status = pthread_create(&myThread, NULL, clockWork, NULL);
  if(status < 0) 
  {
    perror("pthread_t_create failed");
    exit(1); 
  }  
  
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