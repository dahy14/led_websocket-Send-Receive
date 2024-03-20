int ledGreen = 19;
int ledBlue = 18;
int ledRed = 5;

int channelGreen = 0;
int channelBlue = 1;
int channelRed = 2;

int refreshRate = 5000;
int bitResolution = 8;

void setup()
{
    pinMode(ledGreen, OUTPUT);
    pinMode(ledBlue, OUTPUT);
    pinMode(ledRed, OUTPUT);
    ledcSetup(channelGreen, refreshRate, bitResolution);
    ledcSetup(channelBlue, refreshRate, bitResolution);
    ledcSetup(channelRed, refreshRate, bitResolution);
    ledcAttachPin(ledGreen, channelGreen);
    ledcAttachPin(ledBlue, channelBlue);
    ledcAttachPin(ledRed, channelRed);
    Serial.begin(115200);
}

void loop()
{
}