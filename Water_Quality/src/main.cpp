#include <Arduino.h>

#define Sensor A0
#define onTDS D1
#define onTurbi D2
#define VREF 3.0
#define SCOUNT 30

int analogBuffer[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;
volatile float ntu = 0;

int getMedianNum(int bArray[], int iFilterLen);
float getSensorReading(int PinSensor);

static unsigned long analogSampleTimepoint = 0;
static unsigned long printTimepoint = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(onTDS, OUTPUT);
  pinMode(onTurbi, OUTPUT);
}

void loop()
{
  digitalWrite(onTDS, HIGH);
  digitalWrite(onTurbi, LOW);
  float tdsReading = getSensorReading(onTDS);
  delay(2000);
  digitalWrite(onTDS, LOW);
  digitalWrite(onTurbi, HIGH);
  float turbidityReading = getSensorReading(onTurbi);
}

float getSensorReading(int PinSensor)
{
  if (millis() - analogSampleTimepoint > 40U) // every 40 milliseconds, read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(Sensor); // read the analog value and store it in the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }

  if (millis() - printTimepoint > 800U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
    {
      averageVoltage = getMedianNum(analogBuffer, SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    }
    if (PinSensor == onTDS)
    {
      float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
      float compensationVoltage = averageVoltage / compensationCoefficient;
      tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;
      Serial.print("TDS Value:");
      Serial.print(tdsValue);
      return tdsValue;
    }
    else
    {
      if (averageVoltage < 2.5)
      {
        ntu = 3000;
      }
      else
      {
        ntu = (-1120.4 * (averageVoltage * averageVoltage) + 5742.3 * averageVoltage - 4353.8);
      }
      Serial.print("NTU Value:");
      Serial.print(ntu);
      return ntu;
    }
  }

  // Add a default return value in case the condition is not met
  return 0.0;
}

int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (int i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int iFilterMid = iFilterLen / 2;

  // Use Arduino's sort function to sort the array
  std::sort(bTab, bTab + iFilterLen);

  // Calculate median based on the sorted array
  if (iFilterLen % 2 == 0)
  {
    return (bTab[iFilterMid - 1] + bTab[iFilterMid]) / 2;
  }
  else
  {
    return bTab[iFilterMid];
  }
}
