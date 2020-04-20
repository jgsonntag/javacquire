/*-------------------------------------------------------------------------
 NAME:     javacquire.cpp

 PURPOSE:  
 
 AUTHOR:   John Gary Sonntag

 DATE:     16 October 2013
-------------------------------------------------------------------------*/

#include <iostream>
using namespace std;
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "javacquire.h"

extern int optind;

bool option_running;
bool option_quit;
int shmid;
int wknsave;
GEN *gen;
FILE *fptrout;

main(int argc,char *argv[])
{
  bool wkfile;
  char *homedir;
  char line[85],cpath[200];
  FILE *fptr,*fptrwkn;
  void process_input(int,char **);
  void get_shm(char *),destroy_shm();
  void acquire_data(FILE *, char *);

  // Process command-line input
  option_running = 0;
  option_quit = 0;
  process_input(argc,argv);

  // Create (if necessary) and attach shared memory
  get_shm(argv[optind]);
  
  // Is this run a status check?
  if (option_running)
  {
    exit(gen->run);
  }
    
  // Is this a stop acquiring order?
  if (option_quit)
  {
    gen->run = 0;
    exit(0);
  }
  
  // Run normally
  gen->run = 1;
  gen->REgood = 0;
  gen->GTgood = 0;
  gen->GTbad = 0;
  gen->UOgood = 0;
  gen->UObad = 0;
  gen->RDgood = 0;
  gen->RDbad = 0;
  gen->PGgood = 0;
  gen->PGbad = 0;
  gen->VGgood = 0;
  gen->VGbad = 0;
  //fptrout = fopen(argv[optind+2],"w");
  fptrout = fopen("temp","w");
  if ( (fptr=fopen(argv[optind],"r")) == NULL )
  {
    cout << "Cannot open " << argv[optind];
    cout << " - exiting\n";
    exit(-1);
  }
  //strcpy(gen->outfile,argv[optind+2]);

  // Open the saved wkn file (helps deal with the 19-year wkn rollover)
  homedir = getenv("HOME");
  strcpy(cpath,homedir);
  strcat(cpath,"/.javacquire");
  if ((fptrwkn=fopen(cpath,"a+")) == NULL)
  {
    printf("Cannot open %s\n",cpath);
    wkfile = false;
    wknsave = 2048;
  }
  else
  {
    wknsave = 0;
    printf("File %s opened\n",cpath);
    wkfile = true;
    while (fgets(line,85,fptrwkn)!=NULL)
    {
      printf("point A\n");
      sscanf(line,"%d",&wknsave);
    }
  }
  if (wknsave==0) wknsave = 2048;
  printf("Initial wknsave=%d\n",wknsave);

  while(gen->run)
  {
    acquire_data(fptr,argv[optind]);
  }
  
  // Destroy the shared memory
  destroy_shm();
  
  // Close the wkn file
  if (wkfile)
  {
    fprintf(fptrwkn,"%d\n",wknsave);
    fclose(fptrwkn);
  }

  //  Close the serial port and output file
  fclose(fptr);
  fclose(fptrout);

}

