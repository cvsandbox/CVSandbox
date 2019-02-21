/*
    Computer Vision Sandbox, Arduino communication example
    http://www.cvsandbox.com/
    Copyright Andrew Kirillov, 2019

    The Arduino sketch implements very simple communication
    protocol allowing controlling LED (on/off) and querying
    button state over serial port. The sketch is aimed to be
    used with Lua scripting sample demonstrating usage of Serial
    Port plug-in for Computer Vision Sandbox.
*/

// Status LED to blick to show we are alive
const int STATUS_PIN = LED_BUILTIN;
// LED pin to switch ON/OFF
const int LED_PIN    = 3;
// Buttong pin to query state
const int BUTTON_PIN = 4;

// Time to sleep at the end of every loop
const int delayTime = 10;
// Controls frequency of status LED blinking,
// which will be ON/OFF during cycleLen loops
const int cycleLen  = 100;
// Counter for status LED
int       counter   = 0;

// Maximum bytes to read for a command
const int maxRead   = 15;
// Bytes read so far
int       readCount = 0;
// Buffer used to keep currently read command
char      buffer[maxRead + 1];

void setup( )
{
  pinMode( STATUS_PIN, OUTPUT );
  pinMode( LED_PIN, OUTPUT );
  pinMode( BUTTON_PIN, INPUT );

  Serial.begin( 9600 );
}

void loop( )
{
  int  buttonState = digitalRead( BUTTON_PIN );
  char b = 0;

  // blink status LED
  digitalWrite( STATUS_PIN, ( ( counter < cycleLen ) || ( buttonState == HIGH ) ) ? HIGH : LOW );
  counter = ( counter + 1 ) % ( cycleLen * 2 );

  // read command from serial
  while ( Serial.available( ) > 0 )
  {
    b = Serial.read( );

    // end of command
    if ( b == '\n' )
    {
      buffer[readCount] = '\0';
      break;
    }

    // ignore all other space characters in commands
    if ( !isspace( b ) )
    {
      // check if buffer already full - trash if so
      if ( readCount == maxRead )
      {
        readCount = 0;
      }
      
      buffer[readCount] = b;
      readCount++;
    }
  }

  // handle command
  if ( b == '\n' )
  {
    bool failed = false;
    
    if ( strcmp( buffer, "led_on" ) == 0 )
    {
      digitalWrite( LED_PIN, HIGH );
    }
    else if ( strcmp( buffer, "led_off" ) == 0 )
    {
      digitalWrite( LED_PIN, LOW );
    }
    else if ( strcmp( buffer, "btn_state" ) == 0 )
    {
      Serial.print( buttonState );
      Serial.print( ' ' );
    }
    else
    {
      failed = true;
    }

    Serial.print( ( !failed ) ? "OK" : "FAILED" );
    Serial.print( "\n" );
      
    readCount = 0;
  }
  
  delay( delayTime );
}
