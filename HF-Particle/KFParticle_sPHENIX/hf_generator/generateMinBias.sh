#!/bin/bash

source /opt/sphenix/core/bin/sphenix_setup.sh -n new

root.exe -q -b Fun4All_MinBias.C\(\"$1\"\)
echo Script done
