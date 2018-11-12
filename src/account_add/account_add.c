/****************************************************************/
/*  Author: Sodaboy                       */
/*  Date: 07/22/2008                      */
/*  accountadd.c :  Adds an account to the Tethealla PSO    */
/*      server...                     */
/*                                */
/*  History:                          */
/*    07/22/2008  TC  First version...            */
/****************************************************************/

#include  <stdint.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <time.h>
#include  <openssl/md5.h>

#include  <mysql/mysql.h>

/* Computes the message digest for string inString.
   Prints out message digest, a space, the string (in quotes) and a
   carriage return.
 */
void MDString (inString, outString)
const unsigned char *inString;
unsigned char *outString;
{
  MD5(inString, strlen((const char*)inString), outString);
}

void UpdateDataFile ( const char* filename, uint32_t count, void* data, uint32_t record_size, int32_t new_record )
{
  FILE* fp;
  uint32_t fs;

  fp = fopen (filename, "r+b");
  if (fp)
  {
    fseek (fp, 0, SEEK_END);
    fs = ftell (fp);
    if ((count * record_size) <= fs)
    {
      fseek (fp, count * record_size, SEEK_SET);
      fwrite (data, 1, record_size, fp);
    }
    else
      printf ("Could not seek to record for updating in %s\n", filename);
    fclose (fp);
  }
  else
  {
    fp = fopen (filename, "wb");
    if (fp)
    {
      fwrite (data, 1, record_size, fp); // Has to be the first record...
      fclose (fp);
    }
    else
      printf ("Could not open %s for writing!\n", filename);
  }
}

void LoadDataFile ( const char* filename, unsigned* count, void** data, uint32_t record_size )
{
  FILE* fp;
  uint32_t ch;

  printf ("Loading \"%s\" ... ", filename);
  fp = fopen (filename, "rb");
  if (fp)
  {
    fseek (fp, 0, SEEK_END);
    *count = ftell (fp) / record_size;
    fseek (fp, 0, SEEK_SET);
    for (ch=0;ch<*count;ch++)
    {
      data[ch] = malloc (record_size);
      if (!data[ch])
      {
        printf ("Out of memory!\nHit [ENTER]");
        exit (1);
      }
      if(!fread (data[ch], 1, record_size, fp))
      {
        printf("Failed to read...\n");
        exit(1);
      }
    }
    fclose (fp);
  }
  printf ("done!\n");
}


/********************************************************
**
**    main  :-
**
********************************************************/

