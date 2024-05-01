#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>

int sqn;

#define MAX_NODES 30
struct my_msgbuf
{
    long mtype;
    int sequence_num;
    int operation_num;
    char name[200];
    char mtext[200];
};

struct GraphData
{
        int numNodes;
        int adjacencyMatrix[MAX_NODES][MAX_NODES];
};

struct graph_vertex
{
    int ver;
};



sem_t mutex;

void intArrayToCharArray(const int *intArray, char *charArray, int size) {
    for (int i = 0; i < size; ++i) {
        // Convert each digit to a character and store it in the char array
        charArray[i] = intArray[i] + '0';
    }
}

//FOR DFS THE REQUIRED CODE
void readGraphDataFromFile(const char *filename, struct GraphData *graph)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Read the number of nodes
    fscanf(file, "%d", &graph->numNodes);

    // Read the adjacency matrix
    for (int i = 0; i < graph->numNodes; ++i)
    {
        for (int j = 0; j < graph->numNodes; ++j)
        {
            fscanf(file, "%d", &graph->adjacencyMatrix[i][j]);
        }
    }

    fclose(file);
}

void printGraphData(const struct GraphData *graph)
{
    printf("Number of Nodes: %d\n", graph->numNodes);

    printf("Adjacency Matrix:\n");
    for (int i = 0; i < graph->numNodes; ++i)
    {
        for (int j = 0; j < graph->numNodes; ++j)
        {
            printf("%d ", graph->adjacencyMatrix[i][j]);
        }
        printf("\n");
    }
}
struct ThreadData1
{
    const struct GraphData *graph;
    int currentVertex;
    bool *visited;
    int *traversalPath;
    int pathSize;
};

void *dfsThread(void *arg)
{
    struct ThreadData1 *threadData = (struct ThreadData1 *)arg;
    int currentVertex = threadData->currentVertex;
    bool *visited = threadData->visited;

    printf("%d ", currentVertex);

    // Append the current vertex to the traversal path array
    threadData->traversalPath[threadData->pathSize++] = currentVertex;

    // Create a new visited array for the current thread
    bool *newVisited = malloc(threadData->graph->numNodes * sizeof(bool));
    if (newVisited == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < threadData->graph->numNodes; ++i)
    {
        newVisited[i] = visited[i];
    }
    newVisited[currentVertex] = true;

    for (int i = 0; i < threadData->graph->numNodes; ++i)
    {
        if (threadData->graph->adjacencyMatrix[currentVertex][i] == 1 && !newVisited[i])
        {
            // Create a new thread for the unvisited neighbor
            struct ThreadData1 newThreadData = {.graph = threadData->graph,
                                               .currentVertex = i,
                                               .visited = newVisited,
                                               .traversalPath = threadData->traversalPath,
                                               .pathSize = threadData->pathSize};
            // Recursively call dfsThread
            dfsThread(&newThreadData);

            // Update path size after the thread has finished
            threadData->pathSize = newThreadData.pathSize;
        }
    }

    free(newVisited);  // Free the memory allocated for the newVisited array
    pthread_exit(NULL);
}

void dfs(const struct GraphData *graph, int startVertex, int *arr)
{
    bool visited[MAX_NODES] = {false};

    printf("DFS Traversal starting from vertex %d: ", startVertex);

    // Create a thread for the starting vertex
    struct ThreadData1 threadData = {.graph = graph,
                                    .currentVertex = startVertex,
                                    .visited = visited,
                                    .traversalPath = arr,
                                    .pathSize = 0};
    pthread_t thread;
    pthread_create(&thread, NULL, dfsThread, &threadData);

    // Wait for the thread to finish
    pthread_join(thread, NULL);

    printf("\n");
}
//THE DFS CODE ENDS HERE

//BFS CODE BEGINS HERE
struct Queue
{
    int front, rear, capacity;
    int *array;
    pthread_mutex_t mutex;
};

struct GraphData1
{
    int numNodes;
    int adjacencyMatrix[MAX_NODES][MAX_NODES];
    struct Queue *queue;  // Queue for BFS
    pthread_mutex_t mutex;  // Mutex for BFS operations
};

void readGraphDataFromFile1(const char *filename, struct GraphData1 *graph)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Read the number of nodes
    fscanf(file, "%d", &graph->numNodes);

    // Read the adjacency matrix
    for (int i = 0; i < graph->numNodes; ++i)
    {
        for (int j = 0; j < graph->numNodes; ++j)
        {
            fscanf(file, "%d", &graph->adjacencyMatrix[i][j]);
        }
    }

    fclose(file);
}

