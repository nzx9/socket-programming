# socket-programming
---

## What is a Socket?
- Socket is a way to speak to other programs using standard Unix file descriptors.

### Types of Internet Socket
---

- Stream Sockets   (`SOCK_STREAM`) => Two Way Connected Sockets [TCP/IP]     
    ex: Telnet, HTTP     
   *IP(Internet Protocol) => Deals with Internet routing and not generally responsible for data integrity.*

    
- Datagram Sockets (`SOCK_DGRAM`)  => Connectionless Sockets [UDP]    
 ex: tftp(trivial file transfer protocol), dhcpcd (DHCP client), multiplayer games, audio streaming, video conferencing     
 *this also uses IP for routing*   
 *Connectionless*
 


**(More than two but only consider these two)**

### Low level and Network Theory
---

#### Data Encapsulation
![data encapsulation](https://beej.us/guide/bgnet/html/dataencap.svg)

#### Layered Model
- Application Layer (telnet, ftp)
- Host to Host Transport Layer (TCP, UDP)
- Internet Layer (IP and routing)
- Network Access Layer (Ethernet, wi-fi)