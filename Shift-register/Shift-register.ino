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
int col[] = {8, 7, 6, 5, 4, 3, 2, 1};
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

int DIAG[][8] = {
  {0,0,0,0,0,0,0,1},
  {0,0,0,0,0,0,1,0},
  {0,0,0,0,0,1,0,0},
  {0,0,0,0,1,0,0,0},
  {0,0,0,1,0,0,0,0},
  {0,0,1,0,0,0,0,0},
  {0,1,0,0,0,0,0,0},
  {1,0,0,0,0,0,0,0}
};

int DIAG_SQUARE[][8] = {
  {0,0,0,1,1,0,0,0},
  {0,0,1,1,1,1,0,0},
  {0,1,1,1,1,1,1,0},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {0,1,1,1,1,1,1,0},
  {0,0,1,1,1,1,0,0},
  {0,0,0,1,1,0,0,0}
};

int LEFT[][8] = {
  {0,0,0,1,0,0,0,0},
  {0,0,1,1,0,0,0,0},
  {0,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {0,1,1,1,1,1,1,1},
  {0,0,1,1,0,0,0,0},
  {0,0,0,1,0,0,0,0}
};

int BLANK[][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};

void loop(){
  scroll_shape(LEFT);
  // TODO: make draw_shape switch on all in a row at same time
  // TODO: start with moving dot and accelerometer
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
  write_registers();
}

// set register to light off single dot at (c, r) co-ordinate
void single_dot_off(int r, int c){
  set_row_register(r, OFF_ROW);
  set_col_register(c, OFF_COL);
  write_registers();
}

// draw 8x8 shape array
void draw_shape(int shape[][8]){
  for (int r = 1; r <= 8; r++) {
    for (int c = 1; c <= 8; c++) {
      if (shape[r-1][c-1] == 1){
        single_dot_on(r, c);
        single_dot_off(r, c);
      }
    }
  }
}

// scroll shape from right to left
void scroll_shape(int shape[][8]){
  int placeholder [][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
  };
  // outside loop for scroll
  for (int i = 0; i < 16; i++){
    unsigned long start_time = millis();
    for (int c = 0; c < 8; c++){
      for (int r = 0; r < 8; r++){
        if (i - c < 0 || i - c >= 8){
          placeholder[r][7 - c] = 0;
        }
        else {
          placeholder[r][7 - c] = shape[r][i - c];
        }
      }
    }
    while (millis() - start_time < 100){
      draw_shape(placeholder);
    }
  }
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

// light up entire nth row
void whole_row(int n){
  for (int c = 1; c <= 8; c++){
    single_dot_on(n, c);
  }
  write_registers();
}

// light up entire nth col
void whole_col(int n){
  for (int r = 1; r <= 8; r++){
    single_dot_on(r, n);
  }
  write_registers();
}

// light up every led individually
void debug_individual(){
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

// push and update shift register, one at a time
void debug_push_register(int value){
  digitalWrite(RCLK_PIN, LOW);
  digitalWrite(SRCLK_PIN, LOW);
  
  digitalWrite(SER_PIN, value);
  
  digitalWrite(SRCLK_PIN, HIGH);
  digitalWrite(RCLK_PIN, HIGH);
}
