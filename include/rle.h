#pragma once

#include <Arduino.h>

struct RleVideo {
    uint32_t numFrames;
    const uint32_t *offsets;
    const uint8_t *data;
};

void rleVideoInit(RleVideo &rv, const uint8_t *blob);
const uint8_t *rleVideoFrame(const RleVideo &rv, uint32_t idx);
size_t rleDecodeFrame(const uint8_t *src, uint8_t *dst, size_t dstSize);
