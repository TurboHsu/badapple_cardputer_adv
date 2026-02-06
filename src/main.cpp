#include <M5Cardputer.h>
#include <global.h>
#include <task.h>

// Global shared resources
M5Canvas canvas(&M5Cardputer.Display);
int gVolume = 50;
int gBrightness = 50;

void setup()
{
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Speaker.begin();

    // Read real hardware values
    gVolume = map(M5Cardputer.Speaker.getVolume(), 0, 255, 0, 100);
    gBrightness = map(M5Cardputer.Display.getBrightness(), 0, 255, 0, 100);

    startMenuTask();
}

void loop() {}