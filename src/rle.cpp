#include <rle.h>

void rleVideoInit(RleVideo &rv, const uint8_t *blob) {
    memcpy(&rv.numFrames, blob, 4);
    rv.offsets = (const uint32_t *)(blob + 4);
    rv.data = blob + 4 + rv.numFrames * 4;
}

const uint8_t *rleVideoFrame(const RleVideo &rv, uint32_t idx) {
    return rv.data + rv.offsets[idx];
}

size_t rleDecodeFrame(const uint8_t *src, uint8_t *dst, size_t dstSize) {
    size_t written = 0;
    while (written < dstSize) {
        uint8_t code = *src++;
        if (code == 0x80) break;
        if (code == 0x00) {
            dst[written++] = *src++;
        } else if (code < 0x80) {
            size_t n = code;
            if (written + n > dstSize) n = dstSize - written;
            memset(dst + written, 0x00, n);
            written += n;
        } else {
            size_t n = code & 0x7F;
            if (written + n > dstSize) n = dstSize - written;
            memset(dst + written, 0xFF, n);
            written += n;
        }
    }
    return written;
}
