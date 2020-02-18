#!/usr/bin/env bash

# run script to same path, avoids overwriting saved data
rm -i debug_garb.txt
make && ../task_run.sh task1 exp1.txt debug_garb.txt $1
