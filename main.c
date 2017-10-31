#include<16f1825.h>
#include <string.h>
#include <math.h>
#use delay(INTERNAL=8MHz)
#fuses XT, NOWDT, NOPROTECT, BROWNOUT, PUT, NOLVP
#use rs232(baud=9600, xmit=PIN_C4, rcv=PIN_C5, errors, Parity=N)
#use i2c(Master, sda=PIN_C1, scl=PIN_C0)

#define MPU6050_SLAVE_WRT     0xD0 
#define MPU6050_SLAVE_RD      0xD1 
#define MPU6050_GYRO_CONFIG   0x1B
#define MPU6050_GYRO_ACCEL    0x1C
#define MPU6050_ACCEL_XOUT_H  0x3B
#define MPU6050_ACCEL_XOUT_L  0x3C
#define MPU6050_ACCEL_YOUT_H  0x3D
#define MPU6050_ACCEL_YOUT_L  0x3E
#define MPU6050_ACCEL_ZOUT_H  0x3F
#define MPU6050_ACCEL_ZOUT_L  0x40
#define MPU6050_GYRO_XOUT_H   0x43
#define MPU6050_GYRO_XOUT_L   0x44
#define MPU6050_GYRO_YOUT_H   0x45
#define MPU6050_GYRO_YOUT_L   0x46
#define MPU6050_GYRO_ZOUT_H   0x47
#define MPU6050_GYRO_ZOUT_L   0x48
#define MPU6050_PWR_MGMT_1    0x6B
#define MPU6050_WHO_AM_I      0X75
#define MPU6050_RA_SMPLRT_DIV       0x19
#define MPU6050_RA_CONFIG           0x1A
#define MPU6050_RA_FIFO_EN          0x23
#define MPU6050_OFFSET_GYRO_X 80
#define MPU6050_OFFSET_GYRO_y 110
#define MPU6050_OFFSET_GYRO_Z -170
#define MPU6050_OFFSET_ACCEL_X 1360
#define MPU6050_OFFSET_ACCEL_Y 0
#define MPU6050_OFFSET_ACCEL_Z -984

void Write2Mpu6050(unsigned int8 RegAddr,unsigned int8 data)
{
   i2c_start ();    // Start condition
   i2c_write(MPU6050_SLAVE_WRT);// Device address write
   i2c_write(RegAddr);// Regiter address
   i2c_write(data);// data send
   i2c_stop();     // Stop condition
}

unsigned int8 ReadMpu6050(unsigned int8 RegAddr)
{
   unsigned int8 value;
   i2c_start();
   i2c_write(MPU6050_SLAVE_WRT);
   i2c_write(RegAddr);
   i2c_start();
   i2c_write(MPU6050_SLAVE_RD);// Device address read   
   value = i2c_read(0);   
   i2c_stop();   
   return value;
 }

signed int16 GetdataMpu6050(unsigned int8 RegAddr)
{
 unsigned int8 LSB;
 unsigned int8 MSB;
 MSB=ReadMpu6050(RegAddr);
 LSB=ReadMpu6050(RegAddr+1); 
 return (make16(MSB,LSB));
 }

void InitMpu6050()
{
   Write2Mpu6050(MPU6050_PWR_MGMT_1, 0x01);
   Write2Mpu6050(MPU6050_GYRO_CONFIG, 0x00); //full scale range mode 0 +-250do/s   
   Write2Mpu6050(MPU6050_GYRO_ACCEL, 0x00); //full scale range mode 0 +-2g
   delay_ms(30);
   Write2Mpu6050(MPU6050_PWR_MGMT_1, 0x08);// internal 8MHz, disabled SLEEP mode, disable CYCLE mode   
   //Sets sample rate to 8000/(1+7) = 1000Hz
   Write2Mpu6050(MPU6050_RA_SMPLRT_DIV, 0x07);
   //Disable FSync, 256Hz DLPF
   Write2Mpu6050(MPU6050_RA_CONFIG, 0x06); //DLPF disable 0x06
   //Disable sensor output to FIFO buffer
   Write2Mpu6050(MPU6050_RA_FIFO_EN, 0x00);
}

// Self test MPU6050
/*
void SelftestMpu6050()
{
   Write2Mpu6050(MPU6050_RA_PWR_MGMT_1, 0x00); // internal 8MHz, disabled SLEEP mode, disable CYCLE mode
   Write2Mpu6050(MPU6050_RA_SMPLRT_DIV, 0x07); //sample rate: 8khz
   Write2Mpu6050(MPU6050_RA_CONFIG, 0x06); //DLPF disable
   Write2Mpu6050(MPU6050_RA_GYRO_CONFIG, 0xe0); //Enable Selftest Gyroscope
   Write2Mpu6050(MPU6050_RA_ACCEL_CONFIG, 0xf0); //Enable Selftest Acceleromenter
   delay_ms(5);
   unsigned int8 xg,yg,zg;
   char temp[16];
   xg=ReadMpu6050(MPU6050_RA_SELF_TEST_X);
   yg=ReadMpu6050(MPU6050_RA_SELF_TEST_Y);
   zg=ReadMpu6050(MPU6050_RA_SELF_TEST_Z);
   delay_ms(5);
   sprintf(temp,"%ux%uy%uz\r\n",xg,yg,zg);
   fprintf(com1,temp);
}
*/

float32 get_angle()
{
   float32 angle=0;
   int i;
   signed int16 y_accel,z_accel;
   for(i=0;i<5;i++)
   {
      y_accel = GetdataMpu6050(MPU6050_ACCEL_YOUT_H )-MPU6050_OFFSET_ACCEL_Y;
      z_accel = GetdataMpu6050(MPU6050_ACCEL_ZOUT_H )-MPU6050_OFFSET_ACCEL_Z;
     angle+=atan2(y_accel,z_accel)* 57.296;
   }
   return(angle/5.0);
}

void TestMpu6050()
{
      signed int16 xg,yg,zg;
      signed int16 value;
      xg=GetdataMpu6050(MPU6050_ACCEL_XOUT_H);
      yg=GetdataMpu6050(MPU6050_ACCEL_YOUT_H);
      zg=GetdataMpu6050(MPU6050_ACCEL_ZOUT_H);
      sprintf(value,"%ldx%ldy%ldz\r\n",xg,yg,zg);
//!      fprintf(com1,value);
}

void main()
{
 InitMpu6050();
 TestMpu6050();
 while(1)
 {
  ReadMpu6050(MPU6050_WHO_AM_I);
 }
}
 
