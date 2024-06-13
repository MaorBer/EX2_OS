#!/bin/bash

# Compile the program with gcov support
gcc -fprofile-arcs -ftest-coverage mync.c -o mync

# Run the tests
# Test 1
./mync -e "ttt 123456789" -i TCPS5270 &
nc localhost 5270

# Test 2
./mync -e "ttt 123456789" -b TCPS5270 &
nc localhost 5270

# Test 3
nc -l -p 5272 &
./mync -e "ttt 123456789" -i UDPS5271 -o TCPClocalhost,5272 &
nc -u localhost 5271

# Test 4
nc -l -p 5272 &
./mync -e "ttt 123456789" -i UDSSShoi.socket -o TCPClocalhost,5272 &
nc -U hoi.socket

# Test 5
./mync -e "ttt 123456789" -i UDSSDhoi.socket -o UDPS5271 &
socat - UNIX-SENDTO:hoi.socket &
nc -u localhost 5271

# Test 6
./mync -b TCPS5270 &
./mync -b TCPClocalhost,5270

# Test 7
socat UNIX-RECVFROM:hoi1.socket,fork - &
./mync -e "ttt 123456789" -o UDSCDhoi1.socket -i UDSSDhoi2.socket &
socat - UNIX-SENDTO:hoi2.socket

# Test 8
nc -lU hoi.socket &
./mync -e "ttt 123456789" -b UDSCShoi.socket

# Test 9
./mync -e "ttt 123456789" -b UDSSShoi.socket &
nc -U hoi.socket

# Run gcov on the program
gcov mync.c