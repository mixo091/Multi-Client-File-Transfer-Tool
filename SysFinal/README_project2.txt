
Michalis Michopoulos sdi1700091


//Compile Server
make clean
make 

// Run server Examples
./dataserver -s 10 -q 100 -p 8096 -b 2048
./dataserver -s 20 -q 60 -p 8080 -b 1024

//Compile Client
g++ client.cpp -o client

//I have dummy dirs TestDir1 TestDir2 TestDir3 (Dwste sxetiko monopati px ../TestDir1 me bash to location tou dataserver.cpp

// Run Clients


./client -i <ip> -p 8096 -d ../TestDir1
./client -i <ip> -p 8096 -d ../TestDir2
./client -i <ip> -p 8096 -d ../TestDir3

//Stop Clients and Server using CRL C  in each console



