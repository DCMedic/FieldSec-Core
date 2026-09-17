#!/usr/bin/env python3
"""Convert FieldSec Developer Board FS1/FS2 WLAN records to CSV."""
import csv
import sys

writer = csv.writer(sys.stdout)
writer.writerow(["bssid", "rssi", "channel", "auth", "ssid"])
for raw in sys.stdin:
    parts = raw.rstrip("\r\n").split("|", 6)
    if len(parts) == 7 and parts[0] in {"FS1", "FS2"} and parts[1] == "AP":
        writer.writerow(parts[2:])
