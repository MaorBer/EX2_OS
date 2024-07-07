
# mync README

This project provides a way to simulate a server and client setup using various networking protocols (TCP, UDP, UDS) and commands. The tool allows you to run a command (such as a tic-tac-toe game) on the server and communicate with it using network clients.

## Prerequisites
- `nc` (netcat) for TCP and UDP communication.
- `socat` for Unix Domain Socket (UDS) datagram communication.
- A file named `run_commands` containing all the executable commands.

## Example Usage

In all of the examples below, when we use the `-e` flag, we execute a command (in this case, a tic-tac-toe game that we implemented). We simulate a server & client with the `nc` command, and for the Unix Domain Socket datagram, we use the `socat` command.

### Commands Overview

All of the commands that you can use are in the `run_commands` file.

### TCP Server and Client Communication

#### Open a TCP server and wait for input from a client. Output goes to stdout.

**Server:**
```sh
./mync -e "./ttt 123456789" -i TCPS4269
```

**Client:**
```sh
nc localhost 4269
```

#### Open a TCP server and wait for input from a client. Output goes to client.

**Server:**
```sh
./mync -e "./ttt 123456789" -b TCPS4269
```

**Client:**
```sh
nc localhost 4269
```

### UDP Server and Client Communication

#### Open a UDP server and wait for input from a client. Output goes to a TCP server listening on port 6699.

**TCP Server:**
```sh
nc -l -p 6699
```

**UDP Server & TCP Client:**
```sh
./mync -e "./ttt 123456789" -i UDPS4269 -o TCPClocalhost,6699
```

**UDP Client:**
```sh
nc -u localhost 4269
```

### Unix Domain Socket Stream Server and Client Communication

#### Open a UDS stream server and wait for input from a client. Output goes to a TCP server listening on port 6699.

**TCP Server:**
```sh
nc -l -p 6699
```

**UDS Stream Server & TCP Client:**
```sh
./mync -e "./ttt 123456789" -i UDSSShoi.socket -o TCPClocalhost,6699
```

**UDS Stream Client:**
```sh
nc -U hoi.socket
```

### Unix Domain Socket Datagram Server and Client Communication

#### Open a UDS datagram server and UDP server. The input is from the UDS client and the output goes to the UDP client.

**Servers:**
```sh
./mync -e "./ttt 123456789" -i UDSSDhoi.socket -o UDPS4269
```

**UDS Client:**
```sh
socat - UNIX-SENDTO:hoi.socket
```

**UDP Client:**
```sh
nc -u localhost 4269
```

### Chat Between Two Terminals

**Terminal 1:**
```sh
./mync -b TCPS4269
```

**Terminal 2:**
```sh
./mync -b TCPClocalhost,4269
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
- Replace `"./ttt 123456789"` with the actual command you want to run.
- Ensure the `run_commands` file is properly set up with all the necessary commands.
Authors:
Maor Berenstein
Daniel Bespalov
