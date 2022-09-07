import os
from PIL import Image
import numpy as np
import subprocess
import subprocess
import time
import telnetlib
import struct
import time
import os.path
import re


print("starting openocd:")
openocd_cmd = (
    "openocd -f interface/stlink.cfg -f target/stm32h7x.cfg").split()

openocd = subprocess.Popen(openocd_cmd)

counter = 0
time.sleep(1)
openocd.poll()
while openocd.returncode == 1:
    counter = counter + 1
    print("jtag not detected. retrying")
    openocd = subprocess.Popen(openocd_cmd)
    time.sleep(1)
    openocd.poll()

    if(counter == 5):
        print("openocd open? device not connected?")

        exit()


print("jtag detected")
print("open telnet:")


host = "localhost"
port = 4444

dirname = os.path.join(os.getcwd(), 'dump', 'dump.bin')
dirname = re.escape(dirname)

command = "dump_image " + dirname + ' 0xD0000000 0xBB800\n'


with telnetlib.Telnet(host, port) as session:

    session.read_until(b"> ")

    session.write(b"halt\n")
    session.read_until(b"> ")

    session.write(command.encode('UTF-8'))
    session.read_until(b"> ")

    print("dump written")

    session.write(b"resume\n")
    time.sleep(0.1)

    session.read_until(b"> ")
    print("resume")

    session.close()
print("close telnet:")


# Send the signal to all the process groups
openocd.kill()


print("Convert Dump to png (RGB565)")


line = 480
column = 800


with open("dump/dump.bin", "rb") as f:
    image = f.read()

dataArray = struct.unpack("H" * ((len(image)) // 2), image)


a = [0]*3
a = [a]*column
a = [a]*line
a = np.array(a, dtype=np.uint8)

for x in range(0, line):
    for y in range(0, column):
        index = y+column*x
        pixel = dataArray[index]

        R = pixel & 0b1111100000000000
        G = pixel & 0b0000011111100000
        B = pixel & 0b0000000000011111

        a[x, y, 0] = R >> 8
        a[x, y, 1] = G >> 3
        a[x, y, 2] = B << 3

# Use PIL to create an image from the new array of pixels
new_image = Image.fromarray(a, "RGB")

fileCounter = 0
fname = "screenshot_"
fend = ".png"

path = "image/" + fname + str(fileCounter) + fend

while(os.path.isfile(path)):
    fileCounter = fileCounter + 1
    path = "image/" + fname + str(fileCounter) + fend

print(path)
new_image.save(path)
