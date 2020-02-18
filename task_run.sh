#!/usr/bin/env bash

die () {
    echo >&2 "$@"
    exit 1
}

executable=$1
inFile=$2
outFile=$3
debug=$4
echo $debug

[ "$#" -ge 3 ] || die "3 arguments required, $# provided."
[ -n "$executable" ] || die "Executable is empty."
[ -n "$inFile" ] || die "Input file is empty."
[ -n "$outFile" ] || die "Output file is empty."

echo "Running \""$executable"\" with:"
echo "Input File: "$inFile
echo "Output File: "$outFile

shopt -s nocasematch

if [[ ! "$debug" =~ ^(debug|dbg|lldb|db) ]]
then
  ./$executable $inFile $outFile && python3 ../visualize.py $inFile $outFile
else
  lldb ./$executable $inFile $outFile
fi

shopt -u nocasematch
