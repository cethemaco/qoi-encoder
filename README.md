# QOI C Encoder

This is an encoder for the [Quite OK Image Format](https://qoiformat.org/). (Available on Github [here.](https://github.com/phoboslab/qoi))

The encoder is written in C, with Python being used for Image conversion via Pillow.

# Dependencies
- Python (made with Python 3.8.10, will probably work with future version, older ones maybe not)
- Pillow (made with version 10.1.0)
- `gcc`
- `make`

# Usage

## Linux
After making sure every dependency is installed, just run `make`. This repository includes the same tests images as [the website.](https://qoiformat.org/qoi_test_images.zip)

To use your own images:
```bash
$ make qoi.o
$ python3 imgtobytes.py filename.png | ./qoi.o > output.qoi
```
with `filename.png` being the filename of any image you like (and that Pillow supports), and `output.qoi` the name of the export file

## Windows
There probably is a way to run it natively on Windows. Just haven't figured it out yet. Sorry! Just use the WSL and follow the Linux instructions.

# Limitations / Caveats
- When converting images from "P" mode to "RGBA" mode, Pillow turns all transparent pixels into either black and white. Because of that, all images saved in "P" will lose their transparency.
- Compression isn't as good as the reference encoder