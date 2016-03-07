// README: The following code is based on the assumption that:
//         Pin 1 of LED matrix --> registers[0]
//         Pin 2 of LED matrix --> registers[1]
//         ... and so on.

// shift register info
const int NUM_SHIFT_REGISTERS = 2;
const int NUM_REGISTER_PINS = NUM_SHIFT_REGISTERS * 8;
bool registers[NUM_REGISTER_PINS];

// arduino pin assignment info
const int SER_PIN = 8;    // pin 14 on the 75HC595
const int RCLK_PIN = 9;   // pin 12 on the 75HC595
const int SRCLK_PIN = 10; // pin 11 on the 75HC595

// led matrix info
int col[] = {1, 2, 3, 4, 5, 6, 7, 8};
int row[] = {9, 10, 11, 12, 13, 14, 15, 16};
bool LED_MATRIX_CONFIG = 0; // 0 if rows are cathodes and columns are anodes
                            // 1 if rows are anodes and columns are cathodes
int ON_ROW, ON_COL, OFF_ROW, OFF_COL; // ON-OFF states depending on config, see set_states()

void set_states(){
  if (LED_MATRIX_CONFIG) {
    ON_ROW = 1; ON_COL = 0; // rows must be HIGH and columns LOW to be ON
    OFF_ROW = 0; OFF_COL = 1; // rows must be LOW and columns HIGH to be OFF
  }
  else {
    ON_ROW = 0; ON_COL = 1; // rows must be LOW and columns HIGH to be ON
    OFF_ROW = 1; OFF_COL = 0; // rows must be HIGH and columns LOW to be OFF
  }
}

void setup(){
  set_states();
  
  pinMode(SER_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);
  
  // reset all register pins
  turn_all_off();
  
  // Serial.begin(9600);
}

int shape[8][8] = {
  {0,0,0,0,0,0,0,1},
  {0,0,0,0,0,0,1,0},
  {0,0,0,0,0,1,0,0},
  {0,0,0,0,1,0,0,0},
  {0,0,0,1,0,0,0,0},
  {0,0,1,0,0,0,0,0},
  {0,1,0,0,0,0,0,0},
  {1,0,0,0,0,0,0,0}
};

void loop(){
  for (int r = 1; r <= 8; r++) {
    for (int c = 1; c <= 8; c++) {
      // Serial.print(shape[r][c]);
      if (shape[r-1][c-1] == 1){
        single_dot_on(r, c);
        write_registers();
        // delay(250);      
        single_dot_off(r, c);
        write_registers();
      }
      else{
        // delay(250);
      }
    }
    // Serial.print("\n");
  }
}             

// set and display registers
// only call AFTER all values are set how you would like (slow otherwise)
void write_registers(){
  digitalWrite(RCLK_PIN, LOW);

  // have to start backwards because of 'push' nature!
  for(int i = NUM_REGISTER_PINS - 1; i >=  0; i--){
    digitalWrite(SRCLK_PIN, LOW);

    int val = registers[i];
    digitalWrite(SER_PIN, val);

    digitalWrite(SRCLK_PIN, HIGH);
  }

  digitalWrite(RCLK_PIN, HIGH);
}

// set register corresponding to rth row
void set_row_register(int r, int value){
  int row_index = row[r - 1];
  int register_index = row_index - 1;
  registers[register_index] = value;
}

// set register corresponding to cth col
void set_col_register(int c, int value){
  int col_index = col[c - 1];
  int register_index = col_index - 1;
  registers[register_index] = value;
}

// set register to light up single dot at (c, r) co-ordinate
void single_dot_on(int r, int c){
  set_row_register(r, ON_ROW);
  set_col_register(c, ON_COL);
}

// set register to light off single dot at (c, r) co-ordinate
void single_dot_off(int r, int c){
  set_row_register(r, OFF_ROW);
  set_col_register(c, OFF_COL);
}

// turn all LEDs off
void turn_all_off(){
  for (int r = 1; r <= 8; r++){
    for (int c = 1; c <= 8; c++) {
      // for rth row and cth column
      single_dot_off(r, c);
    }
  }
  write_registers();
}

// turn all LEDs on
void turn_all_on(){
  for (int r = 1; r <= 8; r++){
    for (int c = 1; c <= 8; c++) {
      // for rth row and cth column
      single_dot_on(r, c);
    }
  }
  write_registers();
}

// push and update, one at a time
void push_register(int value){
  digitalWrite(RCLK_PIN, LOW);
  digitalWrite(SRCLK_PIN, LOW);
  
  digitalWrite(SER_PIN, value);
  
  digitalWrite(SRCLK_PIN, HIGH);
  digitalWrite(RCLK_PIN, HIGH);
}

// light up entire nth row
void whole_row(int n){

}

// light up entire nth col
void whole_col(int n){

}

void debug(){
  for (int r = 1; r <= 8; r++) {
    for (int c = 1; c <= 8; c++) {
      // for rth row and cth column
      single_dot_on(r, c);
      write_registers();
      delay(100);
      single_dot_off(r, c);
      write_registers();
      delay(100);
    }
  }
}
