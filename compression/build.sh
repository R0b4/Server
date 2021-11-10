gcc -c */*.c -Wall
gcc -c *.cpp -Wall
ar rvs zlib.a *.o
rm *.o