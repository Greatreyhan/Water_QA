#include <Arduino.h>
#define TdsSensorPin 34
#define TurbiditySensor 35
#define VREF 5.0     
#define SCOUNT 30
float averageVoltage = 0, tdsValue = 0, temperature = 25;
volatile float ntu = 0;

int* analogBuffer;      
int* analogBufferTemp;   
int analogBufferIndex = 0;

int getMedianNum(int* bArray, int iFilterLen);
int getSensorReading(int PinSensor);

void setup() {
    Serial.begin(115200);
    pinMode(TdsSensorPin, INPUT);
    pinMode(TurbiditySensor, INPUT);

    analogBuffer = (int*)malloc(SCOUNT * sizeof(int));
    analogBufferTemp = (int*)malloc(SCOUNT * sizeof(int));

    if (analogBuffer == NULL || analogBufferTemp == NULL) {
        Serial.println("Gagal mengalokasikan memori!");
        while (1); 
    }
}

void loop() {
    getSensorReading(TurbiditySensor);
}

int getSensorReading(int PinSensor) {
    static unsigned long analogSampleTimepoint = millis();

    if (millis() - analogSampleTimepoint > 40U) { 
        analogSampleTimepoint = millis();
        analogBuffer[analogBufferIndex] = analogRead(PinSensor); 
        analogBufferIndex++;
        if (analogBufferIndex == SCOUNT) 
            analogBufferIndex = 0;
    }

    static unsigned long printTimepoint = millis();
    if (millis() - printTimepoint > 800U) { 
        printTimepoint = millis();

        for (int i = 0; i < SCOUNT; i++) 
            analogBufferTemp[i] = analogBuffer[i];

        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;

        if (PinSensor == TdsSensorPin) {
            float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
            float compensationVoltage = averageVoltage / compensationCoefficient;
            tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage -
                        255.86 * compensationVoltage * compensationVoltage + 
                        857.39 * compensationVoltage) * 0.5;

            Serial.print("TDS Value: ");
            Serial.println(tdsValue);
            return tdsValue;
        } else {
            if (averageVoltage < 2.5) {
                ntu = 3000;
            } else {
                ntu = -1120.4 * (averageVoltage * averageVoltage) + 
                       5742.3 * averageVoltage - 4353.8;
            }
            Serial.print("NTU Value: ");
            Serial.println(ntu);
            return ntu;
        }
    }
}

int getMedianNum(int* bArray, int iFilterLen) {
    int* bTab = (int*)malloc(iFilterLen * sizeof(int)); 

    if (bTab == NULL) {
        Serial.println("Gagal mengalokasikan memori untuk median!");
        while (1); 
    }

    for (int i = 0; i < iFilterLen; i++)
        bTab[i] = bArray[i];

    for (int j = 0; j < iFilterLen - 1; j++) {
        for (int i = 0; i < iFilterLen - j - 1; i++) {
            if (bTab[i] > bTab[i + 1]) {
                int temp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = temp;
            }
        }
    }

    int median = (iFilterLen & 1) ? bTab[(iFilterLen - 1) / 2] 
                                  : (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;

    free(bTab);
    return median;
}
