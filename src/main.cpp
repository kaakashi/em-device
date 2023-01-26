#include <Arduino.h>
#include <SPI.h>
#include <ADE9153A.h>
#include <ADE9153AAPI.h>

void resetADE9153A(void);

#define SPI_SPEED 1000000     // SPI Speed
#define CS_PIN 12             // 8-->Arduino Zero. 15-->ESP8266
#define ADE9153A_RESET_PIN 19 // On-board Reset Pin
#define USER_INPUT 21         // On-board User Input Button Pin

#define RELAY_PIN 22
#define RELAY_LED 14

#define RESET_PIN  19



ADE9153AClass ade9153A;

AcalRegs acalVals;
PQRegs pqVals;
Temperature tempVal;

void setup() {

  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RELAY_LED, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  resetADE9153A();
  delay(100);

  if(ade9153A.SPI_Init(SPI_SPEED, CS_PIN))
  {
    Serial.println("spi init");
    ade9153A.SPI_Write_16(REG_AI_PGAGAIN, 0x000A);
    
    ade9153A.SPI_Write_32(REG_CONFIG0, 0);
    ade9153A.SPI_Write_16(REG_EP_CFG,ADE9153A_EP_CFG);
	  ade9153A.SPI_Write_16(REG_EGY_TIME,ADE9153A_EGY_TIME);		//Energy accumulation ON
    
    ade9153A.SPI_Write_32(REG_AVGAIN, 0xFFF36B16);
    //ade9153A.SPI_Write_32(REG_AIGAIN, 0xF8000011);
    ade9153A.SPI_Write_32(REG_AIGAIN, 7316126);
    ade9153A.SPI_Write_16(REG_PWR_TIME, 3905+1);
    }
  digitalWrite(RELAY_PIN, LOW);
  delay(2000);
  digitalWrite(RELAY_PIN, HIGH);
}


//double cof = 12.8880143934; // uV/ADC
double cof = 13.1027412633; // uV/ADC
//double aicoff = 0.592690058585; // uA/ADC
double aicoff = 0.334210143474; // uA/ADC
//double apcoff = 1.5087426; // mW/ADC
//double apcoff = 1.0256576475;
double apcoff = 0.583797990358;
//double aenCoff = 0.583436597;
double aenCoff = 0.332012448975;

// the loop function runs over and over again until power down or reset
void loop() {
  signed long int volt, vrms, irms, power, energy;
  double pVolt;
  double pCurr;
  double pWatt;
  double pEgy;
  double PF;
  double Freq;
  double Temp;

  digitalWrite(RELAY_LED, !digitalRead(RELAY_LED));
  
  ade9153A.ReadPQRegs(&pqVals);
  ade9153A.ReadTemperature(&tempVal);

  vrms =(signed long int)ade9153A.SPI_Read_32(REG_AVRMS_2);
  irms = (signed long int)ade9153A.SPI_Read_32(REG_AIRMS_2);
  power = (signed long int)ade9153A.SPI_Read_32(REG_AWATT);
  energy = (signed long int)ade9153A.SPI_Read_32(REG_AWATTHR_HI);

  pVolt = vrms;
  pVolt *= cof;
  pVolt /= 1000000;

  pCurr = irms;
  pCurr *= aicoff;
  pCurr /= 1000000;

  if(power < 0){
  power = -power;
  pWatt = power;
  pWatt *= apcoff;
  pWatt /= 1000;
  }
  else{
    pWatt = power;
    pWatt *= apcoff;
    pWatt /= 1000;
  }
  energy = -energy;
  pEgy = energy;
  pEgy *= aenCoff;
  pEgy /=1000000000;
  
  //Serial.printf("%d %d %.5f\n",irms, vrms, pVolt);
  
  // Serial.printf("%d\t%.5f\n",irms,pCurr);

  // Serial.print("Power Factor:\t");
  // Serial.println(pqVals.PowerFactorValue);
  
  PF = pqVals.PowerFactorValue;
  
  /*
  if(PF<0.0){
    PF += 0.07;
    PF *= -1;
  }
  else if(PF>0.0){
    PF -= 0.07;
    if(PF < 0.0)
    { 
      PF *= -1;
    }
  }*/

  if(PF>0.00){
    PF = 0.00;
  }
  else{
    PF *= -1;
  }
  
  Freq = pqVals.FrequencyValue;

  Temp = tempVal.TemperatureVal;
  
  //Serial.printf("%d",power);

  Serial.printf("%d %d %d %d %.5f A %.5f V %.5f W %.5f kWhr\n",irms, vrms, power, energy, pCurr, pVolt, pWatt, pEgy);

  //Serial.printf("%.5f A %.5f V %.5f W %.5f KWhr %.2f PF %.1f Hz %.2f degC\n",pCurr, pVolt, pWatt, pEgy, PF, Freq, Temp);

  delay(100);
  
}





















void resetADE9153A(void)
{

  digitalWrite(ADE9153A_RESET_PIN, LOW);
  delay(100);
  digitalWrite(ADE9153A_RESET_PIN, HIGH);
  delay(1000);
  Serial.println("Reset Done");
}
