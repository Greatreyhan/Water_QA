#include <Arduino.h>
#define TdsSensorPin 34
#define TurbiditySensor 35
#define VREF 5.0     
#define SCOUNT  30           
int analogBuffer[SCOUNT];   
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;
volatile float ntu = 0;
int getMedianNum(int bArray[], int iFilterLen);
int getSensorReading(int PinSensor);
void setup()
{
    Serial.begin(115200);
    pinMode(TdsSensorPin,INPUT);
    pinMode(TurbiditySensor, INPUT);
}

void loop()
{
   getSensorReading(TurbiditySensor);
}
int getSensorReading(int PinSensor){
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(PinSensor);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
      analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      
      if(PinSensor == TdsSensorPin){
        float compensationCoefficient=1.0+0.02*(temperature-25.0);    
        float compensationVolatge=averageVoltage/compensationCoefficient;  
        tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; \
        Serial.print("TDS Value:");
        Serial.println(tdsValue);
        return tdsValue;
      }
      else{
        if(averageVoltage < 2.5){
          ntu = 3000;
        }else{
          ntu = -1120.4*(averageVoltage*averageVoltage)+5742.3*averageVoltage-4353.8; 
        }
        Serial.print("NTU Value:");
        Serial.println(ntu);
        return ntu;
      }
   }
}
int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}