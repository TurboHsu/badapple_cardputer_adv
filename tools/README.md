# Tools (for data generation)

Contains utility scripts to generate asset files from source media.

You need to have `ffmpeg` and `python3` installed to run these scripts.

Generate the required data files by running the following commands:

## Cover

It's a RLE encoded monochrome image with 88x135 resolution.

```bash
ffmpeg -i tools/data/cover.jpg -vf "scale=88:135:flags=lanczos" -pix_fmt monob -f rawvideo tools/data/cover.bin
python tools/rle_encode.py --single tools/data/cover.bin -o tools/data/cover.rle
python tools/bin2h.py tools/data/cover.rle -o include/asset/bad_apple_cover_rle.h -n BAD_APPLE_COVER_RLE -W 88 -H 135
```

## Video

Video frames are resized to 240x135 at 8 FPS, then converted to a rle file with custom encoding.
Check `tools/rle_encode.py` for more details.

```bash
ffmpeg -i tools/data/input.mp4 -vf "fps=8,scale=240:135:flags=lanczos" -pix_fmt monob -f rawvideo tools/data/frames.bin
python tools/rle_encode.py tools/data/frames.bin -o tools/data/frames.rle
python tools/bin2h.py tools/data/frames.rle -o include/asset/bad_apple_video_frames_rle.h -n BAD_APPLE_VIDEO_FRAMES_RLE -W 240 -H 135
```

## Audio

It's a buzzer, I think 4kHz mono 8-bit PCM is good enough.

```bash
ffmpeg -i tools/data/input.mp4 -vn -ac 1 -ar 4000 -acodec pcm_u8 tools/data/audio.wav
python tools/bin2h.py -o bad_apple_audio_wav.h -n BAD_APPLE_AUDIO_WAV tools/data/audio.wav
```
