#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include "mqsemaphore.h"

int create_mq(const char key_path[]){
    key_t key = ftok(pathname, 0);
    if(key < 0){
        printf("Can\'t generate key\n");
        exit(-1);
    }
    
    int msqid = msgget(key, 0666 | IPC_CREAT);
    if(msqid < 0){
        printf("Can\'t get msqid\n");
        exit(-1);
    } 
    
    return msqid;
}

int main(){
    int len; 
    int sem_value = 0;
    
    struct request request_buf;
    struct response response_buf;

    STAILQ_HEAD(stailhead, entry) head = STAILQ_HEAD_INITIALIZER(head);
    struct stailhead *headp;
    struct entry {
        int id;
        int sem_value;
        STAILQ_ENTRY(entry) entries;
    } *n1;
    
    STAILQ_INIT(&head);
    
    int msqid = create_mq(pathname);
    
    while(1){
        
        if ((len = msgrcv(msqid, (struct request *) &request_buf, sizeof(request_buf), SEMAPHORE_MSG, 0) < 0)){
            printf("Can\'t receive message from queue\n");
            exit(-1);
        }
        
        if (request_buf.sem_value >= 0) {
            sem_value += request_buf.sem_value;
            
            response_buf.m_type = request_buf.id;
            
            printf("condition is set (id=%d, sem_value=%d), total sem_value=%d\n", request_buf.id, request_buf.sem_value, sem_value);
            
            if (msgsnd(msqid, (struct response *) &response_buf, sizeof(response_buf), 0) < 0){
                printf("Can\'t send message to queue 1\n");
                msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
                exit(-1);
            }
        } else {
            n1 = malloc(sizeof(struct entry));
            n1->id = request_buf.id;
            n1->sem_value = request_buf.sem_value;
            STAILQ_INSERT_HEAD(&head, n1, entries);
        }

        while (!STAILQ_EMPTY(&head)) {
            n1 = STAILQ_FIRST(&head);
        
            if (sem_value + n1->sem_value >= 0) {
                
                sem_value += n1->sem_value;
                response_buf.m_type = n1->id;
                
                printf("condition is set (id=%d, sem_value=%d), total sem_value=%d\n", request_buf.id, request_buf.sem_value, sem_value);
                
                printf("responding to (id=%d)\n", response_buf.m_type);
                
                if (msgsnd(msqid, (struct response *) &response_buf, sizeof(response_buf), 0) < 0){
                    printf("Can\'t send message to queue 2\n");
                    msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
                    exit(-1);
                }
            } else {
                break;
            }

            STAILQ_REMOVE_HEAD(&head, entries);
            free(n1);
        }
        
    }
    return 0; 
}
