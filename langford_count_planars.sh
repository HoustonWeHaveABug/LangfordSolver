#!/bin/bash
grep ^[0-9] ${1:-/dev/stdin} | sed s/\-[0-9]*//g | sort -u | wc -l
