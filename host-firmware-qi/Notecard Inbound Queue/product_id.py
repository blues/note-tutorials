import configparser
c = configparser.ConfigParser()
r = c.read('notecard.ini')

if len(r) == 0:
    exit()

p = c['NOTECARD']['ProductId']

if (' ' in p):
    raise ValueError('Product ID in notecard.ini file contains spaces.')

print("-DPRODUCT_ID=\\\"{0}\\\"".format(p))