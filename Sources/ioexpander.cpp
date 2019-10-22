
#include "i2c.h"
#include "ioexpander.h"

using namespace USBDM;

static constexpr unsigned I2C_ADDRESS = 0x20 << 1;	// Address of i2c communication
static constexpr unsigned I2C_SPEED = 400 * kHz;	// SPeed of i2c communication
static constexpr uint8_t IO_IODIR = 0x00;			// IO-Expander IO Direction Register
static constexpr uint8_t IO_GPIO = 0x09;			// IO-Expander General Purpose Input Output

static constexpr uint8_t PINMASK(int x) {
	return (1 << x); // Create a mask for a pin number
}

I2c0 i2c { I2C_SPEED, I2cMode_Polled };

static volatile uint8_t pinDirections;
static volatile uint8_t pinValues;


/**
 * A GPIO Expander API for the MCP23008.
 * This API allows you to easily set pin directions and logic levels for the MCP23008 at a higher level.
 */
IoExpander::IoExpander() {

	pinDirections = 0x00;
	pinValues = 0x00;

}

/**
 * Set a single pin on the IO Expander as an output pin.
 *
 * @param[in] pin - Pin number to be set as output.
 */
void IoExpander::setPinAsOutput(int pin) {

	if (!isPin(pin)) return;

	pinDirections &= ~PINMASK(pin);
	applyPinDirections();

}

/**
 * Set all eight pins on the IO Expander as outputs.
 */
void IoExpander::setAllPinsAsOutput() {

	pinDirections = 0x00;
	applyPinDirections();

}

/**
 * Set an output pin on the IO Expander to a logic-high state.
 *
 * @param[in] pin - Pin number to be set high.
 */
void IoExpander::pinHigh(int pin) {

	if (!isPin(pin)) return;

	pinValues |= PINMASK(pin);
	applyPinValues();

}

/**
 * Set an output pin on the IO Expander to a logic-low state.
 *
 * @param[in] pin - Pin number to be set low.
 */
void IoExpander::pinLow(int pin) {

	if (!isPin(pin)) return;

	pinValues &= ~PINMASK(pin);
	applyPinValues();

}

/**
 * Toggle an output pin on the IO Expander to be inverted from its previous logic level.
 *
 * @param[in] pin - Pin number to be toggled.
 */
void IoExpander::pinToggle(int pin) {

	if (!isPin(pin)) return;

	pinValues ^= PINMASK(pin);
	applyPinValues();

}

/**
 * Set a single pin on the IO Expander as an input pin.
 *
 * @param[in] pin - Pin number to be set as input.
 */
void IoExpander::setPinAsInput(int pin) {

	if (!isPin(pin)) return;

	pinDirections |= PINMASK(pin);
	applyPinDirections();

}

/**
 * Set all eight pins on the IO Expander as inputs.
 */
void IoExpander::setAllPinsAsInput() {

	pinDirections = 0xFF;
	applyPinDirections();

}

/**
 * Read the current logic state of the given pin.
 *
 * @param[in] pin - The pin number to check.
 *
 * @return - Boolean value of logic level, if it's true then pin is high.
 */
bool IoExpander::pinRead(int pin) {

	if (!isPin(pin)) return false;

	uint8_t txData[] = { IO_GPIO };
	uint8_t rxData[1];

	i2c.startTransaction();
	i2c.txRx(I2C_ADDRESS, sizeof(txData), txData, sizeof(rxData), rxData);
	i2c.endTransaction();

	return (rxData[0] & PINMASK(pin));

}

/**
 * Transmit the current pin directions to the IO Expander.
 */
void IoExpander::applyPinDirections() {

	uint8_t txData[] = { IO_IODIR, pinDirections };

	i2c.startTransaction();
	i2c.transmit(I2C_ADDRESS, sizeof(txData), txData);
	i2c.endTransaction();

}

/**
 * Transmit the current pin values to the IO Expander.
 */
void IoExpander::applyPinValues() {

	uint8_t txData[] = { IO_GPIO, pinValues };

	i2c.startTransaction();
	i2c.transmit(I2C_ADDRESS, sizeof(txData), txData);
	i2c.endTransaction();

}

/**
 * Check if a pin number is part of the eight pins on the IO Expander.
 *
 * @param[in] pin - Pin number to check.
 *
 * @return - Boolean value, true if pin is part of the IO Expander.
 */
bool IoExpander::isPin(int pin) {

	if (pin < 0 || pin > 7) return false;
	else return true;

}
