#include <global.h>
#include <task.h>
#include <rle.h>
#include <asset/bad_apple_cover_rle.h>

#define MENU_ITEM_COUNT 3
#define MENU_X 12
#define MENU_Y 44
#define MENU_LINE_H 18
#define MENU_LEFT_W 148
#define HIGHLIGHT_PAD 2
#define MENU_FONT_SIZE 1.5f
#define MENU_CHAR_W 9
#define ANIM_SPEED 0.25f
#define INTRO_STEP_MS 40
#define BATT_X 4
#define BATT_Y 125
#define COVER_X (CANVAS_W - BAD_APPLE_COVER_RLE_WIDTH)

static const char *menuLabels[] = {"Play", "Volume", "Brightness"};
static int cursorIdx = 0;
static float highlightY = MENU_Y - HIGHLIGHT_PAD;

static bool numInputActive = false;
static char numBuf[4] = {};
static int numBufLen = 0;

static uint8_t coverBuf[BAD_APPLE_COVER_RLE_WIDTH * BAD_APPLE_COVER_RLE_HEIGHT / 8];

static int getMenuValue(int idx) {
    if (idx == 1) return gVolume;
    if (idx == 2) return gBrightness;
    return -1;
}

static void setMenuValue(int idx, int val) {
    val = constrain(val, 0, 100);
    if (idx == 1) {
        gVolume = val;
        M5Cardputer.Speaker.setVolume(map(val, 0, 100, 0, 255));
    } else if (idx == 2) {
        gBrightness = val;
        M5Cardputer.Display.setBrightness(map(val, 0, 100, 0, 255));
    }
}

static void drawHighlight(float y) {
    canvas.fillRoundRect(MENU_X - 4, (int)y, MENU_LEFT_W - MENU_X, MENU_LINE_H + HIGHLIGHT_PAD, 3, 0x2945);
}

static void drawMenuItem(int idx, bool selected) {
    int y = MENU_Y + idx * MENU_LINE_H;
    canvas.setTextSize(MENU_FONT_SIZE);
    canvas.setTextColor(selected ? WHITE : 0x8C51);
    canvas.setCursor(MENU_X, y);
    canvas.print(menuLabels[idx]);

    int val = getMenuValue(idx);
    if (val >= 0) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d%%", val);
        int valW = strlen(buf) * MENU_CHAR_W;
        canvas.setCursor(MENU_LEFT_W - valW - 8, y);
        canvas.print(buf);
    }
}

static void drawCursorArrow(float y) {
    int cy = (int)y + MENU_LINE_H / 2;
    canvas.fillTriangle(2, cy - 4, 2, cy + 4, 8, cy);
}

static void drawTitle() {
    canvas.setTextSize(2);
    canvas.setTextColor(WHITE);
    canvas.setCursor(10, 10);
    canvas.print("Bad Apple!!");
}

static void drawSeparator() {
    for (int x = MENU_X; x < MENU_LEFT_W - 4; x += 2)
        canvas.drawPixel(x, 38, 0x4A49);
}

static void drawCover() {
    canvas.drawBitmap(COVER_X, 0, coverBuf, BAD_APPLE_COVER_RLE_WIDTH, BAD_APPLE_COVER_RLE_HEIGHT, BLACK, WHITE);
}

static void drawBattery() {
    int level = M5Cardputer.Power.getBatteryLevel();
    bool charging = M5Cardputer.Power.isCharging();

    char buf[32];
    snprintf(buf, sizeof(buf), "%s %d%%", charging ? "CHG" : "BAT", level);

    canvas.setTextSize(1);
    canvas.setTextColor(0x6B4D);
    canvas.setCursor(BATT_X, BATT_Y);
    canvas.print(buf);

    int verW = strlen(APP_VERSION) * 6;
    canvas.setCursor(MENU_LEFT_W - verW, BATT_Y);
    canvas.print(APP_VERSION);
}

static void pushMenu() {
    canvas.fillSprite(BLACK);
    drawTitle();
    drawSeparator();
    drawHighlight(highlightY);
    for (int i = 0; i < MENU_ITEM_COUNT; i++)
        drawMenuItem(i, i == cursorIdx);
    canvas.setTextColor(WHITE);
    drawCursorArrow(highlightY);
    drawBattery();
    drawCover();
    canvas.pushSprite(0, 0);
}

