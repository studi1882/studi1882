
#include "aimtticpx.h"

AimTTI_CPX my_AimTTI_CPX;

//einfache PID
#include <PID_v1.h>//zum PID package http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/
//ist das das aktuellste Paket? => ggf. sonst aendern

//moechte ich interrupts benutzen? siehe zum Beispiel:
//https://www.arduino.cc/reference/en/language/functions/interrupts/interrupts/
//https://www.tutorialspoint.com/arduino/arduino_interrupts.htm


//noch einmal anzuschauen:
//https://forum.arduino.cc/t/pid-controller-sample-time/126170

#include <PWFusion_MAX31856.h>

#define NUM_THERMOCOUPLES   (sizeof(tcChipSelects) / sizeof(uint8_t))

unsigned long myTime; //initialisieren der Zeit

long last_set_current_Time = 0; //initialisieren der Zeit, wann das letzte Mal gemessen wurde
//unsigned int measuretemp;// wieder rein
int measuretemp = 1;//nur zum testen 
int delaytint = 500;//
int delayt = delaytint;//standard delay time
int delayt_used = delaytint;//tatsaechlich benutzte Verspätung, je nachdem, ob Spannung eingestellt oder nicht.
int max_voltage = 12;
int max_current = 4;
int delay_time_power_supply = 100;

float temperature_array[4];//hängt von der Zahl der Thermocouples ab.
float set_current;
float set_voltage;

String SerialBuffer;
char CharBuffer[5];

uint8_t tcChipSelects[] = {10, 9, 8, 7};  // define chip select pins for each thermocouple
MAX31856  thermocouples[NUM_THERMOCOUPLES];



//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp=2, Ki=5, Kd=1;//die Parameter ggf. automatisch anpassen oder in Abhängigkeit von der Temperatur
//mal getestet mit Material on top, da dauert es mit den aktuellen Werten echt ewig, bis es halbwegs stabil ist.
//es schwankt auch am Ende (nach 10 Minuten!) noch um 0.2°C etwa und es bildet sich viel Wasser auf der Oberfläche, ein weiteres Problem, die Luft muss ich auf jeden Fall sehr trocken machen.
//also Problem: Wie bekomme ich schnell eine gute Regelung hin.
//ggf. die Temperatur auf der Oberfläche mit kontrollieren.
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, REVERSE);//DIRECT für Heizen, REVERSE fürs Kühlen

void setup()
{
    // Give the MAX31856 a chance to stabilize
  delay(1000);  

  Serial.begin(115200);  // set baudrate of serial port
  Serial.println(F("Playing With Fusion: MAX31856, SEN-30007/8"));
  Serial.println(F("Continous Mode Example"));
  
  // Initialize each MAX31856... options can be seen in the PWFusion_MAX31856.h file
  for (int i=0; i<NUM_THERMOCOUPLES; i++)
  {
    thermocouples[i].begin(tcChipSelects[i]);
    thermocouples[i].config(T_TYPE, CUTOFF_60HZ, AVG_SEL_1SAMP, CMODE_AUTO);
  
    Serial2.begin(9600); //serial with cpx400DP
  my_AimTTI_CPX.begin( 2, &Serial2);//die klasse so abändern, dass man einen default port einstellt, den man aber ändern kann
  //testen, ob ich AimTTI_CPX.m_port = 1 // kann ich so den Port ändern? eine funktion schreiben, die für ein Senden den m_port ändert.
  my_AimTTI_CPX.turnOn();
//  my_AimTTI_CPX.setV(max_voltage);
//    delay(2000);
//    my_AimTTI_CPX.setV(0);
      my_AimTTI_CPX.setV(max_voltage);
      my_AimTTI_CPX.setI(1);


  
  //my_AimTTI_CPX.turnOff();//wir lassen den an.
  
  
  
  Input = 24.02;//hier später die erste Temperatur einlesen
  //initialize the variables we're linked to
  Setpoint = 15;//gewuenschte Temperatur //hier beim Regeln akutelle 44% Luftfeuchtigkeit und 23°C => Taupunkt 10.6°C

  //turn the PID on
  myPID.SetMode(AUTOMATIC);//Regelung irgendwo neu starten können
}
}

