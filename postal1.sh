#!/bin/sh
# POSTAL Launch Script
# Copyright 2017 Declan Hoare
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as published by
# the Free Software Foundation
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
cd `dirname $0`

if [ `uname -m` = "x86_64" ]
then
	BIN="./postal1-x86_64"
else
	BIN="./postal1-x86"
fi

for i in "$@"
do
	if [ "$i" = "--force-x86_64" ]
	then
		BIN="./postal1-x86_64"
	fi
	if [ "$i" = "--force-x86" ]
	then
		BIN="./postal1-x86"
	fi
done

eval "$BIN $@"

