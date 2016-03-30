#include <Wire.h>

const int FS = 200, \
          ADDR = 0x68;
const float ACC_SENS = 16384, GYR_SENS = 131, \
            ACC_BIAS = 0.05, GYR_BIAS = 1 - ACC_BIAS;
int16_t xA_raw, yA_raw, zA_raw, \
        xG_raw, yG_raw, zG_raw, \
        T_raw;
float xA, yA, zA, xG, yG, zG, T, \
      pitch_init, roll_init, \
      pitch_acc, roll_acc, \
      pitch_gyr, roll_gyr, \
      pitch_rate_gyr, roll_rate_gyr, \
      pitch_final, roll_final;

void setup(){
  init_imu();
  init_state();
  Serial.begin(9600);
}

void init_imu(){
  Wire.begin();
  Wire.beginTransmission(ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
}

void init_state(){
  read_imu();
  get_acc_angles();
  pitch_init = pitch_acc;
  roll_init = roll_acc;
  // pitch_init = 0;
  // roll_init = 0;
}

void read_imu(){
  Wire.beginTransmission(ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDR, 14, true);

  xA_raw = Wire.read() << 8 | Wire.read();
  yA_raw = Wire.read() << 8 | Wire.read();
  zA_raw = Wire.read() << 8 | Wire.read();
  T_raw  = Wire.read() << 8 | Wire.read();
  xG_raw = Wire.read() << 8 | Wire.read();
  yG_raw = Wire.read() << 8 | Wire.read();
  zG_raw = Wire.read() << 8 | Wire.read();  
}

void get_acc_angles(){
  xA = xA_raw / ACC_SENS;
  yA = yA_raw / ACC_SENS;
  zA = zA_raw / ACC_SENS;

  pitch_acc = asin(xA) * 180 / PI;
  roll_acc = atan(yA / zA) * 180 / PI;
}

void get_gyr_angles(){
  xG = xG_raw / GYR_SENS;
  yG = yG_raw / GYR_SENS;
  zG = zG_raw / GYR_SENS;

  roll_rate_gyr = xG + yG * sin(roll_init) * tan(pitch_init) + zG * cos(roll_init) * tan(pitch_init);
  pitch_rate_gyr = yG * cos(roll_init) - zG * sin(roll_init);

  roll_gyr = roll_init + roll_rate_gyr / FS;
  pitch_gyr = pitch_init + pitch_rate_gyr / FS; 
}

const int COUNT = 100;
unsigned int countr = COUNT;

void loop(){
  //
  // TODO: Fix bug with nan; investigate trig nuances
  // TODO: Optimize away _init, yaw or z, even roll, pitch_rate_gyr conversion
  //

  read_imu();
  get_gyr_angles();
  get_acc_angles();

  pitch_final = ACC_BIAS * pitch_acc + GYR_BIAS * pitch_gyr;
  roll_final = ACC_BIAS * roll_acc + GYR_BIAS * roll_gyr;

  pitch_init = pitch_final;
  roll_init = roll_final;

  if (countr == 0){

    Serial.print("AcX = ");    Serial.print(xA);
    Serial.print(" | AcY = "); Serial.print(yA);
    Serial.print(" | AcZ = "); Serial.print(zA);
    T = T_raw / 340.00 + 36.53;
    Serial.print(" | Tmp = "); Serial.print(T);
    Serial.print(" | GyX = "); Serial.print(xG);
    Serial.print(" | GyY = "); Serial.print(yG);
    Serial.print(" | GyZ = "); Serial.println(zG);
    Serial.print("pitch_acc = "); Serial.print(pitch_acc);
    Serial.print("  | roll_acc = "); Serial.println(roll_acc);
    Serial.print("pitch_gyr = "); Serial.print(pitch_gyr);
    Serial.print("  | roll_gyr = "); Serial.println(roll_gyr);
    Serial.print("pitch_final = "); Serial.print(pitch_final);
    Serial.print("  | roll_final = "); Serial.println(roll_final);
    Serial.println();
    countr = COUNT;

  }
  countr -= 1;

  delay(1000 / FS);
}