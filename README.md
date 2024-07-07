# Operation Systems Ariel Task 2

This task involves simulating a server and client setup similar to NetCat using various networking protocols (TCP, UDP, UDS) and commands. The server can execute a command (such as a tic-tac-toe game) and communicate with clients using different network protocols.

## Prerequisites

- `nc` (netcat) for TCP and UDP communication.
- `socat` for Unix Domain Socket (UDS) datagram communication.
- `gcc` for compilation.

## Compilation

First, compile the project:

```sh
make
```


### TCP Server and Client Communication

#### Example 1: Open a TCP server and wait for input from a client. Output goes to stdout.



**Server:**
```sh
./mync -e "./ttt 123456789" -i TCPS4269
```

**Client:**
```sh
nc localhost 4269
```

#### Example 2: Open a TCP server and wait for input from a client. Output goes to the client.

**Server:**
```sh
./mync -e "./ttt 123456789" -b TCPS4269
```

**Client:**
```sh
nc localhost 4269
```


### Unix Domain Socket Stream Server and Client Communication

#### Open a UDS stream server and wait for input from a client. Output goes to a TCP server listening on port 6699.

**(Output here)TCP Server:**
```sh
nc -l -p 6699
```

**UDS Stream Server & TCP Client:**
```sh
./mync -e "ttt 123456789" -i UDSSShoi.socket -o TCPC127.0.0.1,6699
```

**(Input here)UDS Stream Client:**
```sh
nc -U hoi.socket
```

### Unix Domain Socket Datagram Server and Client Communication

#### Example: UDS datagram server to TCP server

**TCP Server:**
```sh
nc -l -u -p 6699
```

**UDS Datagram Server & TCP Client:**
```sh
./mync -e "ttt 123456789" -i UDSDS6699 -o TCPC127.0.0.1,6699
```

**UDS Datagram Client:**
```sh
socat - UNIX-CONNECT:UDSSThoi.socket
```

### Input from UDS Server and Output to UDS Client (Datagram)

**Open the UDS server (the output will go here):**
```sh
socat UNIX-RECVFROM:hoi1.socket,fork -
```

**Open the UDS client for sending the output, and the UDS server for input:**
```sh
./mync -e "./ttt 123456789" -o UDSCDhoi1.socket -i UDSSDhoi2.socket
```

**Open the UDS client for sending the input:**
```sh
socat - UNIX-SENDTO:hoi2.socket
```

### Unix Domain Socket Stream Server and Client

**Example 1:**
1. Open the UDS stream server:
    ```sh
    nc -lU hoi.socket
    ```
2. Open the UDS stream client:
    ```sh
    ./mync -e "./ttt 123456789" -b UDSCShoi.socket
    ```

**Example 2:**
1. Open the UDS stream server:
    ```sh
    ./mync -e "./ttt 123456789" -b UDSSShoi.socket
    ```
2. Open the UDS stream client:
    ```sh
    nc -U hoi.socket
    ```

## Notes
- Please run the commands as requested and all the examples from the task with combination of the original netcat as a server/client when needed.


## Authors:
- Maor Berenstein
- Daniel Bespalov
