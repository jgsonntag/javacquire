#include <iostream>
using namespace std;
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

#include "javacquire.h"

#define PROJ_ID 1

extern GEN *gen;
extern int shmid;

void get_shm(char *path)
{
  //int shmid;
  key_t key;
   
  // Generate the key
  key = ftok(path,PROJ_ID);

  cout << key << "\n";
   
  // Get shmid if shared memory segment already exists
  if ((shmid=shmget(key,sizeof(GEN),0666)) == -1)
  {
   
    // Or create it if not
    if ((shmid=shmget(key,sizeof(GEN),0666|IPC_CREAT))==-1)
    {
      cout << "Cannot create shared memory - exiting\n";
      exit(-1);
    }
  }
  
  // Attach to the shared memory
  if ( (gen=(GEN *)shmat(shmid,0,0)) < (GEN *)0)
  {
    cout << "Cannot attach shared memory - exiting\n";
    exit(-1);
  }
  
}


void destroy_shm()
{
  struct shmid_ds *buf;
  
  shmdt(gen);
  shmctl(shmid,IPC_RMID,buf);
}
