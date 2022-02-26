#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define BUFFER_SIZE 32

int done;

void handler1(int signum) {
  printf("this is handler1(): got a signal %d\n", signum);
  done = 1;
}

int main(int argc, char *argv[]) {

  int pid;
  int memid;
  int key = IPC_PRIVATE;
  char *ptr;
  char buffer[BUFFER_SIZE];
  struct sigaction action;
  done = 0;

  // shmget() returns the shared memory identifier associated with 'key'
  // key is of 'key_t' type; for requesting resources (like shared mem)
  // So i think memid is the shared mem id
  memid = shmget(key, BUFFER_SIZE, IPC_EXCL | 0666);
  
  // Sets 'action' struct equal to void handler1 'object' (Int to be passed in later?)
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = handler1;

  // sigaction is a built in function. Sets program behavior when receiving certain OS signals. 
  // SIGUSR1 is a signal, used for interprocess communication
  // &action is a reference to the newly set action struct
  // So I guess when -SIGUSR1 is calld in terminal, the action of the handler1() function will be executed,
  // and set 'done' equal to 1 which allow while loop to end.
  sigaction(SIGUSR1, &action, NULL);

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

    // this is the parent
    printf("I am the parent, and my pid is %d\n", getpid());

    // shmat() is 'shared memory attach'
    // memid is the id of the shared memory
    ptr = (char *) shmat(memid, 0, 0);

    // If shared mem is empty
    if (ptr == NULL) {
      printf("shmat() failed\n");
      return(8);
    }

    while(!done);

    strcpy(buffer, "hello");
    printf("Parent is writing '%s' to the shared memory\n", buffer);
    
    // Copy the contents of the buffer to ptr of the share mem id attach; shmat()
    strcpy(ptr, buffer);
    //wait(NULL);
    while(!done);

    //pid = fork();
    strcpy(buffer, "from");
    printf("Parent is writing '%s' to the shared memory\n", buffer);

    // Copy the contents of the buffer to ptr of the share mem id attach; shmat()
    strcpy(ptr, buffer);
    //wait(NULL);
    while(!done);

    strcpy(buffer, "Tyler");
    printf("Parent is writing '%s' to the shared memory\n", buffer);

    // Copy the contents of the buffer to ptr of the share mem id attach; shmat()
    strcpy(ptr, buffer);
    //wait(NULL);
    while(!done);
  } 

  // if pid == 0; means child
  else {

    // this is the child
    pid = getpid();

    printf("I am the child, and my pid is %d\n", pid);

    // Having kill here gets me parent next
    kill(getppid(), SIGUSR1);
    done = 0;

    ptr = (char *) shmat(memid, 0, 0);
    if (ptr == NULL) {
      printf("shmat() in child failed\n");
      return(8);
    }

    printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);

    // kill(getppid(), SIGUSR1);
    shmdt(ptr);

    ptr = (char *) shmat(memid, 0, 0);

    kill(getppid(), SIGUSR1);
    printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);

    shmdt(ptr);

    ptr = (char *) shmat(memid, 0, 0);

    kill(getppid(), SIGUSR1);
    printf("I am the child, and I read this from the shared memory: '%s'\n", ptr);

    shmdt(ptr);

    //kill(getppid(), SIGUSR1);

  }

  shmdt(ptr);
  shmctl(memid, IPC_RMID, NULL);

  return 0;
}
