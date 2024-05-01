#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

struct my_msgbuf
{
    long mtype;
    int sequence_num;
    int operation_num;
    char name[200];
    char mtext[200];
};

void cleanup(int lb_msqid)
{
    // Inform load balancer to terminate
    struct my_msgbuf lb_buf;
    lb_buf.mtype = 101;
    lb_buf.operation_num = 105; // Termination signal
    if (msgsnd(lb_msqid, &lb_buf, sizeof(struct my_msgbuf) - sizeof(long), 0) == -1)
    {
        perror("msgsnd to load balancer");
        exit(1);
    }

    printf("Cleanup process has informed the load balancer to terminate.\n");

    // Sleep to allow load balancer to inform servers
    sleep(5);

    // Perform any additional cleanup activities if needed
}

int main(void)
{
    key_t lb_key;
    int lb_msqid;

    // Create message queue for load balancer
    if ((lb_key = ftok("loadBalancer.c", 'B')) == -1)
    {
        perror("Error in ftok for load balancer\n");
        exit(1);
    }

    if ((lb_msqid = msgget(lb_key, 0666)) == -1)
    {
        perror("Error in msgget for load balancer\n");
        exit(2);
    }

    char input[10];
    while(1){
        printf("Do you want to initiate cleanup? (yes/no): ");
        fgets(input, sizeof(input), stdin);

        if (strcmp(input, "yes\n") == 0)
        {
            // Run cleanup process
            cleanup(lb_msqid);
            break;
        }
    }
    

    return 0;
}
