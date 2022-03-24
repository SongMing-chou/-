g++ -c msg.cpp 
g++ -c publisher_test.cpp 
g++ -o publisher_test publisher_test.o msg.o -lpthread