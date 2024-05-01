# Load-Balancing-Graph-Related-Query

## Problem Statement
In this assignment, you will simulate an application for a distributed graph database system involving the following components:

### Components
1. **Load Balancer Process**
2. **Primary Server Process**
3. **Two Secondary Server Processes (Secondary Server 1 and Secondary Server 2)**
4. **Cleanup Process**
5. **Client Processes**

### Overall Workflow
- Clients send requests (read or write) to the Load Balancer.
- Load Balancer forwards write requests to the Primary Server.
- Load Balancer forwards odd-numbered read requests to Secondary Server 1 and even-numbered read requests to Secondary Server 2.
- Servers create a new thread to process the request and send the output back to the client.
- Cleanup Process informs the Load Balancer to terminate the application.
- Load Balancer performs cleanup, informs servers to exit, and terminates itself.
- Servers perform cleanup and terminate.

### Graph Database
- The database can contain cyclic and acyclic graphs.
- Graphs are unweighted, undirected, and may contain self-loops.
- Each graph is represented as a text file named `Gx.txt` (e.g., `G1.txt`).
- The first line of the file contains the number of nodes `n` (≤ 30).
- The next `n` lines represent the adjacency matrix of the graph.

### Client Process
- Clients send requests to the Load Balancer via a single message queue.
- Clients have the following menu options:
 1. Add a new graph to the database (write operation)
 2. Modify an existing graph (write operation)
 3. Perform DFS on an existing graph (read operation)
 4. Perform BFS on an existing graph (read operation)
- Clients send requests in the format: `<Sequence_Number Operation_Number Graph_File_Name>`
- For write operations, clients write the graph information to a shared memory segment.
- For read operations, clients write the starting vertex to a shared memory segment.
- Clients receive output from the servers via the message queue and display it on the console.

### Load Balancer
- Receives client requests via the message queue.
- Sends odd-numbered requests to Secondary Server 1 and even-numbered requests to Secondary Server 2.

### Primary Server
- Receives write requests from the Load Balancer.
- Creates a new thread to handle each request.
- Thread reads the graph information from the shared memory segment, modifies the graph file, and sends a success message to the client.

### Secondary Server
- Receives read requests from the Load Balancer.
- Creates a new thread to handle each request.
- For DFS, creates a new thread for each unvisited node, ensuring depth-wise traversal.
- For BFS, processes levels serially and nodes within a level concurrently.
- Sends the traversal output to the client via the message queue.

### Cleanup Process
- Displays a menu to terminate the application.
- Informs the Load Balancer to terminate.
- Load Balancer informs servers to terminate and performs cleanup.

### Handling Concurrent Requests
- Multiple read operations can be performed on the same graph file simultaneously.
- Simultaneous write operations and read-write operations on the same graph file must be performed serially using semaphores or mutexes.

## Notes
- Implement using POSIX-compliant C programs on Ubuntu 22.04.
-  The functionality of secondary server has been implemented using a single file, open 2 different terminals and run the program. Both the secondary servers will act as 2 different secondary serevr.
- Used a single message queue for all communication.
- Shared memory segments are created by clients for each request.
