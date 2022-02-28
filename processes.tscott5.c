#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define BUFFER_SIZE 32

int doneParent;
int doneChild;

void handlerParent(int signum) {
  printf("----------------------------------------------\n");
  printf("Got signal %d\n", signum);
  doneParent = 1;
}

void handlerChild(int signum) {
  printf("----------------------------------------------\n");
  printf("Got signal %d\n", signum);
  doneParent = 1;
}

int main(int argc, char *argv[]) {

  int pid;
  int memid;
  int key = IPC_PRIVATE;
  char *ptr;
  char buffer[BUFFER_SIZE];
  struct sigaction actionParent;
  struct sigaction actionChild;
  doneParent = 0;
  doneChild = 0;

  // shmget() returns the shared memory identifier associated with 'key'
  // key is of 'key_t' type; for requesting resources (like shared mem)
  // So i think memid is the shared mem id
  memid = shmget(key, BUFFER_SIZE, IPC_EXCL | 0666);
  
  // Sets 'action' struct equal to void handler1 'object' (Int to be passed in later?)
  memset(&actionParent, 0, sizeof(struct sigaction));
  actionParent.sa_handler = handlerParent;

  memset(&actionChild, 0, sizeof(struct sigaction));
  actionChild.sa_handler = handlerParent;

  // sigaction is a built in function. Sets program behavior when receiving certain OS signals. 
  // SIGUSR1 is a user defined signal, used for interprocess communication
  // &action is a reference to the newly set action struct
  // So I guess when -SIGUSR1 is calld in terminal, the action of the handler1() function will be executed,
  // and set 'done' equal to 1 which allows while loop to end.
  sigaction(SIGUSR1, &actionParent, NULL);
  sigaction(SIGUSR2, &actionChild, NULL);

  if (memid < 0) {
    printf("shmget() failed\n");
    return(8);
  }

  // Creates process
  pid = fork();

  // Process not created
  if (pid < 0) {
    printf("fork failed\n");
    return(8);
  }

  // Process created successfully
  // fork() returns process id of child to parent 
  if (pid > 0) {

    printf("I am the parent, and my pid is %d\n", getpid());
    while(!doneParent);
    doneParent = 0;

    // shmat() is 'shared memory attach'
    // memid is the id of the shared memory
    ptr = (char *) shmat(memid, 0, 0);

    // If shared mem is empty
    if (ptr == NULL) {
      printf("shmat() failed\n");
      return(8);
    }

    strcpy(buffer, "hello");
    printf("Parent is writing '%s' to the shared memory\n", buffer);   

    // Copy the contents of the buffer to ptr of the share mem id attach; shmat()
    strcpy(ptr, buffer);
    while(!doneParent);
    doneParent = 0;

    strcpy(buffer, "from");
    printf("Parent is writing '%s' to the shared memory\n", buffer);    

    // Copy the contents of the buffer to ptr of the share mem id attach; shmat()
    strcpy(ptr, buffer);
    while(!doneParent);
    doneParent = 0;

    strcpy(buffer, "tyler");
    printf("Parent is writing '%s' to the shared memory\n", buffer);    

    // Copy the contents of the buffer to ptr of the share mem id attach; shmat()
    strcpy(ptr, buffer);
    while(!doneParent);
    doneParent = 0;

    strcpy(buffer, "done");
    printf("Parent is writing '%s' to the shared memory\n", buffer);

    // Copy the contents of the buffer to ptr of the share mem id attach; shmat()
    strcpy(ptr, buffer);
    while(!doneParent);
    doneParent = 0;

  } 

  // if pid == 0; child process
  else {

    // child process id
    pid = getpid();

    printf("I am the child, and my pid is %d\n", pid);

    // Stops while loop in parent process by passing signal.
    kill(getppid(), SIGUSR1);
    sleep(1);

    ptr = (char *) shmat(memid, 0, 0);
    // if (ptr == NULL) {
    //   printf("shmat() in child failed\n");
    //   return(8);
    // }
    printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);

    // Share memory detach; removes element from shared mem after read by child
    shmdt(ptr);

    kill(getppid(), SIGUSR1);
    sleep(1);

    ptr = (char *) shmat(memid, 0, 0);
    printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);
    shmdt(ptr);

    kill(getppid(), SIGUSR1);
    sleep(1);

    ptr = (char *) shmat(memid, 0, 0);
    printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);
    shmdt(ptr);

    kill(getppid(), SIGUSR1);
    sleep(1);

    ptr = (char *) shmat(memid, 0, 0);
    printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);
    shmdt(ptr);

    //kill(getppid(), SIGUSR1);
    sleep(1);

    //exit(0);

    // Exit both processes once 'done' is read
    if (ptr == "done") {
      exit(0);
    }
  }

  shmdt(ptr);
  shmctl(memid, IPC_RMID, NULL);

  return 0;
}
