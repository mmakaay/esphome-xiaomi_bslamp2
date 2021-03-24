#!/usr/bin/env python3
#
# This script was used to work out how to map the RGB/brightness input
# data to GPIO pinout voltages.

maxV = 3.27

requirements = [
    # Input                # Expected output
    (0, 0, 0, 1.00,        False, None, None, None),
    (255, 255, 255, 0.00,  False, None, None, None),
    (255, 0,   0, 0.01,    True, 2.95, maxV, maxV),
    (255, 0,   0, 1.00,    True, 1.82, 3.27, 3.27),
    (255, 255, 0, 0.01,    True, 3.04, 2.86, 3.17),
    (255, 255, 0, 1.00,    True, 2.56, 1.77, 3.15),
    (255, 255, 255, 0.01,  True, 3.04, 2.86, 3.07),
    (255, 255, 255, 1.00,  True, 2.56, 1.77, 2.73),
    (0, 255, 0, 0.01,      True, 3.27, 2.95, 3.27),
    (0, 255, 0, 1.00,      True, 3.27, 1.82, 3.27),
    (0, 255, 255, 0.01,    True, 3.13, 2.86, 3.07),
    (0, 255, 255, 1.00,    True, 2.97, 1.76, 2.71),
    (0, 0, 255, 0.01,      True, 3.27, 3.27, 2.95),
    (0, 0, 255, 1.00,      True, 3.27, 3.27, 1.82),
    (255, 0, 255, 0.01,    True, 2.88, 3.12, 2.86),
    (255, 0, 255, 1.00,    True, 1.85, 2.92, 1.77),
    (255, 128, 0, 0.01,    True, 2.86, 2.93, 3.17),
    (255, 128, 0, 1.00,    True, 1.77, 2.10, 3.15),
    (255, 0 ,128, 0.01,    True, 2.86, 3.12, 3.10),
    (255, 0 ,128, 1.00,    True, 1.77, 2.94, 2.84),
    (255, 128, 128, 0.01,  True, 2.86, 2.93, 3.10),
    (255, 128, 128, 1.00,  True, 1.77, 2.10, 2.86),
    (128, 255, 0, 0.01,    True, 3.11, 2.86, 3.17),
    (128, 255, 0, 1.00,    True, 2.87, 1.76, 3.15),
    (255, 128, 255, 0.01,  True, 2.86, 2.93, 2.87),
    (255, 128, 255, 1.00,  True, 1.76, 2.06, 1.80),
    (255, 64, 0, 0.01,     True, 2.86, 3.07, 3.17),
    (255, 64, 0, 1.00,     True, 0.01, 2.20, 3.12),
    (255, 0, 64, 0.01,     True, 2.86, 3.12, 3.15),
    (255, 0, 64, 1.00,     True, 0.01, 2.68, 2.95),
    (255, 0, 128, 0.01,    True, 2.86, 3.13, 3,10),
    (255, 0, 128, 1.00,    True, 0.01, 2.66, 2.41),
    (192, 255, 192, 0.01,  True, 3.08, 2.86, 3.12),
    (192, 255, 192, 1.00,  True, 2.22, 0.01, 2.62),
    (128, 255, 128, 0.01,  True, 3.11, 2.86, 3.15),
    (128, 255, 128, 1.00,  True, 2.50, 0.01, 2.92),
    (64, 255, 64, 0.01,    True, 3.12, 2.86, 3.17),
    (64, 255, 64, 1.00,    True, 2.66, 0.01, 3.09),
]

def get_red(r, g, b, brightness):
    if r == 0:
        if g == 1 and b == 1:
            return round(3.13 - (brightness * 100 - 1) * (3.13 - 2.97)/99, 2)
        else:
            return maxV; 
    if g == 1:
        if r == 1:
            return round((3.04 - (brightness * 100 - 1) * (3.04 - 2.56)/99), 2)
        elif r > 0:
            return round((3.11 - (brightness * 100 - 1) * (3.11 - 2.87)/99), 2)
    elif g > 0:
        if b == 1:
            return round((2.86 - (brightness * 100 - 1) * (2.86 - 1.76)/99), 2)
        else:
            return round((2.86 - (brightness * 100 - 1) * (2.86 - 1.77)/99), 2)
    elif b == 1:
        return round((2.88 - (brightness * 100 - 1) * (2.88 - 1.85)/99), 2)
    elif b > 0:
        return round((2.86 - (brightness * 100 - 1) * (2.86 - 1.77)/99), 2)
    else:
        return round((2.95 - (brightness * 100 - 1) * (2.95 - 1.82)/99), 2)
    raise Exception("Shouldn't get here")

