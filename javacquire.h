
#include <sys/time.h>


struct GEN
{
  bool run;
  char outfile[100];
  char rcvid[100];
  char boardver[100];
  char firmver[100];
  long totalmem;
  long availmem;
  int wkn;
  double wks;
  int leapsecs;
  int year;
  int month;
  int day;
  double gpsclockerror;
  double tdop;
  double hdop;
  double vdop;
  double lat;
  double lon;
  double ellht;
  double nvel;
  double evel;
  double hvel;
  unsigned int REgood;
  unsigned int GTgood;
  unsigned int GTbad;
  unsigned int UOgood;
  unsigned int UObad;
  unsigned int RDgood;
  unsigned int RDbad;
  unsigned int PGgood;
  unsigned int PGbad;
  unsigned int VGgood;
  unsigned int VGbad;
  unsigned int DPgood;
  unsigned int DPbad;
  unsigned int GOgood;
  unsigned int GObad;
};