void loop()
{
  //Checken, ob Serieller Input vorhanden
  if (Serial.available() > 0)
  {
    serialEventDB();
  }
  
  //delay(delayt);                                   // to test 500ms delay... can be as fast as ~100ms in continuous mode, 1 samp avg

  if(measuretemp == 1)
  {
    delay(delayt_used);                                  // 500ms delay... can be as fast as ~100ms in continuous mode, 1 samp avg
    //statt delay mache ich das so wie im PID code über eine Abfrage: // entnommen aus: http://brettbeauregard.com/blog/2011/04/improving-the-beginner%e2%80%99s-pid-sample-time/
    //unsigned long now = millis();
   //int timeChange = (now - lastTime);
   //if(timeChange>=SampleTime)
   //{
    //
    Serial.println(delayt_used);
    Serial.print(F("/*"));
    for (int i=0; i<NUM_THERMOCOUPLES; i++)
    {
      //Print the current time
      //Serial.print("Time: ");
      myTime = millis();
      Serial.print(myTime); // prints time since program started without enter
      Serial.print(F(","));
  
      // Get latest measurement from MAX31856 channels
      thermocouples[i].sample();
    
      // Print information to serial port
      temperature_array[i] = print31856Results(i, thermocouples[i]);
  
      // Attempt to recove misconfigured channels
      if(thermocouples[i].getStatus() == 0xFF)
      {
        thermocouples[i].config(T_TYPE, CUTOFF_60HZ, AVG_SEL_1SAMP, CMODE_AUTO);
        if(i == 3)
        {
          Serial.print(F("0"));//"re-attempt config on TC"));//0 wenn wieder verbindet, kein Komma, um csv Format zu erfüllen
        }
        else
        {
          Serial.print(F("0,"));//"re-attempt config on TC"));//0 wenn wieder verbindet
        }
        //Serial.print(i);
        //Serial.print(","); // ich glaube die Line brauche ich nicht.
      }
      else
      {
        if(i == 3)
        {
          Serial.print(F("1"));//beim letzten kein leerzeichen haben
        }
        else
        {
          Serial.print(F("1,"));
        }
      }
    }
    Serial.println(F("*/"));
  }
  Input = temperature_array[3];
  myPID.Compute();
  Serial.print(F("Der PID sagt:"));
  Serial.println(Output);

  //das ganze noch an das Spannungsgeraet haengen
    //my_AimTTI_CPX.turnOn();//hier noch eine Funktion bauen, die checkt, ob das an ist, und die checkt, dass die Spannung nicht zu hoch ist
    //wir setzen die Stromstärke nur alle 2 Sekunden
   if(last_set_current_Time+200 < myTime)
    {
          myTime = millis();
    Serial.println(myTime);
    last_set_current_Time = myTime; 
    set_current = max_current*Output/255;//hier eine Unterscheidung zwischen Heating und cooling machen, diese ist fürs Heizen, also je nach Temperatur des Elektromagneten und der Umgebung und der internen Temperatur
    //Bullshit, unterscheidung oben über Richtung set_current = max_current*(255-Output)/255;//hier eine Unterscheidung zwischen Heating und cooling machen,diese ist fürs Kühlen
    my_AimTTI_CPX.setI(set_current, delay_time_power_supply);

    //set_voltage = max_voltage*Output/255;

    //my_AimTTI_CPX.setV(set_voltage, delay_time_power_supply);//dauert aktuell etwa 300 ms
    //delay_time_power_supply = -(myTime- millis());
    myTime = millis();
    Serial.println(myTime);
        if (delayt-300>0){//delay_time_power_supply // diese delay time in die Setup funktion, damit das vorher geklärt wird.
    delayt_used = delayt-300;//delay_time_power_supply
        }
        else
        {
          delayt_used = 0;
        }
    
    }
    else
    {
      delayt_used = delayt;
    }


  
  //analogWrite(PIN_OUTPUT, Output);

  //else // zum testen
  //{
  //  Serial.println(F("no measurement running"));
  //}
}


