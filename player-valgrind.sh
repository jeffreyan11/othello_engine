#!/bin/bash

valgrind --log-file=valgrind.log --leak-check=full ./player $*
