#usr/bin/python3
import argparse
import sys
from PIL import Image
from pprint import pprint

def has_transparency(img):
    """
    Courtesy of Vinyl Da.i'gyu-Kazotetsu: https://stackoverflow.com/a/58567453

    Because Pillow loses transparency values when converting from "P"-mode images,
    and I don't know how it interacts with other modes, every image that isn't RGBA
    will lose any transparency information.
    """
    if img.mode == "RGBA":
        extrema = img.getextrema()
        if extrema[3][0] < 255:
            return True

    return False


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("files", nargs="+")
    parser.add_argument("--pixels", action="store_true", required=False)
    args = parser.parse_args()

    for file in args.files:
        im = Image.open(file)
        transparency = has_transparency(im)
        rgb = im.convert("RGBA" if transparency else "RGB")
        w, h = im.size
        pixels = list(rgb.getdata())
        
        if not args.pixels:
            bytes_to_transmit = (
                b"\x00" # "all channels linear alpha". I have no idea what that means.
                + (b"\x04" if transparency else b"\x03") # Transparency
                + w.to_bytes(4, "little") # Width in little endian
                + h.to_bytes(4, "little") # Height in little endian
                + bytes(channel for pixel in pixels for channel in pixel) # Pixel Data
            )

            sys.stdout.buffer.write(bytes_to_transmit)
        else:
            pprint(pixels)

main()
