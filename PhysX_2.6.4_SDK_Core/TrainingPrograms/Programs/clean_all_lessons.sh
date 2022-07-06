#!/bin/sh

for i in `find . -name Makefile | grep linux | grep -v "IV" | sed -e "s/\ /#/g"`
do
	DIR=`dirname $i | sed -e "s/#/\ /g"`
	make -C "$DIR" clean -s
done

rm -f ../../Bin/linux/release/Lesson* ../../Bin/linux/debug/Lesson*


