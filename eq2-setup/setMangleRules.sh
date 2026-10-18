#!/bin/sh

# Clear the rules
iptables -F
iptables -t mangle -F


iptables -t mangle -A FORWARD -s 192.168.42.0/24 -d 192.168.40.0/24 -j DSCP --set-dscp-class CS1
iptables -t mangle -A FORWARD -s 192.168.43.0/24 -d 192.168.40.0/24 -j DSCP --set-dscp-class CS4
