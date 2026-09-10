#define BLYNK_TEMPLATE_ID "TMPL3yiCbTFBd"
#define BLYNK_TEMPLATE_NAME "Smart Home Automation"
#define BLYNK_AUTH_TOKEN "54C6RUbYIaNVwSeh6GSK7mRu0MV7HOT5"

#include <WiFi.h>
#include<BlynkSimpleEsp32.h>
#include<DHT.h>

char ssid[] = "CirkitWifi";
char pass[] = "";

//------------------- Pin Definition -------------------

#define RELAY1 16
#define RELAY2 17
#define RELAY3 18
#define RELAY4 19

#define PIRPIN 5
#define GASPIN 10
#define WATERPIN 3
#define FLAMEPIN 12
#define BUZZER 2
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN,DHTTYPE);
BlynkTimer timer;

int temperature,humidity,gaslevel,waterlevel;

#define DRY_VALUE 1000
#define WET_VALUE 4004

bool gasAlert = false;
bool flameAlert = false;
bool pirAlert = false;

bool humanDetected=false;
bool fanLockedByUser=false;
bool gasDetected=false;

//------------------- Relay Buttons -------------------

BLYNK_WRITE(V0)
{
    int Button1=param.asInt();

    if(Button1==1)
        digitalWrite(RELAY1,LOW);
    else
        digitalWrite(RELAY1,HIGH);   
}
BLYNK_WRITE(V1)
{
    int Button2=param.asInt();

    if(Button2==1)
        digitalWrite(RELAY2,HIGH);
    else
        digitalWrite(RELAY2,LOW);
}
BLYNK_WRITE(V2)
{
    int Button3=param.asInt();

    if(Button3)
    {
        digitalWrite(RELAY3,LOW);
        fanLockedByUser=false;
    }
    else
    {
        digitalWrite(RELAY3,HIGH);
        fanLockedByUser=true;
    }
}
BLYNK_WRITE(V3)
{
    int Button4=param.asInt();

    if(Button4==1)
        digitalWrite(RELAY4,LOW);
    else
        digitalWrite(RELAY4,HIGH);
}

//------------------- Send Sensor Data -------------------

void sendSensor()
{
    float temp=dht.readTemperature();
    float hum=dht.readHumidity();

    temperature = map(temp,0,50,0,100);
    temperature= constrain(temperature,0,100);

    humidity = map(hum,0,80,0,100);
    humidity= constrain(humidity,0,100);

    Blynk.virtualWrite(V4,temperature);
    Blynk.virtualWrite(V5,humidity);

 //---------------- GAS ----------------

    int gasvalue=analogRead(GASPIN);

    gaslevel= map(gasvalue,0,4004,0,100);
    gaslevel= constrain(gaslevel,0,100);

    if (gasvalue>1500)
    {
        fanLockedByUser=false;
        gasDetected=true;
        tone(2,1000);

        if(!fanLockedByUser)
        {
            digitalWrite(RELAY3,LOW);
        }
        if(!gasAlert)
        {
            gasAlert=true;
        }
        Blynk.virtualWrite(V10,1);
    }
    else
    {
        gasDetected=false;
        gasAlert=false;
        noTone(2);
        Blynk.virtualWrite(V10,0);
    }
    Blynk.virtualWrite(V6,gaslevel);

//---------------- FLAME ---------------

    int flamevalue=digitalRead(FLAMEPIN);

    if(flamevalue==LOW)
    {
        digitalWrite(BUZZER,HIGH);
        tone(2,1000);

        if(!flameAlert)
        {
            flameAlert=true;
        }
        Blynk.virtualWrite(V8,1);
    }
    else
    {
        flameAlert=false;
        noTone(2);
        Blynk.virtualWrite(V8,0);
    }
    
//---------------- WATER SENSOR ----------------

    int watervalue=analogRead(WATERPIN);

    waterlevel = map(watervalue,DRY_VALUE,4004,0,100);
    waterlevel = constrain(waterlevel,0,100);

    if(watervalue<=1000)
    {
        digitalWrite(RELAY4,LOW);
    }
    else if(watervalue >=3800)
    {
        digitalWrite(RELAY4, HIGH); 
    }
    Blynk.virtualWrite(V7,waterlevel);

}

void setup()
{
    pinMode(RELAY1,OUTPUT);
    pinMode(RELAY2,OUTPUT);
    pinMode(RELAY3,OUTPUT);
    pinMode(RELAY4,OUTPUT);

    pinMode(PIRPIN,INPUT);
    pinMode(FLAMEPIN,INPUT);
    pinMode(BUZZER,OUTPUT);

    digitalWrite(RELAY1,HIGH);
    digitalWrite(RELAY2,LOW);
    digitalWrite(RELAY3,HIGH);
    digitalWrite(RELAY4,HIGH);

    digitalWrite(BUZZER,LOW);
    
    Serial.begin(115200);
    dht.begin();
    Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass);
    timer.setInterval(1000L,sendSensor);
   
}

void loop()
{
    Blynk.run();
    timer.run();

    int motion=digitalRead(PIRPIN);
    if(motion)
    {
        humanDetected=true;
        fanLockedByUser=false;

        if(!pirAlert)
        {
            pirAlert=true;
            tone(2,1000);
        }
        Blynk.virtualWrite(V9,1);
    }
    else
    {
        pirAlert=false;
        Blynk.virtualWrite(V9,0);
    }
  
    if(humanDetected && !fanLockedByUser )
    {
        if (temperature>=51)
        {
           digitalWrite(RELAY3, LOW);   
        } 
    }

}
