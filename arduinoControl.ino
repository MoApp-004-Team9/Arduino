#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Stepper.h>

//global variable
char serial_input;

//led light definition
int fan_led = 4;
int humidifier_led = 5;

//lcd panel definition
LiquidCrystal_I2C lcd(0x27,16,2);

//fine dust definition
unsigned long pulse =0;
float ugm3 = 0;
int GP2Y1023 = 3;

//temperature and humidity definition
int dhtPin = 2;
DHT dht11(dhtPin,DHT11);
int errDht;
float temperature,humi;

//Step motor definition
const int stepsPerRevolution = 1024;
Stepper myStepper(stepsPerRevolution,11,9,10,8);
int stepFlag = 0;

//photo transistor definition
int photoPin = A0;
long photoTr = 0;
int photoVal = 0;
int flame_flag = 0;

//Buzzer definition
int buzzer1 = 6;
int buzzer2 = 7;

//water sensor definition
int waterSensor = A1;
int waterValue = 0;
float waterPercent = 0;

void setup(){
  //Wire.begin();
  lcd.init();
  lcd.backlight();
  myStepper.setSpeed(20);
  pinMode(GP2Y1023,INPUT);
  pinMode(buzzer1,OUTPUT);
  pinMode(buzzer2,OUTPUT);
  pinMode(fan_led,OUTPUT);
  pinMode(humidifier_led,OUTPUT);
  Serial.begin(9600);
}

void loop(){
  
  /* I2C device find code
  byte error,address;
  int nDevices;

  nDevices = 0;
  for(address = 1; address < 127 ; address++){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if(error == 0){
      Serial.print("I2C device found at address 0x");
      if(address < 16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if(error == 4){
      Serial.print("Unknown error at address 0x");
      if(address < 16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if(nDevices == 0)
      Serial.println("NO I2C devices found\n");
    else
      Serial.println("done\n");
  */

  //fine dust sensor control
  pulse = pulseIn(GP2Y1023,LOW,20000);
  ugm3 = pulse2ugm3(pulse);

  //dht11 sensor control
  temperature = dht11.readTemperature();
  humi = dht11.readHumidity();

  //Step Motor control
  /*
  if(ugm3 > 36 and stepFlag == 1){
    myStepper.step(-stepsPerRevolution);
    stepFlag = 0;
  }
  */

  //photo transistor control
  int photoResult = analogRead(photoPin);

  //Buzzer control
  if(photoResult<=800){
    digitalWrite(buzzer1,HIGH);
    digitalWrite(buzzer2,HIGH);
    flame_flag = 1;
  }
  else{
    digitalWrite(buzzer1,LOW);
    digitalWrite(buzzer2,LOW);
    flame_flag = 0;
  }

  //water sensor control
  waterValue = analogRead(waterSensor);
  waterPercent = (waterValue - 500)/ 200.0 * 100.0;
  if(waterPercent < 0){
    waterPercent = 0;
  }
  
  //serial print section
  /*if(ugm3 > 1){
    Serial.print(ugm3);
    Serial.print("ug/m3");
  }
  Serial.print(" temperature : ");
  Serial.print(temperature);
  Serial.print(" humidity : ");
  Serial.println(humi);
  if(abs(photoResult - photoVal) > 0){
    Serial.print("photo : ");
    Serial.println(photoResult);
    photoVal = photoResult;
  }
  Serial.print("water: ");
  Serial.println(waterPercent);
  */
  

  //lcd print section
  lcd.clear();
  lcd.print("Dust: ");
  lcd.print(ugm3,1);
  lcd.print("ug/m3");
  lcd.setCursor(0,1);
  lcd.print("T:");
  lcd.print(temperature,0);
  lcd.print("`C H:");
  lcd.print(humi,0);
  lcd.print("g/m3");

  //write serial port
  char buf[255];
  
  int s_dust = ugm3;
  int s_temp = temperature;
  int s_humi = humi;
  int s_flame = flame_flag;
  int s_water = waterPercent;

  sprintf(buf,"%d:%d:%d:%d:%d:%cE",s_dust,s_temp,s_humi,s_flame,s_water,serial_input);

  Serial.write(buf);

  //read serial port and device control
  serial_input = '1';
  if(Serial.available()){
    serial_input = Serial.read();
  }
  
  //1:초기화 2:창문on 3:창문off 4:선풍기on 5:선풍기off 6:가습기on 7:가습기off
  if(serial_input == '2' && stepFlag == 0){
    myStepper.step(-341);
    stepFlag = 1;
  }
  else if(serial_input == '3' && stepFlag == 1){
    myStepper.step(341);
    stepFlag = 0;
  }
  else if(serial_input == '4'){
    digitalWrite(fan_led,HIGH);
  }
  else if(serial_input == '5'){
    digitalWrite(fan_led,LOW);
  }
  else if(serial_input == '6'){
    digitalWrite(humidifier_led,HIGH);
  }
  else if(serial_input == '7'){
    digitalWrite(humidifier_led,LOW);
  }
  
  delay(1000);
}

//User Function
float pulse2ugm3(unsigned long pulse){
  float value = (pulse - 1400) / 14.0;
  if(value > 300){
    value = 0;
  }
  return value;
}

