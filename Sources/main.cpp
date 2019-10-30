// comment
#include "hardware.h"
#include "math.h"
#include "i2c.h"
#include "mma845x.h"
#include "ioexpander.h"
#include "pit.h"

#define PERIODOF(x) (1/x)

using namespace USBDM;

// PIT timer
using Timer = Pit;
using TmrChGameLogic = Timer::Channel<0>;
using TmrChScorer = Timer::Channel<1>;

// Accelerometer
I2c0 i2c0;
MMA845x accelerometer(i2c0, MMA845x::AccelerometerMode_2Gmode);

// IO Expander API class
IoExpander io;

static constexpr float MAX_TILT = 2000;
static constexpr float MIN_TILT = -2000;
static constexpr int MAX_OFFSET = 1;
static constexpr int MIN_OFFSET = -1;
static constexpr int LED_START_POS = 3;

static constexpr float gameScorerSpeed = 1 * Hz;
float gameLogicSpeed = 10 * Hz;

float acc = 0.0f;
float vel = 0.0f;
float pos = LED_START_POS;

uint8_t score = 0;
bool isGameRunning = false;

// Function Prototypes
void startGame();
void showScore();
void gameOver();
void gameLoop();
void initialisePIT();
void initialiseAccelerometer();
float getAcceleration();
int randomWalk();
void clamp(int&, int, int);
void clamp(float&, float, float);


int main() {

	// Initial setup
	initialiseAccelerometer();
	io.setAllPinsAsOutput();
	startGame();

	for (;;) {

		console.write("Score = ").writeln(score);
		console.write("Speed = ").write(gameLogicSpeed).writeln("Hz");

		waitMS(100);

	}

	return 0;

}

/**
 * Sets up a new game and shows a startup animation
 */
void startGame() {

	io.allPinsHigh();
	initialisePIT();

	acc = 0;
	vel = 0;
	pos = LED_START_POS;
	score = 0;
	gameLogicSpeed = 10*Hz;
	TmrChGameLogic::configure(PERIODOF(gameLogicSpeed), PitChannelIrq_Enabled);

	waitMS(500);
	isGameRunning = true;

}

/**
 * Shows the score in binary on the LED bar
 */
void showScore() {

	io.customPinValues(score);

}

/**
 * Ends the game and restarts it
 */
void gameOver() {

	isGameRunning = false;
	io.allPinsHigh();
	waitMS(1000);
	showScore();
	waitMS(3000);
	startGame();

}

/**
 * This is the main game logic. This is where the LED position gets updated and displayed.
 * This is a callback from a PIT channel
 */
void gameLoop() {

	if (!isGameRunning) return;

	// Game logic
	acc = getAcceleration(); 	// Get acceleration from accelerometer
	vel += acc;					// Add acceleration to velocity of LED
	vel *= 0.9f;				// Give the velocity some drag
	clamp(vel, -0.5f, 0.5f);	// Limit min and max velocity
	pos += vel;					// Add velocity to position of LED
	pos += randomWalk() / 2.0;	// Apply the random walk to the position

	int position = (int)round(pos);

	// Display LED position
	io.allPinsLow();
	io.pinHigh(position);

	// Check if LED is out of bounds
	if (pos > 7 || pos < 0) gameOver();

}

/**
 * Callback method that runs every second
 */
void gameScorer() {

	if (!isGameRunning) return;

	// Increase score
	score++;

	// Increase game speed
	gameLogicSpeed += 1 * Hz;
	TmrChGameLogic::configure(PERIODOF(gameLogicSpeed), PitChannelIrq_Enabled);

}

/**
 * Initialise the PIT
 */
void initialisePIT() {

	Timer::configure(PitDebugMode_Stop);

	TmrChGameLogic::setCallback(gameLoop);
	TmrChGameLogic::enableNvicInterrupts(NvicPriority_Normal);
	TmrChGameLogic::configure(PERIODOF(gameLogicSpeed), PitChannelIrq_Enabled);

	TmrChScorer::setCallback(gameScorer);
	TmrChScorer::enableNvicInterrupts(NvicPriority_Normal);
	TmrChScorer::configure(PERIODOF(gameScorerSpeed), PitChannelIrq_Enabled);

	// Check for errors so far
	checkError();

}

/**
 * Initialise the Accelerometer
 */
void initialiseAccelerometer(){

	console.write("Device ID = 0x").write(accelerometer.readID(), Radix_16).writeln(" (should be 0x1A)");

	checkError(); // Check if any USBDM error yet (constructors)
	console.write("Doing simple calibration\nMake sure the device is level!\n");
	waitMS(2000);

	ErrorCode err = accelerometer.calibrateAccelerometer(); // Calibrate

	// Check calibration error
	if (err != E_NO_ERROR) {
	  console.writeln("Calibration failed!");
	  __asm__("bkpt");
	}

	// Make sure we have new values
	waitMS(100);
	console.writeln("Calibration complete.");

}

/**
 * Get the acceleration of the LED based on the accelerometer's Y-axis value
 *
 * @return - Acceleration value
 */
float getAcceleration() {

	// Get the currents values of the Accelerometer
	int status;
	int16_t accX, accY, accZ;
	accelerometer.readAccelerometerXYZ(status, accX, accY, accZ);

	int value = accY;
	clamp(value, MIN_TILT, MAX_TILT);

	return -value / 2000.0f;

}

/**
 * Provides an offset value that may vary by up to +/- 1 on each call.
 *
 * @return - offset value in range [MIN_OFFSET .. MAX_OFFSET]
 */
int randomWalk() {

	static int offset = 0;

	switch(rand() % 2) {
		case 0:
			offset -= 1;
			break;
		case 1:
			offset += 1;
			break;
	}

	if (offset < MIN_OFFSET) {
		offset = MIN_OFFSET;
	}

	if (offset > MAX_OFFSET) {
		offset = MAX_OFFSET;
	}

	return offset;

}

/**
 * Limit/clamp a given value within a certain range
 *
 * @param[out] n - Value to be limited
 * @param[in] lower - Lower value of the limited range
 * @param[in] upper - Upper value of the limited range
 */
void clamp(int &n, int lower, int upper) {

	if (n >= upper) n = upper;
	else if (n <= lower) n = lower;

}

/**
 * Limit/clamp a given value within a certain range
 *
 * @param[out] n - Value to be limited
 * @param[in] lower - Lower value of the limited range
 * @param[in] upper - Upper value of the limited range
 */
void clamp(float &n, float lower, float upper) {

	if (n >= upper) n = upper;
	else if (n <= lower) n = lower;

}
