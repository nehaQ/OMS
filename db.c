// Help from http://www.raspberry-projects.com/pi/programming-in-c/mysql/setup-mysql-c-access
// and ttp://www.raspberry-projects.com/pi/programming-in-c/mysql/accessing-the-database
#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include "data_structures.h"

#define DB_NAME 	"mchi9na4"
#define	DB_USERNAME	"mchi9na4"
#define DB_PASSWORD	"physix64ever"

MYSQL *con;

void version()
{
  printf("Version:%s\n", mysql_get_client_info());
}

void db_connect()
{
  // Initialise the object to start the connection
  con = mysql_init(NULL);
  
  if(con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    return;
  }
  
  // Setup connection to database
  if(mysql_real_connect(con, "dbhost.cs.man.ac.uk", DB_USERNAME, DB_PASSWORD, DB_NAME,0, NULL, 0) == NULL)
    fprintf(stderr, "%s\n", mysql_error(con));
  else
    printf("Connection to database was successful\n");
}

void db_disconnect()
{
  mysql_close(con);
  printf("Connection to database was succesfully closed\n");
}

void db_write_user(char* msg, char* unit, int time_)
{
  // Check if connection is still there
  if(con != NULL)
  {
    if(mysql_query(con, "INSERT INTO message (message_enter, time_unit, time_enter) VALUES ('msg', 'unit', 'time_')"))
    {
      fprintf(stderr, "%s\n", mysql_error(con));
      return;
    }
  }
}
    
void db_write_visitor(char* name, char* msg)
{
  // Check if connection is still there
  if(con != NULL)
  {
    if(mysql_query(con, "INSERT INTO visitor (visitor, message_enter) VALUES ('name', 'msg')"))
    {
      fprintf(stderr, "%s\n", mysql_error(con));
      return;
    }
  }
}

char*** db_read_user()
{
  //message_struct *messages;
  char*** messages;
  printf("from reader");
  if(con != NULL)
  {
    if(!mysql_query(con, "SELECT * FROM visitor ORDER BY time_stamp ASC"))
    {
      MYSQL_RES* result = mysql_store_result(con);
      if(result != NULL)
      {
        int rows = mysql_num_rows(result);
        int columns = mysql_num_fields(result);
        messages = (char***)malloc(rows * sizeof(char**));
        int i;
        for(i=0; i<rows; i++)
          messages[i] = (char**)malloc(columns * sizeof(char*));
          
	int index = 0;
        
        // An array of strings
        MYSQL_ROW row;
        while((row = mysql_fetch_row(result)))
        {
          if(columns >= 2)
          {
            char *temp = row[0];
            messages[index][0] = temp;
            messages[index][1] = (char*)row[1];
            messages[index][2] = row[2];            
            index++;
          }
        }
        mysql_free_result(result);
      }
    }
  }
  return messages;
}

void db_read_visitor()
{
  if(con != NULL)
  {
    if(!mysql_query(con, "SELECT * FROM message ORDER BY id DESC LIMIT 1"))
    {
      MYSQL_RES* result = mysql_store_result(con);
      if(result != NULL)
      {
        int rows = mysql_num_rows(result);
        int columns = mysql_num_fields(result);
        
        // An array of strings
        MYSQL_ROW row;
        while((row = mysql_fetch_row(result)))
        {
          if(columns >= 2)
          {
            char* name = row[0];
            char* msg = row[1];
            printf("user msg %s", msg);
          }
        }
        mysql_free_result(result);
      }
    }
  }
}
        
        
