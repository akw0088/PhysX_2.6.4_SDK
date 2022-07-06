#!/bin/sh

for i in `find . -name Makefile | grep linux | grep -v "IV" | sed -e "s/\ /#/g"`
do
	DIR=`dirname $i | sed -e "s/#/\ /g"`
	BUILD=release make -C "$DIR" -s
done


