
#include "i2c.h"
#include "ioexpander.h"

using namespace USBDM;

static constexpr unsigned I2C_ADDRESS = 0x20 << 1;
static constexpr unsigned I2C_SPEED = 400 * kHz;

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

	pinDirections &= ~(1 << pin);
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

	pinValues |= (1 << pin);
	applyPinValues();

}

/**
 * Set an output pin on the IO Expander to a logic-low state.
 *
 * @param[in] pin - Pin number to be set low.
 */
void IoExpander::pinLow(int pin) {

	if (!isPin(pin)) return;

	pinValues &= ~(1 << pin);
	applyPinValues();

}

/**
 * Toggle an output pin on the IO Expander to be inverted from its previous logic level.
 *
 * @param[in] pin - Pin number to be toggled.
 */
void IoExpander::pinToggle(int pin) {

	if (!isPin(pin)) return;

	pinValues ^= (1 << pin);
	applyPinValues();

}

/**
 * Set a single pin on the IO Expander as an input pin.
 *
 * @param[in] pin - Pin number to be set as input.
 */
void IoExpander::setPinAsInput(int pin) {

	if (!isPin(pin)) return;

	pinDirections |= (1 << pin);
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
 * Transmit the current pin directions to the IO Expander.
 */
void IoExpander::applyPinDirections() {

	uint8_t txData[] = { 0x00, pinDirections };

	i2c.startTransaction();
	i2c.transmit(I2C_ADDRESS, sizeof(txData), txData);
	i2c.endTransaction();

}

/**
 * Transmit the current pin values to the IO Expander.
 */
void IoExpander::applyPinValues() {

	uint8_t txData[] = { 0x09, pinValues };

	i2c.startTransaction();
	i2c.transmit(I2C_ADDRESS, sizeof(txData), txData);
	i2c.endTransaction();

}

/**
 * Check if a pin number is part of the eight pins on the IO Expander.
 *
 * @param[in] pin - Pin number to check.
 *
 * @return - Boolean value, true if pin is part of IO Expander.
 */
bool IoExpander::isPin(int pin) {

	if (pin < 0 || pin > 7) return false;
	else return true;

}
