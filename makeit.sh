#!/bin/sh
# POSTAL Makeit
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

# Bit confusing, definitely not as flexible as a configure script,
# but it gets the job done. This script handles the BINDIR for the
# target architecture, because make can't change that itself for some
# reason. It also changes it for debug vs release to make that easier,
# although it's not quite as important there.

bindir="./bin"

if [ `uname -m` = "x86_64" ]
then
	bindir1="64"
else
	x86="yes"
fi

bindir2="-release"
for i in "$@"
do
	if [ "$i" = "x86" ]
	then
		x86="yes"
	fi
	if [ "$i" = "debug" ]
	then
		after="debug"
		bindir2="-debug"
	fi
	if [ "$i" = "clean" ]
	then
		remove="yes"
	fi
	if [ "$i" = "smp" ]
	then
		thread=$(cat /proc/cpuinfo | awk '/^processor/{print $3}' | tail -1)
		thread=$(( $thread + 2 ))
		thread="-j$thread"
	fi
done

if [ "$x86" = "yes" ]
then
	before="linux_x86=\"1\""
	bindir1="32"
fi

if [ "$remove" = "yes" ]
then
	after="clean"
	thread=""
fi

bindir="$bindir$bindir1$bindir2"
eval "BINDIR=$bindir $before make -e $after $thread"
