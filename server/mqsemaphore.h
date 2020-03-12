 struct request {
    long m_type;
    int id;
    int sem_value;
} common_request;

struct response {
    long m_type;
};

const char pathname[] = "key1"; 

const int SEMAPHORE_MSG = 128;
