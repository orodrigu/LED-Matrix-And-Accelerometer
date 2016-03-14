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
const int col[] = {8, 7, 6, 5, 4, 3, 2, 1};
const int row[] = {9, 10, 11, 12, 13, 14, 15, 16};
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

// Analog read pins
const int X_PIN = 0;
const int Y_PIN = 1;
const int Z_PIN = 2;

// to hold the caculated values
double x_val;
double y_val;
double z_val;

// Accerlerometer constants
const int TOL = 5; // tolerance for sensitivity neglection
const int FLAT = 335; // x and y readings when level
const int MAX = 70; // max diff in acc reading for any given axis for 90 degree rotation

void setup(){
  set_states();
  
  pinMode(SER_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);
  
  // reset all register pins
  turn_all_off();
  
  Serial.begin(9600);
}

const int DIAG[][8] = {
  {0,0,0,0,0,0,0,1},
  {0,0,0,0,0,0,1,0},
  {0,0,0,0,0,1,0,0},
  {0,0,0,0,1,0,0,0},
  {0,0,0,1,0,0,0,0},
  {0,0,1,0,0,0,0,0},
  {0,1,0,0,0,0,0,0},
  {1,0,0,0,0,0,0,0}
};

const int DIAG_SQUARE[][8] = {
  {0,0,0,1,1,0,0,0},
  {0,0,1,1,1,1,0,0},
  {0,1,1,1,1,1,1,0},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {0,1,1,1,1,1,1,0},
  {0,0,1,1,1,1,0,0},
  {0,0,0,1,1,0,0,0}
};

const int LEFT[][8] = {
  {0,0,0,1,0,0,0,0},
  {0,0,1,1,0,0,0,0},
  {0,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {0,1,1,1,1,1,1,1},
  {0,0,1,1,0,0,0,0},
  {0,0,0,1,0,0,0,0}
};

const int BLANK[][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};

// tilt indicator (line, 1 axis)
// int LIN[][8] = {
//   {1,1,1,1,1,1,1,1},
//   {1,1,1,1,1,1,1,1}
// };

// int IND_LIN_R = 4; 
int dot_r = 4;
int dot_c = 4;

void loop(){
  // unsigned int init_time = millis();
  x_val = analogRead(X_PIN);
  y_val = analogRead(Y_PIN);
  map_dot(x_val, y_val);
  if (dot_r <=7 && dot_r >=1 &&
      dot_c <=7 && dot_c >=1){
    unsigned int init_time = millis();
    while (millis() - init_time < 50){
      draw_dot(dot_r, dot_c);
    }
  }
  // Serial.print(millis() - init_time);
  // z_val = analogRead(Z_PIN);
  // debug_print_acc();
  // Serial.print(dot_r);
  // Serial.print(", ");
  // Serial.print(dot_c);
  // Serial.print("\n");
}

// map x, y acc readinds to DOT position in (r, c)
void map_dot(double x_val, double y_val){
  int delta_r = 0;
  int delta_c = 0;
  if (abs(y_val - FLAT) > TOL){
    // rotation about x
    delta_c = (int) (((FLAT - y_val) / MAX) * 4);
  }
  if (abs(x_val - FLAT) > TOL){
    // rotation about y
    delta_r = (int) (((FLAT - x_val) / MAX) * 4);
  }
  dot_r = 4 + delta_r;
  dot_c = 4 + delta_c;
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
    set_row_register(r, ON_ROW);
    for (int c = 1; c <= 8; c++) {
      if (shape[r-1][c-1] == 1){
        set_col_register(c, ON_COL);
      }
    }
    write_registers();
    set_row_register(r, OFF_ROW);
    for (int c = 1; c <= 8; c++) {
      set_col_register(c, OFF_COL);
    }
    write_registers();
  }
}

// draw LIN with top half in row r
void draw_lin(int r){
  whole_row_on(r);
  whole_row_on(r+1);
  whole_row_off(r);
  whole_row_off(r+1);
}

// draw DOT with top left corner at (r, c)
void draw_dot(int r, int c){
  single_dot_on(r, c);
  single_dot_on(r, c+1);
  single_dot_off(r, c);
  single_dot_off(r, c+1);

  single_dot_on(r+1, c);
  single_dot_on(r+1, c+1);
  single_dot_off(r+1, c);
  single_dot_off(r+1, c+1);
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
void whole_row_on(int n){
  set_row_register(n, ON_ROW);
  for (int c = 1; c <= 8; c++){
    set_col_register(c, ON_COL);
  }
  write_registers();
}

// turn off entire nth row
void whole_row_off(int n){
  set_row_register(n, OFF_ROW);
  for (int c = 1; c <= 8; c++){
    set_col_register(c, OFF_COL);
  }
  write_registers();
}

// light up entire nth col
void whole_col_on(int n){
  set_col_register(n, ON_COL);
  for (int r = 1; r <= 8; r++){
    set_row_register(r, ON_ROW);
  }
  write_registers();
}

// turn off entire nth col
void whole_col_off(int n){
  set_col_register(n, OFF_COL);
  for (int r = 1; r <= 8; r++){
    set_row_register(r, OFF_ROW);
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

void debug_print_acc(){
  Serial.print("x: ");
  Serial.print(x_val);
  Serial.print(" | y: ");
  Serial.print(y_val);
  Serial.print(" | z: ");
  Serial.println(z_val);
}
