# nrsh
Not rsh - An implementation of common shell commands in C over tcp.

## Server & Client
The server and client both have the same dependencies, `cmd.c`, and `file_transfer.c`.
Unlike rsh or ssh, there is actually no shell. Also unlike rsh and ssh, you can run the commands implemented like cp, ls, & rm locally on the client with lcp, lls, & lrm respectively. 
There is also basic get and put functionality similar to ftp.

## Build
Building is straight forward, to build the client run ```gcc client.c cmd.c file_transfer.c -o client```. For the server run ```gcc server.c cmd.c file_transfer.c -o server```

## Why would you do this?
This project was an assignment for my systems programming course. If you wish to see a simple shell in C, check out [shsh](https://github.com/mbags/shsh).
This code was written as a team with [@Emily](https://github.com/emily-c)

