
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
  pinstate state[lineSize];
}b;

#define PLED A2
#define PBUTTON A4

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PLED, OUTPUT);
  digitalWrite(PLED, LOW);

  pinMode(PBUTTON, INPUT);
  digitalWrite(PBUTTON, HIGH); //turn on pullup
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
static line goodMatrix[lineSize];

void loop() {
  Serial.println("pincheck started");
  line lines[lineSize];
  lines[0].pin = 2;
  lines[1].pin = 3;
  lines[2].pin = 4;
  lines[3].pin = 5;
  lines[4].pin = 6;
  lines[5].pin = 7;
  lines[6].pin = 8;
  lines[7].pin = 9;
  lines[8].pin = 10;
  lines[9].pin = 11;
  lines[10].pin = 12;
  lines[11].pin = A2;
  lines[12].pin = A1;
  lines[13].pin = A0;

  if (!digitalRead(PBUTTON))
  {
    digitalWrite(PLED, HIGH);
    while (!digitalRead(PBUTTON)); //wait for release

    while (digitalRead(PBUTTON))
    {
      digitalWrite(PLED, !digitalRead(PLED));
      delay(100);
    }
    digitalWrite(PLED, HIGH);
    generateConnectionMatrix(goodMatrix);
    digitalWrite(PLED, LOW);
  }
  
  generateConnectionMatrix(lines);
  printMatrix(lines);

  delay(4000);  
}

