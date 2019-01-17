#include <HDC100X.h>
#include <knx.h>

#define SerialDBG SerialUSB

HDC100X HDC1(0x43);

// declare array of all groupobjects with their sizes in byte
GroupObject groupObjects[]
{
    GroupObject(2),
    GroupObject(2)
};

// create named references for easy access to group objects
GroupObject& goTemperature = groupObjects[0];
GroupObject& goHumidity = groupObjects[1];

long lastsend = 0;

// Helper functions declarations
void errLeds(void);

uint8_t sendCounter = 0;
uint32_t cyclSend = 0;

// Entry point for the example
void setup(void)
{
    SerialDBG.begin(115200);
    delay(5000);
    SerialDBG.println("start");

    // Programming LED on digital pin D5
    knx.ledPin(5);
    // Programming button on digital pin D7
    knx.buttonPin(7);    
    // register group objects
    knx.registerGroupObjects(groupObjects, 2);

    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    HDC1.begin(HDC100X_TEMP_HUMI,HDC100X_14BIT,HDC100X_14BIT,DISABLE);
    
    if (knx.configured())
    {
        cyclSend = knx.paramInt(0);
        SerialDBG.print("Zykl. send:");
        SerialDBG.println(cyclSend);
    }
    
    // start the framework.
    knx.start();

    String output = "Timestamp [ms], temperature [°C], relative humidity [%]";
    SerialDBG.println(output);
}


// Function that is looped forever
void loop(void)
{
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if(!knx.configured())
        return;
   
    long now = millis();
    if ((now - lastsend) < 3000)
        return;

    lastsend = now;

    float temp = HDC1.getTemp();
    float humi = HDC1.getHumi();
    String output = String(millis());
    output += ", " + String(temp);
    output += ", " + String(humi);
    SerialDBG.println(output);
        
    if (sendCounter++ == cyclSend)
    {
        sendCounter = 0;
    
        goTemperature.objectWrite(temp);
        goHumidity.objectWrite(humi);
    }
}

void errLeds(void)
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}