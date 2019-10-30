#include "hardware.h"
#include "mma845x.h"
#include <math.h>
#include "system.h"
#include "derivative.h"
#include "i2c.h"
#include "delay.h"
#include "ioexpander.h"
//#include "pit.h"
//#include "smc.h"

using namespace USBDM;

IoExpander io;
I2c0       i2c0;

MMA845x  accelerometer(i2c0, MMA845x::AccelerometerMode_2Gmode);

//using Timer        = Pit;
//using TimerChannel = Timer::Channel<0>;

static constexpr int MAX_OFFSET = 4;
static constexpr int MIN_OFFSET = -4;
static const int AXIS_MAX = 2000;
static const int AXIS_MIN = -2000;

static int LED_Position = 0;
static int oldLED       = 0;
static int offset       = 0;
static float period     = 0.1;
static bool STARTGAME   = true;

//Function Prototypes
void InitialiseAccelerometer();
void InitialisePIT();
void AxisYConversion();
void randomWalk();

int main() {
   InitialiseAccelerometer();
   //InitialisePIT();
   io.setAllPinsAsOutput();

   int score;

   for (;;) {
      if (STARTGAME){
         console.writeln("Balance Game").writeln("Starts in...");
         waitMS(1000);
         for (int countDown = 3;countDown > 0; --countDown){
            console.writeln(countDown);
            waitMS(1000);
         }
         console.writeln("Start!");
         STARTGAME = false;
         score  = 0;
         offset = 0;
      }


      AxisYConversion();
      waitMS(500);

      // Scoring condition
      if (LED_Position == 3 || LED_Position == 4){
         score++;
      }

      // Increases the frequency of the callback 'offset'
      if (score%25 == 0){
     	 period *= 2;
     	 //TimerChannel::setPeriod(period/2 * seconds);
     	 randomWalk();
      }


      console.write("Score = ").write(score).write("; Speed = ").writeln(period);


      // Lost/End Game condition
      if (LED_Position <= 0 || LED_Position >= 7) {
    	 console.writeln("You have Lost").write("You Scored ").writeln(score);

    	 //MAKE THE LEDS GLOW
         //;

    	 console.writeln("Please reset position.");
    	 while (LED_Position != 3 && LED_Position != 4){
    		 AxisYConversion();
    	 }

    	 console.writeln("Beginning to Start...");
    	 waitMS(4000);
    	 STARTGAME = true;
      }
   }

   return 0;
}

/**
 * Initialise the Accelerometer
 */
void InitialiseAccelerometer(){
   console.writeln("Starting\n");
   console.write("Device ID = 0x").write(accelerometer.readID(), Radix_16).writeln("(should be 0x1A)");

   // Check if any USBDM error yet (constructors)
   checkError();

   console.write("Doing simple calibration\n"
		 "Make sure the device is level!\n");
   waitMS(2000);

   // Error Case
   if (accelerometer.calibrateAccelerometer() != E_NO_ERROR) {
      console.write("Calibration failed!\n");
      __asm__("bkpt");
   }

   // Make sure we have new values
   waitMS(100);

   console.write("After calibration\n");
}

/**
 * Initialise the PIT
 */
/*
void InitialisePIT(){
   Timer::configure(PitDebugMode_Stop);
   // Set handler programmatically
   TimerChannel::setCallback(randomWalk);
   // Configures the the frequency of the calling of  'randomWalk()'
   TimerChannel::configure(period * seconds, PitChannelIrq_Enabled);
   TimerChannel::enableNvicInterrupts(NvicPriority_Normal);
   // Check for errors so far
   checkError();
}
*/
/**
 * Converts the readings from the accelerometer to be usable for the LEDs
 */
void AxisYConversion(){
   //Gets the currents values of the Accelerometer
   int accelStatus;
   int16_t accelX,accelY,accelZ;
   accelerometer.readAccelerometerXYZ(accelStatus, accelX, accelY, accelZ);

   //Modifies 'accelY' for usage for the LED
   LED_Position = int(3.5 - accelY * (MAX_OFFSET - MIN_OFFSET)/(AXIS_MAX - AXIS_MIN));

   //Testing
   console.write("AccelY = ").write(accelY).write("; LED = ").writeln(LED_Position);

   //Applies randomWalk value
   //LED_Position += offset;

   //Display where the current LED is
   io.pinLow(oldLED);
   io.pinHigh(LED_Position);
   oldLED = LED_Position;
}

/**
 * Provides an offset value that may vary by up to +/- 1 on each call.
 */
void randomWalk() {
   switch(rand()%2) {
      case 0: offset -= 1; break;
      case 1: offset += 1; break;
   }
   if (offset < MIN_OFFSET) {
      offset = MIN_OFFSET;
   }
   if (offset > MAX_OFFSET) {
      offset = MAX_OFFSET;
   }
}
