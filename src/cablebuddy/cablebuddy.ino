
#include <EEPROM.h>

typedef enum pinstate
{
  PINSTATE_UNKNOWN = 0,
  PINSTATE_LOW,
  PINSTATE_HIGH,
  PINSTATE_FLOAT,
  PINSTATE_CONNECTED
}a;


#define lineSize 14



typedef struct line
{
  int pin; //like A0
  pinstate state[lineSize]; //for every pin we will have a 'contact' array to other pins
}b;

#define LED_GREEN A4
#define LED_RED A3
#define PBUTTON A5


static line goodMatrix[lineSize];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(PBUTTON, INPUT);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, HIGH);

  goodMatrix[0].pin = 2;
  goodMatrix[1].pin = 3;
  goodMatrix[2].pin = 4;
  goodMatrix[3].pin = 5;
  goodMatrix[4].pin = 6;
  goodMatrix[5].pin = 7;
  goodMatrix[6].pin = 8;
  goodMatrix[7].pin = 9;
  goodMatrix[8].pin = 10;
  goodMatrix[9].pin = 11;
  goodMatrix[10].pin = 12;
  goodMatrix[11].pin = A2;
  goodMatrix[12].pin = A1;
  goodMatrix[13].pin = A0;

  int addr = 0;
  for (int i=0;i<lineSize;i++) 
  {    
      for (int j=0;j<lineSize;j++) 
      {
        int value = EEPROM.read(addr);
        goodMatrix[i].state[j] = (pinstate)value;
        addr++;
      }
  }
}

static void generateConnectionMatrix(line * lines)
{
  for (unsigned int i = 0; i < lineSize; i++) //foreach every pin
  {
    line* pin = &lines[i];
    for (unsigned int j = 0; j < lineSize; j++) //set all pin default state - input
    {
      line* lin = &lines[j];
      pinMode(lin->pin, INPUT); //set to input
      digitalWrite(lin->pin, HIGH); //turn on pullup
    }
    pinMode(pin->pin, OUTPUT); //set to output
    
    for (unsigned int j = 0; j < lineSize; j++) 
    {
      line* lin = &lines[j];
      digitalWrite(pin->pin, LOW);
      delay(10); //wait for the parasitic capacitors
      int lowstate = digitalRead(lin->pin);
      digitalWrite(pin->pin, HIGH);
      delay(10); //wait for the parasitic capacitors
      int highstate = digitalRead(lin->pin);
      pinstate result = PINSTATE_UNKNOWN;
      if      (lowstate == LOW &&  highstate == HIGH) result = PINSTATE_CONNECTED;
      else if (lowstate == HIGH && highstate == HIGH) result = PINSTATE_HIGH; //default state
      else if (lowstate == LOW &&  highstate == LOW) result = PINSTATE_LOW;
      pin->state[j] = result;
    }
  }
}

static unsigned int checkMatrices(line * good, line * current)
{
  for (unsigned int i = 0; i < lineSize; i++)
  {
    for (unsigned int j = 0; j < lineSize; j++)
    {
      if (good[i].state[j] != current[i].state[j]) 
        return i+1;
    }
  }
  return 0;
}

static void printMatrix(line * lines)
{
  for (unsigned int i = 0; i < lineSize; i++)
  {
    line lin = lines[i];
    Serial.print("line:");
    Serial.print(lin.pin);
    Serial.print("\tstate:");
    for (unsigned int j = 0; j < lineSize; j++) 
    {
      Serial.print(lin.state[j]);
    }
    
    Serial.println();
  }
}

int result; /* the result of the previous cable check */

void loop() {
  Serial.println("pincheck started");

  /* initialize the matrix of the actual cable that will be measured to check */
  line lines[lineSize];
  for (int i=0;i<lineSize;i++)
  {
    lines[i].pin = goodMatrix[i].pin;
  }

  int ledstate = 0;
  Serial.println("waiting for button");
  
  /* IDLE state */
  while (!digitalRead(PBUTTON)) 
  {
    if (result ==0) /* the cable is OK */
    {
      if (ledstate==0) digitalWrite(LED_GREEN, LOW);
      else digitalWrite(LED_GREEN, HIGH);
      ledstate++;
      if (ledstate>1) ledstate = 0;
    }
    else /* the cable is NOT OK blinking the RED led [n] times. [n] the first wire which with BAD contact */
    {
        if (ledstate<result)
        {
          digitalWrite(LED_RED, LOW);
          delay(100);
          digitalWrite(LED_RED, HIGH);
        }
        ledstate++;
        if ( (ledstate) > ( result + 5)) 
          ledstate = 0;
    }
    delay(100);
  }
  int time = 0;
  digitalWrite(LED_GREEN, LOW); 
  /* check if the user wants to enter 'learn' mode */
  while (digitalRead(PBUTTON))
  {
    delay(100);
    time++;
    if (time>100) /* 10 sec elapse d*/
    {
      /* blinking the LED to indicate 'learn' mode */
      for (int i=0;i<10;i++)
      {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, HIGH);
        delay(100);
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
        delay(100);
      }
      /* turn off LEDs */
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, HIGH);

      /* set up the matrix for the good condition */
      generateConnectionMatrix(goodMatrix);

      /* write to eeprom*/
      int addr = 0;
      for (int i=0;i<lineSize;i++) 
      {    
          for (int j=0;j<lineSize;j++) 
          {
            if (addr>EEPROM.length()) 
            {
              Serial.println("EEPROM full!");
              while (1)
              {
                digitalWrite(LED_GREEN, HIGH);
                digitalWrite(LED_RED, LOW);
              }
            }
            EEPROM.write(addr, (int)goodMatrix[i].state[j]);
            addr++;
          }
      }
      
      /*blinking the LED to indicate 'learn' mode finished*/
      for (int i=0;i<10;i++)
      {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, LOW);
        delay(100);
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, HIGH);
        delay(100);
      }
      break;
    }
  }

  /* turn off LEDs */
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, HIGH);

  /* set up the matrix for the current cable */
  generateConnectionMatrix(lines);
  
  /* turn on LEDs */
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  /* for debug purposes*/
  //Serial.println("good: ");
  //printMatrix(goodMatrix);
  //Serial.println("actual: ");
  //printMatrix(lines);

  /* compare the matrices */
  result = checkMatrices(goodMatrix,lines);
  Serial.print("result: ");
  Serial.println(result);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, HIGH);
  
}

