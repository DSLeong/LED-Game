
#ifndef PROJECT_HEADERS_IOEXPANDER_H_
#define PROJECT_HEADERS_IOEXPANDER_H_


class IoExpander {

public:
	IoExpander();
	void setPinAsOutput(int pin);
	void setAllPinsAsOutput();
	void pinHigh(int pin);
	void pinLow(int pin);
	void pinToggle(int pin);
	void setPinAsInput(int pin);
	void setAllPinsAsInput();
	int pinRead(int pin);

private:
	void applyPinDirections();
	void applyPinValues();
	bool isPin(int pin);

};


#endif /* PROJECT_HEADERS_IOEXPANDER_H_ */
