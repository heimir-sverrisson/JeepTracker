#!/usr/bin/env python3

import sys
import csv
from xml.dom import minidom


def readFile(filename):
    fields = ['timestamp', 'fix', 'fix_quality',
              'lon', 'lat', 'ele', 'sats', 'speed', 'angle']
    values = []
    with open(filename, errors='ignore') as f:
        reader = csv.DictReader(f, fieldnames=fields)
        for row in reader:
            if(int(row['fix_quality']) > 1):
                values.append(row)
        return values


def writeWaypoint(filename, gpx_name, lat, lon, ele, waypoint_name):
    root = minidom.Document()
    xml = root.createElement('gpx')
    root.appendChild(xml)
    name = root.createElement('name')
    name.appendChild(root.createTextNode(gpx_name))
    xml.appendChild(name)
    waypoint = root.createElement('wpt')
    waypoint.setAttribute('lat', str(lat))
    waypoint.setAttribute('lon', str(lon))
    xml.appendChild(waypoint)
    elevation = root.createElement('ele')
    elevation.appendChild(root.createTextNode(str(ele)))
    waypoint.appendChild(elevation)
    name = root.createElement('name')
    name.appendChild(root.createTextNode(waypoint_name))
    waypoint.appendChild(name)
    xml_str = root.toprettyxml(encoding="utf-8", indent="    ")
    with open(filename, "wb") as f:
        f.write(xml_str)


def writeTrack(filename, gpx_name, gpx_number, values):
    root = minidom.Document()
    xml = root.createElement('gpx')
    root.appendChild(xml)
    trk = root.createElement('trk')
    name = root.createElement('name')
    name.appendChild(root.createTextNode(gpx_name))
    trk.appendChild(name)
    number = root.createElement('number')
    number.appendChild(root.createTextNode(str(gpx_number)))
    trk.appendChild(number)
    trkseg = root.createElement('trkseg')
    for v in values:
        trkpt = root.createElement('trkpt')
        trkpt.setAttribute('lat', v['lat'])
        trkpt.setAttribute('lon', v['lon'])
        elevation = root.createElement('ele')
        elevation.appendChild(root.createTextNode(v['ele']))
        trkpt.appendChild(elevation)
        time = root.createElement('time')
        time.appendChild(root.createTextNode(v['timestamp']))
        trkpt.appendChild(time)
        trkseg.appendChild(trkpt)
    trk.appendChild(trkseg)
    xml.appendChild(trk)
    xml_str = root.toprettyxml(encoding="utf-8", indent="    ")
    with open(filename, "wb") as f:
        f.write(xml_str)


def main(argv):
    if len(argv) < 3:
        print("Usage:", file=sys.stderr)
        print("\t" + argv[0] + ": input-file output-file")
        exit(1)
    values = readFile(argv[1])
    # writeWaypoint(argv[2], 'JeepTracker', 40.042, -105.0105, 1609, 'Boulder Colorado')
    writeTrack(argv[2], 'JeepTracker', 1, values)


if __name__ == "__main__":
    main(sys.argv)
