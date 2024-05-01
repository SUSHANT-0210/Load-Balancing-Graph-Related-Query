#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int w;

struct my_msgbuf
{   
    long mtype;
    int sequence_num;
    int operation_num;
    char name[200];
    char mtext[200];
};


void handleClientRequest(struct my_msgbuf msg, int msqid)
{
    int sq_num = msg.sequence_num;
    int service = msg.operation_num;

    struct my_msgbuf buf_new; // Declare buf_new once
    printf("%d",sq_num);
    switch (service)
    {
    case 1:
        buf_new.sequence_num = sq_num;
        buf_new.operation_num = service;
        buf_new.mtype = 102; // Primary Server message type
        strcpy(buf_new.mtext, msg.mtext);
        strcpy(buf_new.name, msg.name);
        if (msgsnd(msqid, &buf_new, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
        {
            perror("msgsnd");
        }
        break;

    case 2:
        buf_new.sequence_num = sq_num;
        buf_new.operation_num = service;
        buf_new.mtype = 102; // Primary Server message type
        strcpy(buf_new.mtext, msg.mtext);
        strcpy(buf_new.name, msg.name);
        if (msgsnd(msqid, &buf_new, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
        {
            perror("msgsnd");
        }
        break;

    case 3:
        if (sq_num % 2 == 0)
        {
            buf_new.sequence_num = sq_num;
            buf_new.operation_num = service;
            buf_new.mtype = 103; // Secondary Server 1 message type
            strcpy(buf_new.mtext, msg.mtext);
            strcpy(buf_new.name, msg.name);
            if (msgsnd(msqid, &buf_new, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
            }
        }
        else
        {
            buf_new.sequence_num = sq_num;
            buf_new.operation_num = service;
            buf_new.mtype = 104; 
            strcpy(buf_new.mtext, msg.mtext);
            strcpy(buf_new.name, msg.name);
            if (msgsnd(msqid, &buf_new, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
            }
        }
        break;

    case 4:
        if (sq_num % 2 == 0)
        {
            buf_new.sequence_num = sq_num;
            buf_new.operation_num = service;
            buf_new.mtype = 103; 
            strcpy(buf_new.mtext, msg.mtext);
            strcpy(buf_new.name, msg.name);
            if (msgsnd(msqid, &buf_new, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
            }
        }
        else
        {
            buf_new.sequence_num = sq_num;
            buf_new.operation_num = service;
            buf_new.mtype = 104; 
            strcpy(buf_new.mtext, msg.mtext);
            strcpy(buf_new.name, msg.name);
            if (msgsnd(msqid, &buf_new, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
            }
        }
        break;
        case 1000 : 
        buf_new.sequence_num = sq_num;
        buf_new.operation_num = w;
        w++;
        buf_new.mtype = 500; 
        strcpy(buf_new.mtext, msg.mtext);
        strcpy(buf_new.name, msg.name);
        if (msgsnd(msqid, &buf_new, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
        {
            perror("msgsnd");
        }
        break;
    default:
        printf("Invalid service option: %d\n", service);
    }
}

int main(void)
{

    key_t key;
    int msqid;
    w=103;
    // Create message queue
    if ((key = ftok("loadBalancer.c", 'B')) == -1)
    {
        perror("Error in ftok\n");
        exit(1);
    }

    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1)
    {
        perror("Error in msgget\n");
        exit(2);
    }


    while (1)
{
    struct my_msgbuf buf;
    buf.mtype = 101;
    if (msgrcv(msqid, &buf, sizeof(struct my_msgbuf) - sizeof(long), buf.mtype , 0) == -1)
    {
        //perror("Error in msgrcv\n");
        exit(1);
    }

    printf("%d %ld\n", buf.sequence_num,strlen(buf.name));

    if (buf.operation_num == 105)
    {
        if (msgctl(msqid, IPC_RMID, NULL) == -1)
        {
            perror("msgctl");
            exit(1);
        }
        printf("Main server terminated gracefully.\n");
        exit(0);
    }
    else
    {
        handleClientRequest(buf, msqid);
    }
}

skip:
    return 0;
}