
#include <iostream>
using namespace std;
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

extern bool option_running;
extern bool option_quit;
extern int optind;

void process_input(int argc,char *argv[])
{
  int opt;
  void usage_exit();
  
  //  Process the - options
  while ((opt=getopt(argc,argv,"rq")) != -1)
  {
    switch (opt)
    {
      case 'r':
        option_running = 1;
	break;
      case 'q':
        option_quit = 1;
	break;
    }
  }
  
  // Process the input
  if (optind>=argc)
    usage_exit();
  if (strcmp(argv[optind],"/dev/ttyUSB0\0") &&
      strcmp(argv[optind],"/dev/ttyUSB1\0") &&
      strcmp(argv[optind],"/dev/ttyUSB2\0") &&
      strcmp(argv[optind],"/dev/ttyUSB3\0") )
  {
    cout << argv[optind];
    cout << " is not a valid port - exiting\n";
    usage_exit();
  }
      
}


void usage_exit()
{
  cout << "Usage: javacquire [options] <port>\n";
  cout << "  valid options are:\n";
  cout << "  -r queries if program already running ";
  cout << "on specified port\n";
  cout << "  -q quit acquiring on specified port\n";
  cout << "  valid ports are:\n";
  cout << "  /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyUSB2 /dev/ttyUSB3\n";
  exit(-1);
}

