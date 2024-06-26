#!/bin/bash
# Compile the program with gcov support
gcc -o ttt -fprofile-arcs -ftest-coverage ttt.c

# Run the program
./ttt 123456789

# Run gcov to generate a coverage report
gcov ttt.c

# Test case 1: Testing a valid input
echo "Test case 1: Valid input"
./ttt 123456789 
1
2
5
8
EOF
diff -s output1.txt expected_output1.txt

# Test case 2: Testing an invalid input
echo "Test case 2: Invalid input"
./ttt 987654321 
$
EOF
diff -s output2.txt expected_output2.txt

# Test case 3: Testing edge case 1
echo "Test case 3: Edge case 1"
./ttt 12312312/111 
3
6
9
EOF
diff -s output3.txt expected_output3.txt

# Test case 4: Testing edge case 2
echo "Test case 4: Edge case 2"
./ttt 321321@# 
1
2
3
EOF
diff -s output4.txt expected_output4.txt

# Test case 5: Testing performance
echo "Test case 5: Performance"
time ./ttt 123456789 
1
2
3
EOF

echo "All tests completed!"