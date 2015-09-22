// README: The following code is based on the assumption that:
//         Pin 1 of LED matrix --> registers[0]
//         Pin 2 of LED matrix --> registers[1]
//         ... and so on.

// shift register info
const int number_of_74hc595s = 2;
const int numOfRegisterPins = number_of_74hc595s * 8;
bool registers[numOfRegisterPins];
const int SER_Pin = 8;    // pin 14 on the 75HC595
const int RCLK_Pin = 9;   // pin 12 on the 75HC595
const int SRCLK_Pin = 10; // pin 11 on the 75HC595

// led matrix info
int row[] = {9, 14, 8, 12, 1, 7, 2, 5};
int col[] = {13, 3, 4, 10, 6, 11, 15, 16};

/*
// button info
const int BUTTON = 11;
int lastButState = 0;
int butState;
long lastDebounceTime = 0;
long debounceDelay = 50;
*/

void setup(){
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  
  // pinMode(BUTTON, INPUT);

  // reset all register pins
  turnAllOff();
  writeRegisters();
  
  // Serial.begin(9600);
}

void loop(){
  setAllRegisters(1);
  writeRegisters();
}             

// turn all LEDs off
void turnAllOff(){
  setAllRowRegisters(0);
  setAllColRegisters(1);
}

// set all register pins to 'value'
void setAllRegisters(int value){
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
     registers[i] = value;
  }
}

// set all row register pins to 'value'
void setAllRowRegisters(int value){
  int registerPin = -1;
  for(int i = 0; i < 8; i++){
    registerPin = row[i] - 1;
    registers[registerPin] = value;
  }
}

// set all col register pins to 'value'
void setAllColRegisters(int value){
  int registerPin = -1;
  for(int i = 0; i < 8; i++){
    registerPin = col[i] - 1;
    registers[registerPin] = value;
  }
}

// set and display registers
// only call AFTER all values are set how you would like (slow otherwise)
void writeRegisters(){
  digitalWrite(RCLK_Pin, LOW);

  // have to start backwards because of 'push' nature!
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
    digitalWrite(SRCLK_Pin, LOW);

    int val = registers[i];
    digitalWrite(SER_Pin, val);

    digitalWrite(SRCLK_Pin, HIGH);
  }

  digitalWrite(RCLK_Pin, HIGH);
}

// push and update, one at a time
void pushRegister(int value){
  digitalWrite(RCLK_Pin, LOW);
  digitalWrite(SRCLK_Pin, LOW);
  
  digitalWrite(SER_Pin, value);
  
  digitalWrite(SRCLK_Pin, HIGH);
  digitalWrite(RCLK_Pin, HIGH);
}

// light up single dot at (c, r) co-ordinate
void singleDot(int r, int c){
  setAllColRegisters(1);
  setAllRowRegisters(0);

  int rowPin = row[r - 1];
  int rowRegisterPin = rowPin - 1;
  registers[rowRegisterPin] = 1;

  int colPin = col[c - 1];
  int colRegisterPin = colPin - 1;
  registers[colRegisterPin] = 0;
}

// light up entire nth row
void wholeRow(int n){
  setAllColRegisters(0);
  int rowPin = row[n - 1];
  int registerPin = rowPin - 1;
  registers[registerPin] = 1;
}

// light up entire nth col
void wholeCol(int n){
  setAllRowRegisters(1);
  int colPin = col[n - 1];
  int registerPin = colPin - 1;
  registers[registerPin] = 0;
}

/*
bool debounceCheck(){
  int reading = digitalRead(BUTTON);
  if (reading != lastButState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime > debounceDelay) && (reading != butState)) {
    butState = reading;
    lastButState = reading;
    return 1;
  }
  return 0;
}

void buttonControl(){
  int reading = digitalRead(BUTTON);
  Serial.println(reading);
  pushRegister(reading);
  delay(100); 
}
*/
