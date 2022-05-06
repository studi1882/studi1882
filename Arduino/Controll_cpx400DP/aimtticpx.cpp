/*
 * heater.cpp
 *
 *  Created on: 14.09.2020
 *      Author: Dimitri Benke
 *  Major refactor: 05.05.2022
 *
 */

#include "aimtticpx.h"

#include <SoftwareSerial.h>
#include <Arduino.h>



AimTTI_CPX::AimTTI_CPX(){
	// Constructor of Heater Object, Initializes Serial Port and measures current properties of Heater

	m_port = 0;

	m_resistance=0;
	m_current=0;
	m_voltage=0;
	m_power=0;
	m_isActive=true;
	m_isConnected=false;
}
void AimTTI_CPX::begin(int setPort, Stream* Serial)
{
	m_port = setPort;
	m_Serial = Serial;
	checkConnection();
}
bool AimTTI_CPX::checkConnection(){
	if (m_isActive== false){
		return false;
	}
	unsigned long readTime;
	//	int j=0;
	Serial.println("Heater Check");
	//m_Serial.listen();
	m_Serial->print("V");
	m_Serial->print(m_port);
	m_Serial->println("O?");
	delay(100);
	readTime=millis();
	while ((m_Serial->available()<1) && ((millis()-readTime)<1000)){
		delay(5);
		//Serial.print("waiting for heater Check  ");Serial.println((millis()-readTime));
	}
	if ((millis()-readTime)>950){
		Serial.println("Heater TimeOut");
		m_isConnected = false;
		return false;
	}
	else {
		Serial.println("Heater initialized");
		while (m_Serial->available()>0){
			m_Serial->read();
		}
		m_isConnected=true;
		return true;
	}
}
void AimTTI_CPX::turnOn()
{
	m_Serial->print("OP");
		m_Serial->print(m_port);
		m_Serial->print(' ');
		m_Serial->println(1);
}
void AimTTI_CPX::turnOff()
{
	m_Serial->print("OP");
		m_Serial->print(m_port);
		m_Serial->print(' ');
		m_Serial->println(0);
}
void AimTTI_CPX::setR(float setResistance){

	//Manually set a resistance
	m_resistance=setResistance;

}


float AimTTI_CPX::setV(float setVoltage, int tdelay = 1000){
	if (m_isConnected==false){
		return -1;
	}
  if (setVoltage>12){
    setVoltage=12;
    Serial.print(F("Die Spannung war groesser als "));
    Serial.print(setVoltage);
    Serial.print(F(" V, sie wurde auf "));
    Serial.print(setVoltage);
    Serial.println(" V gesetzt.");
  }
	m_Serial->print("V");
	m_Serial->print(m_port);
	m_Serial->print(' ');
	m_Serial->println(setVoltage);
	delay(tdelay);



	setVoltage=getV();

Serial.print("Voltage set to: ");
Serial.println(setVoltage);
	return setVoltage;
}
float AimTTI_CPX::setI(float setCurrent, int tdelay = 100){
	// sets Current Limit of PSU and reads actual Current
	if (m_isConnected==false){
		return -1;
	}
	if (setCurrent>4){
		setCurrent=4;
    Serial.print(F("Die Stromstaerke war groesser als "));
    Serial.print(setCurrent);
    Serial.print(F(" A , sie wurde auf "));
    Serial.print(setCurrent);
    Serial.println(" A gesetzt.");
	}
	m_Serial->print("I");
	m_Serial->print(m_port);
	m_Serial->print(' ');
	m_Serial->println(setCurrent);
Serial.print(F("Die Stromstaerke sollte auf "));
Serial.println(setCurrent);
Serial.println(" A gesetzt werden.");

	delay(tdelay);
	setCurrent=getI();
Serial.print("Current set to: ");
Serial.println(setCurrent);
	return setCurrent;
}
/*
 * Sets Power of the Heater
 */
float AimTTI_CPX::setP(float setPower){
	if (m_isActive==false){

		return -1;
	}
	if (m_isConnected==false){

		//Serial.println("Power set failed");
		return -1;
	}
	double setVoltage,setCurrent;
	if (m_resistance<=0){
		setVoltage=0;
		setCurrent=0;
		return 0;
	}
	else {
		setVoltage = sqrt(setPower*m_resistance);
		setCurrent = sqrt(setPower/m_resistance);
	}
	//Serial.println(" Power Set Start");
	setV(setVoltage);
	setI(setCurrent);
	m_voltage=getV();
	m_current=getI();
	//Serial.println(" Power Set Finish");
	return m_voltage*m_current;
}

float AimTTI_CPX::getV(){
	// Reads actual Voltage from PSU over RS-232, stores it in heater object and returns it
	if (m_isConnected==false){
		return -1;
	}
	char Buffer[20];
	int i=0;
	unsigned long readTime;

	//m_Serial->listen();
	m_Serial->print("V");
	m_Serial->print(m_port);
	m_Serial->println("O?");
	delay(100);
	readTime=millis();
	while ((m_Serial->available()<5) && ((millis()-readTime)<1000)){
		delay(1);

	}

	while (m_Serial->available()>0){

		Buffer[i]=m_Serial->read();

		i++;

	}

	float readVoltage=atof(Buffer);

	delay(100);

	return readVoltage;
}
float AimTTI_CPX::getI(){
	if (m_isConnected==false){
		return -1;
	}
	// Reads actual Current from PSU over RS-232, stores it in heater object and returns it
	char Buffer[20];
	int i=0;
	unsigned long readTime;
	//m_Serial->listen();
	m_Serial->print("I");
	m_Serial->print(m_port);
	m_Serial->println("O?");
	delay(100);
	readTime=millis();
	while ((m_Serial->available()<5) && ((millis()-readTime)<1000)){
		delay(1);

	}

	while (m_Serial->available()>0){
		Buffer[i]=m_Serial->read();
		i++;

	}

	float readCurrent=atof(Buffer);
	delay(100);

	return readCurrent;
}

float AimTTI_CPX::getR(){
	if (m_isConnected==false){
		return -1;
	}
	float V,I,readResistance;

	V=getV();
	I=getI();

	readResistance = V/I;
	delay(100);
	return readResistance;
}
float AimTTI_CPX::getP(){
	if (m_isConnected==false){
		return -1;
	}
	float V,I,readPower;

	V=getV();
	I=getI();

	readPower = V*I;
	delay(100);
	return readPower;
}

void AimTTI_CPX::refreshAll(){
	m_isConnected=checkConnection();
	if (m_isConnected==true){


		m_voltage=getV();
		m_current=getI();
		m_power=getP();
		setV(1);
		setI(1);
		m_resistance=getR();
		setV(m_voltage);
		setI(m_current);
	}
	else{
		Serial.println("Heater TimeOut");
	}
}