void printGraphData1(const struct GraphData1 *graph)
{
    printf("Number of Nodes: %d\n", graph->numNodes);

    printf("Adjacency Matrix:\n");
    for (int i = 0; i < graph->numNodes; ++i)
    {
        for (int j = 0; j < graph->numNodes; ++j)
        {
            printf("%d ", graph->adjacencyMatrix[i][j]);
        }
        printf("\n");
    }
}

struct ThreadData2
{
    const struct GraphData1 *graph;
    int currentVertex;
    bool *visited;
    int *arrx;
    int pathSize;
};

struct Queue *createQueue(int capacity)
{
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->rear = -1;
    queue->array = (int *)malloc(queue->capacity * sizeof(int));
    pthread_mutex_init(&queue->mutex, NULL);
    return queue;
}

void enqueue(struct Queue *queue, int item)
{
    pthread_mutex_lock(&queue->mutex);

    if (queue->rear == queue->capacity - 1)
    {
        fprintf(stderr, "Queue is full.\n");
        exit(EXIT_FAILURE);
    }
    if (queue->front == -1)
        queue->front = 0;

    queue->rear++;
    queue->array[queue->rear] = item;

    pthread_mutex_unlock(&queue->mutex);
}

int dequeue(struct Queue *queue)
{
    pthread_mutex_lock(&queue->mutex);

    if (queue->front == -1)
    {
        fprintf(stderr, "Queue is empty.\n");
        exit(EXIT_FAILURE);
    }

    int item = queue->array[queue->front];
    queue->front++;

    if (queue->front > queue->rear)
        queue->front = queue->rear = -1;

    pthread_mutex_unlock(&queue->mutex);

    return item;
}

bool isQueueEmpty(struct Queue *queue)
{
    return (queue->front == -1);
}

void *bfsThread(void *arg)
{
    struct ThreadData2 *threadData = (struct ThreadData2 *)arg;
    int currentVertex;

    while (1)
    {
        pthread_mutex_lock(&threadData->graph->mutex);

        if (isQueueEmpty(threadData->graph->queue))
        {
            pthread_mutex_unlock(&threadData->graph->mutex);
            break;
        }

        currentVertex = dequeue(threadData->graph->queue);
        pthread_mutex_unlock(&threadData->graph->mutex);

        // Check if the vertex has been visited before
        if (!threadData->visited[currentVertex])
        {
            // Append the current vertex to the traversal path array
            threadData->arrx[threadData->pathSize++] = currentVertex;
            threadData->visited[currentVertex] = true;

            for (int i = 0; i < threadData->graph->numNodes; ++i)
            {
                if (threadData->graph->adjacencyMatrix[currentVertex][i] == 1 && !threadData->visited[i])
                {
                    pthread_mutex_lock(&threadData->graph->mutex);
                    enqueue(threadData->graph->queue, i);
                    pthread_mutex_unlock(&threadData->graph->mutex);
                }
            }
        }
    }

    pthread_exit(NULL);
}

void bfs(const struct GraphData1 *graph, int startVertex, int *arrx)
{
    bool visited[MAX_NODES] = {false};
    struct ThreadData2 threadData = {.graph = graph,
                                    .currentVertex = startVertex,
                                    .visited = visited,
                                    .arrx = arrx,
                                    .pathSize = 0};

    printf("BFS Traversal starting from vertex %d: ", startVertex);

    // Create a thread for the starting vertex
    pthread_t thread;
    pthread_create(&thread, NULL, bfsThread, &threadData);

    // Wait for the thread to finish
    pthread_join(thread, NULL);

    printf("\n");
}

//BFS CODE ENDS HERE

