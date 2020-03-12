#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../mqsemaphore.h"

int main(){
    int msqid; 

    key_t key; 
    int len; 
    pid_t pid = getpid();
    
    
    struct request request_buf;
    
    struct response response_buf;

    if((key = ftok(pathname,0)) < 0){
        printf("Can\'t generate key\n");
        exit(-1);
    }

    if((msqid = msgget(key, 0666 | IPC_CREAT)) < 0){
        printf("Can\'t get msqid\n");
        exit(-1);
    } 

    request_buf.m_type = SEMAPHORE_MSG;
    request_buf.id = pid;
    request_buf.sem_value = -1;
    
    if (msgsnd(msqid, (struct request *) &request_buf, sizeof(request_buf), 0) < 0){
        printf("Can\'t send message to queue\n");
        msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
        exit(-1);
    }
    
    if(( len = msgrcv(msqid, (struct response *) &response_buf, sizeof(response_buf), pid, 0) < 0)){
        printf("Can\'t receive message from queue\n");
        exit(-1);
    }

    printf("condition is present\n");

    return 0;
}  