static void animateIntro() {
    // Cover
    for (int x = CANVAS_W; x >= COVER_X; x -= 4) {
        canvas.fillSprite(BLACK);
        canvas.drawBitmap(x, 0, coverBuf, BAD_APPLE_COVER_RLE_WIDTH, BAD_APPLE_COVER_RLE_HEIGHT, BLACK, WHITE);
        canvas.pushSprite(0, 0);
        vTaskDelay(pdMS_TO_TICKS(INTRO_STEP_MS));
    }

    // Title
    const char *title = "Bad Apple!!";
    for (int i = 0; title[i]; i++) {
        canvas.fillSprite(BLACK);
        drawCover();
        canvas.setTextSize(2);
        canvas.setTextColor(WHITE);
        canvas.setCursor(10, 10);
        for (int j = 0; j <= i; j++)
            canvas.print(title[j]);
        canvas.pushSprite(0, 0);
        vTaskDelay(pdMS_TO_TICKS(60));
    }

    // Separator
    for (int x = MENU_X; x < MENU_LEFT_W - 4; x += 2) {
        canvas.fillSprite(BLACK);
        drawCover();
        drawTitle();
        for (int px = MENU_X; px <= x; px += 2)
            canvas.drawPixel(px, 38, 0x4A49);
        canvas.pushSprite(0, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    // Menu items
    highlightY = MENU_Y - HIGHLIGHT_PAD;
    for (int step = 0; step < MENU_ITEM_COUNT; step++) {
        canvas.fillSprite(BLACK);
        drawCover();
        drawTitle();
        drawSeparator();
        drawHighlight(highlightY);
        for (int i = 0; i <= step; i++)
            drawMenuItem(i, i == cursorIdx);
        canvas.setTextColor(WHITE);
        drawCursorArrow(highlightY);
        canvas.pushSprite(0, 0);
        vTaskDelay(pdMS_TO_TICKS(120));
    }

    pushMenu();
}

static void animateHighlight() {
    float targetY = MENU_Y + cursorIdx * MENU_LINE_H - HIGHLIGHT_PAD;
    while (fabsf(highlightY - targetY) > 0.5f) {
        highlightY += (targetY - highlightY) * ANIM_SPEED;
        pushMenu();
        vTaskDelay(pdMS_TO_TICKS(16));
    }
    highlightY = targetY;
    pushMenu();
}

static void adjustValue(int idx, int delta) {
    int val = getMenuValue(idx);
    if (val < 0) return;
    setMenuValue(idx, val + delta);
    pushMenu();
}

static void startNumInput() {
    numInputActive = true;
    numBufLen = 0;
    numBuf[0] = '\0';
}

static void commitNumInput() {
    if (!numInputActive) return;
    numInputActive = false;
    if (numBufLen > 0)
        setMenuValue(cursorIdx, atoi(numBuf));
    pushMenu();
}

static void feedNumDigit(char c) {
    if (numBufLen < 3) {
        numBuf[numBufLen++] = c;
        numBuf[numBufLen] = '\0';
        setMenuValue(cursorIdx, atoi(numBuf));
        pushMenu();
    }
}

static void numInputBackspace() {
    if (numBufLen > 0) {
        numBuf[--numBufLen] = '\0';
        if (numBufLen > 0)
            setMenuValue(cursorIdx, atoi(numBuf));
        pushMenu();
    } else {
        numInputActive = false;
        pushMenu();
    }
}

void TaskMenu(void *pvParameters) {
    canvas.createSprite(CANVAS_W, CANVAS_H);
    rleDecodeFrame(BAD_APPLE_COVER_RLE, coverBuf, sizeof(coverBuf));
    animateIntro();

    unsigned long lastBattUpdate = millis();
    bool tonePlayedThisCycle = false;

    for (;;) {
        M5Cardputer.update();
        tonePlayedThisCycle = false;

        if (millis() - lastBattUpdate > 5000) {
            lastBattUpdate = millis();
            pushMenu();
        }

        if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState keys = M5Cardputer.Keyboard.keysState();

            if (!tonePlayedThisCycle) {
                M5Cardputer.Speaker.tone(800 + (esp_random() % 1200), 30);
                tonePlayedThisCycle = true;
            }

            for (auto key : keys.word) {
                if (key >= '0' && key <= '9' && getMenuValue(cursorIdx) >= 0) {
                    if (!numInputActive) startNumInput();
                    feedNumDigit(key);
                    continue;
                }

                if (numInputActive) commitNumInput();

                switch (key) {
                    case ';':
                        if (cursorIdx > 0) { cursorIdx--; animateHighlight(); }
                        break;
                    case '.':
                        if (cursorIdx < MENU_ITEM_COUNT - 1) { cursorIdx++; animateHighlight(); }
                        break;
                    case ',': adjustValue(cursorIdx, -5); break;
                    case '/': adjustValue(cursorIdx, 5); break;
                }
            }

            if (keys.del) {
                if (numInputActive) numInputBackspace();
            }

            if (keys.enter) {
                if (numInputActive) {
                    commitNumInput();
                } else if (cursorIdx == 0) {
                    canvas.fillSprite(BLACK);
                    canvas.pushSprite(0, 0);
                    startPlayTask();
                    break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    vTaskDelete(NULL);
}

void startMenuTask() {
    xTaskCreate(TaskMenu, "Menu", 4096, NULL, 1, NULL);
}