void *handleRequest(void *arg)
{
        struct my_msgbuf *msg = (struct my_msgbuf *)arg;
        int sequenceNumber = msg->sequence_num;
        int operationNumber = msg->operation_num;
        char graphFileName[200];
        strcpy(graphFileName, msg->name);

        key_t key = sequenceNumber+10000;
        if (key == -1)
        {
            perror("Error in ftok\n");
            pthread_exit(NULL);
        }

        int shmid = shmget(key, sizeof(struct graph_vertex), 0666 | IPC_CREAT);
        if (shmid == -1)
        {
            perror("Error in shmget\n");
            pthread_exit(NULL);
        }

        struct graph_vertex *sharedGraphData = shmat(shmid, NULL, 0);
        if (sharedGraphData == (void *)-1)
        {
            perror("Error in shmat\n");
            pthread_exit(NULL);
        }
        
        char charArray[30 + 1];

        if(operationNumber==3)
        {   
                 struct GraphData graph;
                readGraphDataFromFile(graphFileName, &graph);
                printGraphData(&graph);
                int arr[graph.numNodes];
            int startVertex=sharedGraphData->ver;
            if (startVertex < 0 || startVertex >= graph.numNodes)
            {
                fprintf(stderr, "Invalid starting vertex.\n");
                exit(EXIT_FAILURE);
            }
             dfs(&graph, startVertex,arr);
                 printf("DFS Traversal path: ");
            for (int i = 0; i < graph.numNodes; ++i)
            {
                printf("%d ", arr[i]);
            }
            printf("\n");
            // Convert the integers to characters
             intArrayToCharArray(arr, charArray, graph.numNodes);

            // Null-terminate the char array
            charArray[graph.numNodes] = '\0';
        }
        else if(operationNumber==4)
        {
            struct GraphData1 graph;
            readGraphDataFromFile1(graphFileName, &graph);
            printGraphData1(&graph);
            printf("\n");
            int startVertex=sharedGraphData->ver;
            if (startVertex < 0 || startVertex >= graph.numNodes)
            {
                fprintf(stderr, "Invalid starting vertex.\n");
                exit(EXIT_FAILURE);
            }

            // Create a queue and mutex for BFS
            graph.queue = createQueue(100);
            pthread_mutex_init(&graph.mutex, NULL);

            // Initialize the queue with the starting vertex
            enqueue(graph.queue, startVertex);
            int arrx[100];
            
            bfs(&graph, startVertex,arrx);
            printf("BFS Traversal path: ");
            for (int i = 0; i < graph.numNodes; ++i)
            {
                printf("%d ", arrx[i]);
            }
            printf("\n");

            // Clean up resources
            free(graph.queue->array);
            free(graph.queue);
            pthread_mutex_destroy(&graph.mutex);
            
            intArrayToCharArray(arrx, charArray, graph.numNodes);

            // Null-terminate the char array
            charArray[graph.numNodes] = '\0';
        }
        //FOR SENDING DATA TO CLIENT
        key_t keyx = ftok("loadBalancer.c", 'B');
        if (keyx == -1)
        {
            perror("Error in ftok\n");
            pthread_exit(NULL);
        }

        int msgid = msgget(keyx, 0666);
        if (msgid == -1)
        {
            perror("Error in msgget\n");
            pthread_exit(NULL);
        }

        struct my_msgbuf response;
        response.mtype = sequenceNumber;
        response.sequence_num = sequenceNumber;
        response.operation_num = operationNumber;

        strcpy(response.mtext,charArray);

        if (msgsnd(msgid, &response, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
        {
            perror("msgsnd");
            pthread_exit(NULL);
        }
        else
    {
        perror("Error opening graph file\n");
    }

    shmdt(sharedGraphData);
    pthread_exit(NULL);
}
    


int main()
{
    key_t key;

    if ((key = ftok("loadBalancer.c", 'B')) == -1)
    {
        perror("Error in ftok\n");
        exit(1);
    }

    int msgid;
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1)
    {
        perror("Error in msgget\n");
        exit(2);
    }

    struct my_msgbuf bufw;
    bufw.mtype=101;
    bufw.operation_num=1000;
    if (msgsnd(msgid, &bufw, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
    {
        perror("msgsnd");
    }

    struct my_msgbuf bufp;

    bufp.mtype = 500;
    if (msgrcv(msgid, &bufp, sizeof(struct my_msgbuf) - sizeof(long), bufp.mtype, 0) == -1)
    {
        //perror("Error in msgrcv\n");
        exit(1);
    }
    int y=bufp.operation_num;

    sem_init(&mutex, 0, 1);
    int flag=0;
    while (1)
    {
        struct my_msgbuf buf;

            buf.mtype = y;
            if (msgrcv(msgid, &buf, sizeof(struct my_msgbuf) - sizeof(long), buf.mtype, 0) == -1)
            {
               // perror("Error in msgrcv\n");
                exit(1);
            }
        

        pthread_t tid;
        if (pthread_create(&tid, NULL, handleRequest, (void *)&buf) != 0)
        {
            perror("Error creating thread\n");
            exit(1);
        }
    }

    sem_destroy(&mutex);

    return 0;
}
