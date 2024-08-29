# -*- coding: utf-8 -*-
"""
Created on Mon Dec  4 20:23:50 2023

@author: https://github.com/Mark-MDO47
"""
import sys
import copy

MDO_IR_PIXEL_ARRAY_SIZE = 64 # similar to AMG88xx_PIXEL_ARRAY_SIZE in Adafruit library
MDO_IR_PIXEL_ROW_SIZE = 8

MDO_NUM_TGTS = 3
tgtTmpl8 = {"xy": [-1,-1], "veloc": [-9999, -9999], "val": -9999}


# mdo attempt
def show_pix(pixels):
    for iy in range(0, MDO_IR_PIXEL_ARRAY_SIZE, MDO_IR_PIXEL_ROW_SIZE):
        sys.stdout.write("%02d" % iy)
        for ix in range(int(MDO_IR_PIXEL_ARRAY_SIZE/MDO_IR_PIXEL_ROW_SIZE)):
            sys.stdout.write("%6.1f" % pixels[ix + iy])
        sys.stdout.write("\n")
    sys.stdout.write("\n")

# mdo attempt
def tgts_get_tmpl8(tgtSorted):
    a_tgt = copy.deepcopy(tgtTmpl8)
    a_tgt_split = tgtSorted.split(",")
    for idx in range(len(a_tgt_split)):
        a_tgt_split[idx] = int(a_tgt_split[idx])
    a_tgt["xy"]  = [a_tgt_split[1], a_tgt_split[2]]
    a_tgt["val"] = a_tgt_split[0]
    return a_tgt

# mdo attempt
def tgt_not_adjacent(a_tgt, tgt_list):
    not_adjacent = True
    for a_tgt_list in tgt_list:
        if (abs(a_tgt["xy"][0] - a_tgt_list["xy"][0]) <= 1) and (abs(a_tgt["xy"][1] - a_tgt_list["xy"][1]) <= 1):
            not_adjacent = False
            break
    return not_adjacent

# mdo attempt
def calc_eye_pointing_2(pixels, prevTgts, dbg):
    possible_tgts = []
    tgts = []
    # convert raw infrared pixel data to sorted "value,ix,iy"
    for iy, iy_big in enumerate(range(0, MDO_IR_PIXEL_ARRAY_SIZE, MDO_IR_PIXEL_ROW_SIZE)):
        for ix in range(int(MDO_IR_PIXEL_ROW_SIZE)):
            possible_tgts.append("%09d,%03d,%03d" % (int(1000*pixels[iy_big+ix]),ix,int(iy)))
    possible_tgts = sorted(possible_tgts, reverse=True)

    # get the three largest non-adjacent pixel values into tgts
    for a_possible_tgt_entry in possible_tgts:
        a_possible_tgt = tgts_get_tmpl8(a_possible_tgt_entry)
        if tgt_not_adjacent(a_possible_tgt, tgts):
            tgts.append(a_possible_tgt)
            if len(tgts) >= MDO_NUM_TGTS:
                break

# transliterated from https://github.com/adafruit/Adafruit_Learning_System_Guides.git M4_Eyes/HeatSensor.cpp
def calc_eye_pointing_1(pixels, dbg):
    x = 0
    y = 0
    magnitude = 0
    minVal = 100
    maxVal = 0
    i = 0;
    for iyPos in range(35, -40, -10):
        for ixPos in range(35, -40, -10):
            p = pixels[i]
            x += ixPos * p / 10.0
            y += iyPos * p / 10.0
            minVal = min(minVal, p)
            maxVal = max(maxVal, p)
            i += 1
    if dbg: 
        sys.stdout.write("unscaled x,y = %f,%f\n" % (x,y))
    x = - x / MDO_IR_PIXEL_ARRAY_SIZE / 5.0;
    y = y / MDO_IR_PIXEL_ARRAY_SIZE / 5.0;
    if dbg: 
        sys.stdout.write("  scaled x,y = %f,%f\n" % (x,y))
    x = max(-1.0, min(1.0, x));
    y = max(-1.0, min(1.0, y));
    if dbg: 
        sys.stdout.write("  minmax x,y = %f,%f\n" % (x,y))
    magnitude = max(0, min(50, maxVal - 20));
    return x, y, magnitude

# mdo debugging
MDO_DEBUG = True
pixels = [0.0] * MDO_IR_PIXEL_ARRAY_SIZE
pixels[0] = 70.0
show_pix(pixels)
x, y, mag = calc_eye_pointing_1(pixels, MDO_DEBUG)



"""
for (float yPos = 3.5; yPos > -4; yPos -= 1.0) {
  for (float xPos = 3.5; xPos > -4; xPos -= 1.0) {
    float p = pixels[i];
    x += xPos * p;
    y += yPos * p;
    minVal = min(minVal, p);
    maxVal = max(maxVal, p);
    i++;
  }
}
"""