int
main( int32_t argc, char * argv[] )
{
  char inputstr[255] = {0};
  char username[17];
  char password[34];
  char password_check[17];
  char md5password[34] = {0};
  char email[255];
  char email_check[255];
  uint8_t ch;
  time_t regtime;
  uint32_t reg_seconds;
  uint8_t max_fields;

  MYSQL * myData;
  char myQuery[511] = {0};
  MYSQL_ROW myRow ;
  MYSQL_RES * myResult;
  int32_t num_rows, pw_ok, pw_same;
  uint32_t guildcard_number;

  char mySQL_Host[255] = {0};
  char mySQL_Username[255] = {0};
  char mySQL_Password[255] = {0};
  char mySQL_Database[255] = {0};
  uint32_t mySQL_Port;
  int32_t config_index = 0;
  char config_data[255];

  uint8_t MDBuffer[17] = {0};

  FILE* fp;

  if ( ( fp = fopen ("tethealla.ini", "r" ) ) == NULL )
  {
    printf ("The configuration file tethealla.ini appears to be missing.\n");
    return 1;
  }
  else
  {
    while (fgets (config_data, 255, fp) != NULL)
    {
      if (config_data[0] != 0x23)
      {
        if (config_index < 0x04)
        {
          ch = strlen (&config_data[0]);
          if (config_data[ch-1] == 0x0A)
            config_data[ch--]  = 0x00;
          config_data[ch] = 0;
        }
        switch (config_index)
        {
        case 0x00:
          // MySQL Host
          memcpy (&mySQL_Host[0], &config_data[0], ch+1);
          break;
        case 0x01:
          // MySQL Username
          memcpy (&mySQL_Username[0], &config_data[0], ch+1);
          break;
        case 0x02:
          // MySQL Password
          memcpy (&mySQL_Password[0], &config_data[0], ch+1);
          break;
        case 0x03:
          // MySQL Database
          memcpy (&mySQL_Database[0], &config_data[0], ch+1);
          break;
        case 0x04:
          // MySQL Port
          mySQL_Port = atoi (&config_data[0]);
          break;
        default:
          break;
        }
        config_index++;
      }
    }
    fclose (fp);
  }

  if (config_index < 5)
  {
    printf ("tethealla.ini seems to be corrupted.\n");
    return 1;
  }

  if ( (myData = mysql_init((MYSQL*) 0)) &&
    mysql_real_connect( myData, mySQL_Host, mySQL_Username, mySQL_Password, NULL, mySQL_Port,
    NULL, 0 ) )
  {
    if ( mysql_select_db( myData, &mySQL_Database[0] ) < 0 ) {
      printf( "Can't select the %s database !\n", mySQL_Database ) ;
      mysql_close( myData ) ;
      return 2 ;
    }
  }
  else {
    printf( "Can't connect to the mysql server (%s) on port %d !\nmysql_error = %s\n",
      mySQL_Host, mySQL_Port, mysql_error(myData) ) ;

    mysql_close( myData ) ;
    return 1 ;
  }

  printf ("Tethealla Server Account Addition\n");
  printf ("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
  pw_ok = 0;
  while (!pw_ok)
  {
    printf ("New account's username: ");
    scanf ("%s", inputstr );
    if (strlen(inputstr) < 17)
    {
      sprintf (myQuery, "SELECT * from account_data WHERE username='%s'", inputstr );
      // Check to see if that account already exists.
      //printf ("Executing MySQL query: %s\n", myQuery );
      if ( ! mysql_query ( myData, myQuery ) )
      {
        myResult = mysql_store_result ( myData );
        num_rows = (int) mysql_num_rows ( myResult );
        if (num_rows)
        {
          printf ("There is already an account by that name on the server.\n");
          myRow = mysql_fetch_row ( myResult );
          max_fields = mysql_num_fields ( myResult );
          printf ("Row data:\n");
          printf ("-=-=-=-=-\n");
          for (ch=0;ch<max_fields;ch++)
            printf ("column %u = %s\n", ch, myRow[ch] );
        }
        else
          pw_ok = 1;
        mysql_free_result ( myResult );
      }
      else
      {
        printf ("Couldn't query the MySQL server.\n");
        return 1;
      }
    }
    else
      printf ("Desired account name length should be 16 characters or less.\n");
  }
  memcpy (&username[0], &inputstr[0], strlen (inputstr)+1);
  // Gunna use this to salt it up
  regtime = time(NULL);
  pw_ok = 0;
  while (!pw_ok)
  {
    printf ("New account's password: ");
    scanf ("%s", inputstr );
    if ( ( strlen (inputstr ) < 17 ) || ( strlen (inputstr) < 8 ) )
    {
      memcpy (&password[0], &inputstr[0], 17 );
      printf ("Verify password: ");
      scanf ("%s", inputstr );
      memcpy (&password_check[0], &inputstr[0], 17 );
      pw_same = 1;
      for (ch=0;ch<16;ch++)
      {
        if (password[ch] != password_check[ch])
          pw_same = 0;
      }
      if (pw_same)
        pw_ok = 1;
      else
        printf ("The input passwords did not match.\n");
    }
    else
      printf ("Desired password length should be 16 characters or less.\n");
  }
  pw_ok = 0;
  while (!pw_ok)
  {
    printf ("New account's e-mail address: ");
    scanf ("%s", inputstr );
    memcpy (&email[0], &inputstr[0], strlen (inputstr)+1 );
    // Check to see if the e-mail address has already been registered to an account.
    sprintf (myQuery, "SELECT * from account_data WHERE email='%s'", email );
    //printf ("Executing MySQL query: %s\n", myQuery );
    if ( ! mysql_query ( myData, myQuery ) )
    {
      myResult = mysql_store_result ( myData );
      num_rows = (int) mysql_num_rows ( myResult );
      mysql_free_result ( myResult );
      if (num_rows)
        printf ("That e-mail address has already been registered to an account.\n");
    }
    else
    {
      printf ("Couldn't query the MySQL server.\n");
      return 1;
    }
    if (!num_rows)
    {
      printf ("Verify e-mail address: ");
      scanf ("%s", inputstr );
      memcpy (&email_check[0], &inputstr[0], strlen (inputstr)+1 );
      pw_same = 1;
      for (ch=0;ch<strlen(email);ch++)
      {
        if (email[ch] != email_check[ch])
          pw_same = 0;
      }
      if (pw_same)
        pw_ok = 1;
      else
        printf ("The input e-mail addresses did not match.\n");
    }
  }
  // Check to see if any accounts already registered in the database at all.
  sprintf (myQuery, "SELECT * from account_data" );
  //printf ("Executing MySQL query: %s\n", myQuery );
  // Check to see if the e-mail address has already been registered to an account.
  if ( ! mysql_query ( myData, myQuery ) )
  {
    myResult = mysql_store_result ( myData );
    num_rows = (int) mysql_num_rows ( myResult );
    mysql_free_result ( myResult );
    printf ("There are %i accounts already registered on the server.\n", num_rows );
  }
  else
  {
    printf ("Couldn't query the MySQL server.\n");
    return 1;
  }
  reg_seconds = (unsigned) regtime / 3600L;
  ch = strlen (&password[0]);
  sprintf (&config_data[0], "%d", reg_seconds);
  //Throw some salt in the game ;)
  sprintf (&password[ch], "_%s_salt", &config_data[0] );
  //printf ("New password = %s\n", password );
  MDString ((unsigned char *)password, &MDBuffer[0] );
  for (ch=0;ch<16;ch++)
    sprintf (&md5password[ch*2], "%02x", (uint8_t) MDBuffer[ch]);
  md5password[32] = 0;
  if (!num_rows)
  {
    /* First account created is always GM. */
    guildcard_number = 42000001;
    sprintf (myQuery, "INSERT into account_data (username,password,email,regtime,guildcard,isgm,isactive) VALUES ('%s','%s','%s','%u','%u','1','1')", username, md5password, email, reg_seconds, guildcard_number );
  }
  else
  {
    sprintf (myQuery, "INSERT into account_data (username,password,email,regtime,isactive) VALUES ('%s','%s','%s','%u','1')", username, md5password, email, reg_seconds );
  }
  // Insert into table.
  //printf ("Executing MySQL query: %s\n", myQuery );
  if ( ! mysql_query ( myData, myQuery ) )
    printf ("Account successfully added to the database!");
  else
  {
    printf ("Couldn't query the MySQL server.\n");
    return 1;
  }
  mysql_close( myData ) ;
  return 0;
}
