server.c is a simple echo server. It just returns what ever is written into it.  Usage: `./server`

client.c is something special. usage is ./client [`<client-no=1>` `<client-name=client-no>` `<server-port=50500>` `<server-ip=127.0.0.1>`]

We have a message queue, which exists only when a client is waiting/is currently talking with server. 

Let's say at start, a client with client-no=2 is ran, as ./client 2(lets call this client as **client2**). As there is no message queue, now it creates a message queue, and makes a connection with the server.

Note: Any client with any client-no (not just 1) can connect with the server as long as there is no message queue present. 

Now when a new client comes (lets say we ran ./client 3)(let the name be **client3**). It checks whether there is message queue, and it finds a message queue, so it posts it usdPathName as message in the message queue, and waits to accept a connection.

Now when **client2** notices that **client3** is waiting, by recieving a message form the message queue, it notices the message's usdPathName, and calls for *connect.* And **client2** passes the fd, using *send_fd,* now **client3** recieves the fd using *recv_fd* . From now on **client3** talks with the server.

After passing the fd **client2** prompts the user to enter anything to again post a request to get the connection, or enter `exit` to exit from the program. Before posting a request again, it checks whether a message queue exists, if present then it posts a request, just like before. If it doesnot exists, it makes a new connection with the server.

**client3** is now just like how **client2** was before. 

Note: run `./delete_queue ess-mq` at the start, to make sure the message queue is deleted, before running the first client.
