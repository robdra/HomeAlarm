#include <Keypad.h>

void alarm();
void triggerSound();
void activate(char c);
void deactivated();
void failedLogin();
void full(int stat);
void shell(int stat);
void login();
int shellSensor();
int fullSensor();

unsigned long prevMillis = 0;
String pin = "";
int attempts = 0;
bool real = false;

int greenLed = 12;
int greenState = LOW;
int yellowLed = 11;
int yellowState = LOW;
int redLed = 10;
int redState = LOW;

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// connect the pins from right to left to pin 2, 3, 4, 5,6,7,8,9
byte rowPins[ROWS] = {5,4,3,2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9,8,7,6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Serial.begin(9600);
  pinMode(greenLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(A5, INPUT);
  pinMode(A4, INPUT);
  pinMode(A3, OUTPUT);
}
  
void loop()
{
  full(0, 0);
}
//Activates both sensors
void full(int statF, int statS)
{
  //If any sensor is triggered it sounds the alarm
  if(statF == 1 || statS == 1)
  {
    triggerSound();
    digitalWrite(greenLed, HIGH);
  }
  else
  {
    digitalWrite(greenLed, HIGH);
    digitalWrite(yellowLed, LOW);

    for(int i = 0; i < 3; i++)
    {
      tone(A3, 1000);
      delay(500);
      noTone(A3);
      delay(500);
    }
  }

  while(true)
  {
    digitalWrite(redLed, LOW);
    //Only checks the shell sensor if it was not triggered at the time of activation
    if(statS == 0)
    {
      int check = shellSensor();

      if(check == 1)
      {
        alarm();
      }
    }
    //Only checks the interior sensor if it was not triggered at the time of activation
    if(statF == 0)
    {
      int check = fullSensor();

      if(check == 1)
      {
        alarm();
      }
    }
    //Checks if someone is pressing a key on the keypad
    login();
  }
}
//Activates only the shell sensors
void shell(int statS)
{
  //If the sensor is triggered at the time of activation the alarm is triggered
  if(statS == 1)
  {
    triggerSound();
  }
  else
  {
    digitalWrite(yellowLed, LOW);
    tone(A3, 1000);
    delay(100);
    noTone(A3);
    delay(100);
    tone(A3, 1000);
    delay(100);
    noTone(A3);
  }

  while(true)
  {
    unsigned long currMillis = millis();
    digitalWrite(redLed, LOW);
    //Protothreading to be able to blink the green led without delaying the system
    if (currMillis - prevMillis >= 1000) 
    {
      prevMillis = currMillis;

      if (greenState == LOW) {
        greenState = HIGH;
      } else {
        greenState = LOW;
      }

      digitalWrite(greenLed, greenState);
    }
    //Only checks the sensor if it was not triggered at the time of activation
    if(statS == 0)
    {
      int check = shellSensor();

      if(check == 1)
      {
        alarm();
      }
    } 
    //Checks if someone is pressing a key on the keypad 
    login();
  }
}
//Checks the input from the shell sensor
int shellSensor()
{
  int val = digitalRead(A5);
  
  if(val == 1)
  {
    //Sends a B to HC
    Serial.print('B');
    return 1;
  }

  return 0;
}
//Checks input from the interior sensor
int fullSensor()
{
  int val = digitalRead(A4);
  
  if(val == 1)
  {
    //Sends a C to HC
    Serial.print('C');
    return 1;
  }

  return 0;
}
//If three unsuccessful attemts were made to deactivate the alarm
//a D is sent to HC and the program enters and endless loop
void failedLogin()
{
  //Alert the authorities
  Serial.print('D');
  while(true)
  {
    tone(A3, 1000);
    digitalWrite(greenLed, HIGH);
    digitalWrite(yellowLed, HIGH);
    digitalWrite(redLed, HIGH);
    delay(100);
    digitalWrite(greenLed, LOW);
    digitalWrite(yellowLed, LOW);
    digitalWrite(redLed, LOW);
    delay(100);
  }
}
//If a key between 0-9 is pressed on the keypad, the number is stored in a pin variable.
//If # is pressed it concatenates an A with the pin and sends it to HC.
//It then reads from HC to see if the pin was in the system or not
void login()
{
  char key = keypad.getKey();
  
  if (key)
  {
    switch(key)
    {
      case '1':
        pin += key;
        break;
      case '2':
        pin += key;
        break;
      case '3':
        pin += key;
        break;
      case '4':
        pin += key;
        break;
      case '5':
        pin += key;
        break;
      case '6':
        pin += key;
        break;
      case '7':
        pin += key;
        break;
      case '8':
        pin += key;
        break;
      case '9':
        pin += key;
        break;
      case '0':
        pin += key;
        break;
      case '#':
        String check = 'A' + pin;
        Serial.print(check);
        String ret = Serial.readString();
        
        if(ret == "1")
        {
          pin.remove(0);
          //Serial.print(pin);
          deactivated();
        }
        else
        {
          pin.remove(0);
          //Serial.print(pin);
          attempts++;

          if(attempts == 3)
          {
            failedLogin();
          }
        }
        break;
      default:
        break;
    }
    
  }
}
//If a successful attempt to deactivate was made the system enters a loop where no sensor are checked 
void deactivated()
{
  tone(A3, 1000);
  delay(100);
  noTone(A3);
  delay(100);
  tone(A3, 1000);
  delay(100);
  noTone(A3);

  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
  digitalWrite(yellowLed, LOW);
  
  pin = "";
  attempts = 0;

  //If someone press an A or B the activation function is started
  while(true)
  {
    char key = keypad.getKey();

    if(key == 'A' || key == 'B')
    {
      activate(key);
    }
  }
}
//Takes an A or B to activate either the full alarm or just the shell.
//Checks in the same was as the deactivation function but concats with
//an E instead of an A to log that it was an activation this time.
//If it was successful the sensors are checked to see if they are triggered
void activate(char c)
{
  while(true)
  {
    char key = keypad.getKey();

    if (key)
    {
      switch(key)
      {
        case '1':
          pin += key;
          break;
        case '2':
          pin += key;
          break;
        case '3':
          pin += key;
          break;
        case '4':
          pin += key;
          break;
        case '5':
          pin += key;
          break;
        case '6':
          pin += key;
          break;
        case '7':
          pin += key;
          break;
        case '8':
          pin += key;
          break;
        case '9':
          pin += key;
          break;
        case '0':
          pin += key;
          break;
        case '#':
          String check = 'E' + pin;
          Serial.print(check);
          String ret = Serial.readString();

          if(ret == "1")
          {
            int fullStat = fullSensor();
            int shellStat = shellSensor();

            if(c == 'A')
            {
              pin = "";
              full(fullStat, shellStat);
            }
            if(c == 'B')
            {
              pin = "";
              shell(shellStat);
            }
          }
          else
          {
            pin = "";
            deactivated();
          }
          break;
        default:
          break;
      }

    }
  }
}
//A sound and flash if a sensor is triggered while activating the alarm
void triggerSound()
{
  for(int i = 0; i < 3; i++)
  {
    digitalWrite(greenLed, LOW);
    digitalWrite(yellowLed, LOW);
    digitalWrite(redLed, HIGH);
    tone(A3, 1000);
    delay(500);
    digitalWrite(greenLed, LOW);
    digitalWrite(yellowLed, LOW);
    digitalWrite(redLed, LOW);
    noTone(A3);
    delay(500);
  }
  digitalWrite(yellowLed, HIGH);
}
//This is the sound and flashes for when a sensor is triggered while the alarm is active
void alarm()
{
  greenState = LOW;
  yellowState = LOW;
  redState = LOW;

  while(true)
  {
    tone(A3, 1000);
    unsigned long currMillis = millis();

    if (currMillis - prevMillis >= 100) 
    {
      prevMillis = currMillis;

      if (greenState == LOW) 
      {
        greenState = HIGH;
        yellowState = HIGH;
        redState = HIGH;
      }
      else
      {
        greenState = LOW;
        yellowState = LOW;
        redState = LOW;
      }
      digitalWrite(greenLed, greenState);
      digitalWrite(yellowLed, yellowState);
      digitalWrite(redLed, redState);
    }
    login();
  }
}