
#define QMC5883_ADDR 0x0D


//REG CONTROL

//0x09

#define Mode_Standby    0b00000000
#define Mode_Continuous 0b00000001

#define ODR_10Hz        0b00000000
#define ODR_50Hz        0b00000100
#define ODR_100Hz       0b00001000
#define ODR_200Hz       0b00001100

#define RNG_2G          0b00000000
#define RNG_8G          0b00010000

#define OSR_512         0b00000000
#define OSR_256         0b01000000
#define OSR_128         0b10000000
#define OSR_64          0b11000000

#define mgperdig 4.35f



#include <SoftwareWire.h>
SoftwareWire Wire(3, 4);

int minX = 0;
int maxX = 0;
int minY = 0;
int maxY = 0;
int minZ = 0;
int maxZ = 0;

void setMode(uint16_t mode, uint16_t odr, uint16_t rng, uint16_t osr) {
  WriteReg(0x09, mode | odr | rng | osr);
}

void WriteReg(uint8_t Reg, uint8_t val) {
  Wire.beginTransmission(QMC5883_ADDR); //start talking
  Wire.write(Reg); // Tell the HMC5883 to Continuously Measure
  Wire.write(val); // Set the Register
  Wire.endTransmission();
}


void setup() {
  // put your setup code here, to run once:

  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(5, LOW);
  digitalWrite(6, HIGH);
  Serial.begin(9600);
  Wire.begin();

  WriteReg(0x0B, 0x01);
  //Define Set/Reset period
  setMode(Mode_Continuous, ODR_200Hz, RNG_8G, OSR_512);

}

int rx;
int ry;
int rz;

boolean firstRun = true;

void read(uint16_t* x, uint16_t* y, uint16_t* z) {
  Wire.beginTransmission(QMC5883_ADDR);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();
  Wire.requestFrom(QMC5883_ADDR, 6);
  *x = Wire.read(); //LSB  x
  *x |= Wire.read() << 8; //MSB  x
  *y = Wire.read(); //LSB  z
  *y |= Wire.read() << 8; //MSB z
  *z = Wire.read(); //LSB y
  *z |= Wire.read() << 8; //MSB y
}


void loop() {
  // put your main code here, to run repeatedly:

  int range = 10;
  float x = 0.0;
  float y = 0.0;
  float z = 0.0;
  for (int i = 0; i < range; i++) {
    read(&rx, &ry, &rz);
    x += (float)rx * mgperdig;
    y += (float)ry * mgperdig;
    z += (float)rz * mgperdig;
  }
  x = x / (float)range;
  y = y / (float)range;
  z = z / (float)range;
  delay(1);


  if (firstRun) {
    minX = maxX = x;
    minY = maxY = y;
    minZ = maxZ = z;
    firstRun = false;
  }


  if (x < minX ) minX = x;
  if (x > maxX ) maxX = x;
  if (y < minY ) minY = y;
  if (y > maxY ) maxY = y;
  if (z < minZ ) minZ = z;
  if (z > maxZ ) maxZ = z;

  x = map(x, minX, maxX, -360, 360);
  y = map(y, minY, maxY, -360, 360);
  z = map(z, minZ, maxZ, -360, 360);

//  Serial.print(x);
//  Serial.print(",");
//  Serial.print(y);
//  Serial.print(",");
//  Serial.println(z);



  float heading = atan2((float)y, (float)x);

  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.22;
  heading += declinationAngle;

  // Correct for when signs are reversed.
  if (heading < 0)
    heading += 2 * PI;

  // Check for wrap due to addition of declination.
  if (heading > 2 * PI)
    heading -= 2 * PI;

  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180 / M_PI;

  Serial.print("Heading (degrees): "); Serial.println(headingDegrees);

}
