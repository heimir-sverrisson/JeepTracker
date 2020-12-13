#!/usr/bin/env python3

import sys
import csv
import re


def readFile(filename):
    fields = ['timestamp', 'fix', 'fix_quality',
              'lon', 'lat', 'ele', 'satspeed', 'angle']
    values = []
    with open(filename, errors='ignore') as f:
        reader = csv.DictReader(f, fieldnames=fields)
        for row in reader:
            if(int(row['fix_quality']) > 1):
                values.append(row)
        return values


def writeFixedFile(filename, values):
    fields = ['timestamp', 'fix', 'fix_quality',
              'lon', 'lat', 'ele', 'sats', 'speed', 'angle']
    with open(filename, "w") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        for v in values:
            satspeed = v['satspeed']
            if re.match(r'^[023456789]', satspeed):
                v['sats'] = satspeed[0]
                v['speed'] = satspeed[1:]
            else:
                v['sats'] = satspeed[0:2]
                v['speed'] = satspeed[2:]
            del v['satspeed']
            # print(satspeed, v['sats'], v['speed'])
            writer.writerow(v)


def main(argv):
    if len(argv) < 3:
        print("Usage:", file=sys.stderr)
        print("\t" + argv[0] + ": input-file output-file")
        exit(1)
    values = readFile(argv[1])
    writeFixedFile(argv[2], values)


if __name__ == "__main__":
    main(sys.argv)
