import math

# Available:
#   d = 14
#   a = 4

d = 14
a = 2

nr = 58

# START
pins = d + a
print('PINS: ', pins)

for a in range(1, pins+1):
    b = pins - a
    
    print(a*b)

