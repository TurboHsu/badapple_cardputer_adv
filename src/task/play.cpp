#include <global.h>
#include <task.h>
#include <rle.h>
#include <asset/bad_apple_audio_wav.h>
#include <asset/bad_apple_video_frames_rle.h>

#define FRAME_BYTES (CANVAS_W * CANVAS_H / 8)
#define FRAME_MS (1000 / 8)

void startPlayTask() {
    xTaskCreatePinnedToCore(TaskPlayVideo, "PlayVideo", 8192, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(TaskPlayAudio, "PlayAudio", 4096, NULL, 1, NULL, 1);
}

void TaskPlayVideo(void *pvParameters) {
    RleVideo rv;
    rleVideoInit(rv, BAD_APPLE_VIDEO_FRAMES_RLE);

    uint8_t frameBuf[FRAME_BYTES];

    for (uint32_t i = 0; i < rv.numFrames; i++) {
        unsigned long start = millis();
        const uint8_t *src = rleVideoFrame(rv, i);
        rleDecodeFrame(src, frameBuf, FRAME_BYTES);
        canvas.fillSprite(BLACK);
        canvas.drawBitmap(0, 0, frameBuf, CANVAS_W, CANVAS_H, BLACK, WHITE);
        canvas.pushSprite(0, 0);

        long elapsed = millis() - start;
        if (elapsed < FRAME_MS)
            vTaskDelay(pdMS_TO_TICKS(FRAME_MS - elapsed));
    }

    startMenuTask();
    vTaskDelete(NULL);
}

void TaskPlayAudio(void *pvParameters) {
    M5Cardputer.Speaker.playWav(BAD_APPLE_AUDIO_WAV, sizeof(BAD_APPLE_AUDIO_WAV));
    vTaskDelete(NULL);
}
