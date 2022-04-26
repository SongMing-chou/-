g++ -c msg.cpp 
g++ -c consumer_test.cpp 
g++ -o consumer_test consumer_test.o msg.o -lpthread && rm consumer_test.o msg.o