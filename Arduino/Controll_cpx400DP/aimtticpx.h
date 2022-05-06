/*
 * heater.h
 *
 *  Created on: 14.09.2020
 *      Author: Dimitri Benke
 *  Major refactor: 05.05.2022
 */

#ifndef LIB_AIMTTICPX_AIMTTICPX_H_
#define LIB_AIMTTICPX_AIMTTICPX_H_
#include <Arduino.h>



class AimTTI_CPX{
public:
	AimTTI_CPX(); // Constructor sets Resistance of Heater Object
	void begin(int setPort, Stream* Serial); // put this in setup() after Serial.begin();
	float setV(float setVoltage, int tdelay = 1000); // sets Voltage
	float setI(float setCurrent, int tdelay = 100); // sets Current
	float setP(float setPower); // calculates Voltage based on Resistance and setPower

	void turnOn();
	void turnOff();
	float getV(); // gets Voltage from PSU
	float getI(); //gets Current from PSU
	float getP(); // gets Current and Voltage from PSU and Outputs Power
	float getR();// gets current and Voltage from PSU and calculates Resistance
	void refreshAll(); // Refreshes all stored values with values from PSU
	bool checkConnection(); // checks if PSU is connected
	float m_resistance;	// stores resistance, needed for power output
	float m_voltage;		//stores Voltage
	float m_current;		// stores m_current
	float m_power;		// Stores m_power
	bool m_isConnected;	// is true if device is connected
	bool m_isActive;
	Stream* m_Serial;	// Serial connection with 9600 baud
private:
	void setR(float setResistance);
	int m_port;		// number of PSU port

};


#endif /* LIB_AIMTTICPX_AIMTTICPX_H_ */