float print31856Results(uint8_t channel, MAX31856 &tc)
{
  float temperature;
  uint8_t status = tc.getStatus();

  //Serial.print("Thermocouple ");
  //Serial.print(channel);

  if(status)
  {
    // lots of faults possible at once, technically... handle all 8 of them
    // Faults detected can be masked, please refer to library file to enable faults you want represented

//    bis if(TC_FAULT_CJ_OOR & status)      { Serial.print(F("\"CJ Range, \"")); } wieder entkommentieren und die nächste Zeile raus
    Serial.print(F(""));
//    Serial.print(F("\"0 FAULTED - \""));
//    if(TC_FAULT_OPEN & status)        { Serial.print(F("\"OPEN, \"")); }
//    if(TC_FAULT_VOLTAGE_OOR & status) { Serial.print(F("\"Overvolt/Undervolt, \"")); }
//    if(TC_FAULT_TC_TEMP_LOW & status) { Serial.print(F("\"TC Low, \"")); }
//    if(TC_FAULT_TC_TEMP_HIGH & status){ Serial.print(F("\"TC High, \"")); }
//    if(TC_FAULT_CJ_TEMP_LOW & status) { Serial.print(F("\"CJ Low, \"")); }
//    if(TC_FAULT_CJ_TEMP_HIGH & status){ Serial.print(F("\"CJ High, \"")); }
//    if(TC_FAULT_TC_OOR & status)      { Serial.print(F("\"TC Range, \"")); }
//    if(TC_FAULT_CJ_OOR & status)      { Serial.print(F("\"CJ Range, \"")); }
//    //Serial.println();
    Serial.print(F(",,"));//dann gibt es auch keine Temperatur
  }
  else  // no fault, print temperature data
  {
    //Serial.println(F(": Good"));
    Serial.print(F("1,"));
    // MAX31856 External (thermocouple) Temp
    //Serial.print(F("TC Temp = "));                   // print TC temp heading
    temperature = tc.getTemperature();
    Serial.print(temperature);
    Serial.print(F(","));
  }

  // MAX31856 Internal Temp
  //Serial.print(F("Tint = "));
  float cjTemp = tc.getColdJunctionTemperature();
  if ((cjTemp > -100) && (cjTemp < 150))
  {
    Serial.print(cjTemp);
    Serial.print(F(","));
  }
  else
  {
    //Serial.println(F("Unknown fault with cold junction measurement"));
    //Serial.print(F(","));
  }
  //Serial.println();
  return temperature;
}

void serialEventDB()
{
  float Input=0;
  SerialBuffer = Serial.readString();
  SerialBuffer.toCharArray(CharBuffer,5);
  char decision=CharBuffer[0];
  SerialBuffer.remove(0,1);
  Input = SerialBuffer.toInt();//falls nachher flieskommazahlen dann toFloat
  //Serial.println(decision);//zum testen, was ankommt
  switch (decision){
  case 'A': // A for start (Anfang)
    if(Input < 95)
    {
      delayt = delaytint;
      Serial.print(F("The input delay time was to small or not given, it was set to "));
      Serial.print(delayt);
      Serial.println(F(" ms."));
    }
    else
    {
      delayt = (int) Input;
      Serial.print(F("The delay time was set to (ms) "));
      Serial.println(delayt);
    }
    measuretemp = 1;
    Serial.println(F("Time_1,  Status_1 , T_1,reconf_1 , Time_2, Status_2,  T_2 ,reconf_2 , Time_3,reconf_3 , Status_3, T_3,reconf_3 , Time_4, Status_4, T_4,reconf_4 "));//jeweils 0 für false
    break;
  case 'E'://Ende
  {  delayt = delaytint;
    measuretemp = 0;
    Serial.println(F("The measurement was finished"));
  }
    break;
  case 'T':
    if( Input <= 40 & Input >= 10)
    {
    Setpoint = Input;
    Serial.print(F("The temperature was set to "));
    Serial.print(Setpoint);
    Serial.println(F(" °C."));
    }
    else
    {
    Serial.print(F("The Input was too small or too high, it was kept at "));
    Serial.print(Setpoint);
    Serial.println(F(" °C."));
    }
    break;
  case 'N':
  my_AimTTI_CPX.turnOn(); //aendern, dass PID ausgemacht wird! //beim wieder anschalten letzten Output wieder reingeben, was wichtig ist! siehe: http://brettbeauregard.com/blog/2011/04/improving-the-beginner%e2%80%99s-pid-initialization/
  //Output setzten der Klasse PID
  break;
case 'F':
  my_AimTTI_CPX.turnOff();
  break;
  
  case 'D': //Debug
  {
    Serial.print("D");
    Serial.println(Input);
  }
  
    break;
  }
}