def get_green(r, g, b, brightness):
    if g == 0:
        if r == 1 and b == 1:
            return round(3.12 - (brightness * 100 - 1) * (3.12 - 2.92)/99, 2)
        elif r == 1 and b > 0:
            return round(3.12 - (brightness * 100 - 1) * (3.12 - 2.94)/99, 2)
        else:
            return maxV; 
    if r == 1 and g == 1:
        return round((2.86 - (brightness * 100 - 1) * (2.86 - 1.77)/99), 2)
    elif r == 1:
        if b == 1:
            return round((2.93 - (brightness * 100 - 1) * (2.93 - 2.06)/99), 2)
        else:
            return round((2.93 - (brightness * 100 - 1) * (2.93 - 2.10)/99), 2)
    elif r > 0:
        return round((2.86 - (brightness * 100 - 1) * (2.86 - 1.76)/99), 2)
    elif r == 0 and b == 1:
        return round((2.86 - (brightness * 100 - 1) * (2.86 - 1.76)/99), 2)
    else:
        return round((2.95 - (brightness * 100 - 1) * (2.95 - 1.82)/99), 2)
    raise Exception("Shouldn't get here")

def get_blue(r, g, b, brightness):
    if b == 0:
        if r > 0 and g > 0:
            return round((3.17 - (brightness * 100 - 1) * (3.17 - 3.15)/99), 2)
        else:
            return maxV; 
    if r == 1 and g == 1:
        return round((3.07 - (brightness * 100 - 1) * (3.07 - 2.73)/99), 2)
    elif r == 1 and g == 0:
        if b == 1:
            return round((2.86 - (brightness * 100 - 1) * (2.86 - 1.77)/99), 2)
        elif b > 0:
            return round((3.10 - (brightness * 100 - 1) * (3.10 - 2.84)/99), 2)
    elif r == 1 and g > 0:
        if b == 1:
            return round((2.87 - (brightness * 100 - 1) * (2.87 - 1.80)/99), 2)
        else:
            return round((3.10 - (brightness * 100 - 1) * (3.10 - 2.86)/99), 2)
    elif r == 0 and g == 1:
        return round((3.07 - (brightness * 100 - 1) * (3.07 - 2.71)/99), 2)
    else:
        return round((2.95 - (brightness * 100 - 1) * (2.95 - 1.82)/99), 2)
    raise Exception("Shouldn't get here")

def make_rgb_fractions(r, g, b):
    """ These ought to be 0 to 1 on input already, but it won't 
        hurt to make sure they are."""
    m = max(r, g, b)
    r = r/m
    g = g/m
    b = b/m

    return (r, g, b)


for r,g,b,brightness,stateExpected, rVexpected, gVexpected, bVexpected in requirements:
    print(f"IN: {r}, {g}, {b} at {brightness} / ", end="")
    print(f"EXP: ({stateExpected}) {rVexpected},{gVexpected},{bVexpected} ", end="")

    state = True
    if r + g + b == 0 or brightness == 0:
        state = False
        rV = None
        gV = None
        bV = None
    else:
        r, g, b = make_rgb_fractions(r, g, b)
        print(f" [{round(r*255)},{round(g*255)},{round(b*255)}] ", end="")
        rV = get_red(r, g, b, brightness)
        gV = get_green(r, g, b, brightness)
        bV = get_blue(r, g, b, brightness)
    print(f"OUT: ({state}) {rV},{gV},{bV} ", end="")
    if rV == rVexpected and gV == gVexpected and bV == bVexpected and state == stateExpected:
        print(" [PASS]");
    else:
        print()

