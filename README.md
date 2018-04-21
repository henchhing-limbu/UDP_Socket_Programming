# UDP_Socket_Programming
* Client-server architecture is used for the network application. 
* Server is given an IP address which does not change throughout the session. 
* They don’t need to establish connection before they can communicate. 
* There is no handshaking between UDP sender and receiver. Each UDP segment is handled independently of others. 
* All the network communication uses UDP with possible segment loss involved. Therefore, the stop-and-wait protocol is implemented on both client and server side to have data reliably transferred. 
* The time out for this protocol is 3 seconds and maximum tries is 500. 
* Client requests services from the server and server responds to it.