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
    (255, 255, 0, 0.01,    True, 3.04, 2.86, 3.17),
    (255, 255, 255, 0.01,  True, 3.04, 2.86, 3.07),
    (0, 255, 0, 0.01,      True, 3.27, 2.95, 3.27),
    (0, 255, 255, 0.01,    True, 3.13, 2.86, 3.07),
    (0, 0, 255, 0.01,      True, 3.27, 3.27, 2.95),
    (255, 0, 255, 0.01,    True, 2.88, 3.12, 2.86),
]

def get_red(r, g, b, brightness):
    if r == 0:
        if g == 1 and b == 1:
            v = round(3.1317 - brightness * 0.16, 2)
            return v
        else:
            return maxV; 
    if g == 1:
        v = round(r * (3.0417 - brightness * 0.48), 2)
    elif b == 1:
        v = round(r * (2.88 - (brightness - 0.01) * (2.88 - 1.85)), 2)
    else:
        v = round(r * (2.95 - (brightness - 0.01) * (2.95 - 1.82)), 2)
        v = round(r * (2.9617 - brightness * 1.13), 2)
    return v

def get_green(r, g, b, brightness):
    if g == 0:
        if r == 1 and b == 1:
            return round(3.12 - (brightness - 0.01) * (3.12 - 2.92), 2)
        else:
            return maxV; 
    if r == 1:
        v = round(g * (2.86 - (brightness - 0.01) * (2.86 - 1.77)), 2)
    elif r == 0 and b == 1:
        v = round(g * (2.86 - (brightness - 0.01) * (2.86 - 1.77)), 2)
    else:
        v = round(g * (2.95 - (brightness - 0.01) * (2.95 - 1.82)), 2)
    return v

def get_blue(r, g, b, brightness):
    if b == 0:
        if r > 0 and g > 0:
            return 3.17
        else:
            return maxV; 
    if r == 1 and g == 1:
        v = round(r * (3.0717 - brightness * 0.48), 2)
    elif r == 1 and g == 0:
        v = round(r * (2.86 - (brightness - 0.01) * (2.86 - 1.77)), 2)
    elif r == 0 and g == 1:
        v = round(g * (3.0717 - brightness * 0.48), 2)
    else:
        v = round(b * (2.95 - (brightness - 0.01) * (2.95 - 1.82)), 2)
    return v

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
        rV = get_red(r, g, b, brightness)
        gV = get_green(r, g, b, brightness)
        bV = get_blue(r, g, b, brightness)
    print(f"OUT: ({state}) {rV},{gV},{bV} ", end="")
    if rV == rVexpected and gV == gVexpected and bV == bVexpected and state == stateExpected:
        print(" [PASS]");
    else:
        print(" [FAIL]");

