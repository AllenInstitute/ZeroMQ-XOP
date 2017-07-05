#!/bin/sh

# This file is part of the `ZeroMQ-XOP` project and licensed under BSD-3-Clause.

perl xop-stub-generator.pl interface.h
cp functions.* ../src
