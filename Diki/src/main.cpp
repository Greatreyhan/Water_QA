#define BLYNK_TEMPLATE_ID "TMPL6vHrqjRYr"
#define BLYNK_TEMPLATE_NAME "Water"
#define BLYNK_AUTH_TOKEN "80qwoGFi0MO5WbdEz5G3QSbwblJ1y2m0"
#include <Arduino.h>
#include <math.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

/* Fill-in information from Blynk Device Info here */

/* Comment this out to disable prints and save space */


char ssid[] = "infinergy";
char pass[] = "okeokeoke";
BlynkTimer timerBlynk;

float volt;
float ntu;
 
float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}

float reading_turbidity(){
  volt = 0;
    for(int i=0; i<800; i++)
    {
        volt += ((float)analogRead(35)/4095)*3.3;
    }
    volt = volt/800;
    volt = round_to_dp(volt,2);
    if(volt < 2.5){
      ntu = 3000;
    }else{
      ntu = -1120.4*(volt*volt)+5742.3*volt-4353.8; 
    }
    return ntu;
}

float reading_tds(){
  volt = 0;
    for(int i=0; i<800; i++)
    {
        volt += ((float)analogRead(34)/4095)*3.3;
    }
    volt = volt/800;
    volt = round_to_dp(volt,2);
    float tdsValue=(133.42*volt*volt*volt - 255.86*volt*volt + 857.39*volt)*0.5;
    return tdsValue;
}

void uploadingData(){
    float turb = reading_turbidity();
    Serial.print("Turbidity : ");
    Serial.println(turb);
    Blynk.virtualWrite(V4, turb);
    float tds = reading_tds();
    Serial.print("TDS : ");
    Serial.println(tds);
    Blynk.virtualWrite(V5, tds);
}

void setup()
{
  Serial.begin(115200);
  timerBlynk.setInterval(1L, uploadingData); //Staring a timer
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected");
}
 
void loop()
{
    Blynk.run();
    timerBlynk.run();
}
 
