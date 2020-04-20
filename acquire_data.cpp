
#include <iostream>
using namespace std;
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include "javacquire.h"

#define QSIZE 100
#define PI (4.0*atan((double)(1.0)))

extern GEN *gen;
extern FILE *fptrout;
extern int wknsave;

char msgid[QSIZE];
int nq = 0;

void acquire_data(FILE *fptrin, char *port)
{
  bool output;
  unsigned short shortwkn,shortyear;
  unsigned int ittag,iwks;
  char xmsglength[5],msgbody[1000],rcvid[100],stemp[100];
  int flag,msglength;
  float floatnvel,floatevel,floathvel,floathdop,floatvdop,floattdop;
  double dtemp;
  double ttag;
  int get2bytes(FILE *, char *);
  int get1morebyte(FILE *, char *);
  int getmsglength(FILE *, char *);
  int passchecksum2bytes(char *,char *,char *,int);
  int passchecksum1byte(char *,char *,char *,int);
  void writemsg(char *, char *,char *,int, FILE *);
  void writecrnl(FILE *);
  void writenl(FILE *);
  void byteswap(char *,char *,int);

  // Read first two bytes at start or after message acquired OK
  if (nq==0) 
  {
    flag = get2bytes(fptrin,msgid);
  }

  // Otherwise get another character to attempt re-sync
  else 
  {
    flag = get1morebyte(fptrin,msgid);
  }

  // CHECK FOR EACH KNOWN MESSAGE TYPE
  // Text replies (in an assumed sequence)
  if (strncmp(msgid,"RE",2) == 0)
  {
    nq = 0;
    msglength = getmsglength(fptrin,xmsglength);
    fread((void *)msgbody,msglength,1,fptrin);
    fprintf(fptrout,"RE %d\n",msglength);
    if (gen->REgood==0) // assume this is a receiver ID message
    {
      strncpy(rcvid,msgbody,msglength);
      rcvid[msglength] = '\0';
      strcpy(gen->rcvid,rcvid);
      fprintf(fptrout,"receiver ID is %s\n",gen->rcvid);
    }
    else if (gen->REgood==1) // assume this is a total mem avail msg
    {
      strncpy(stemp,msgbody,msglength);
      stemp[msglength] = '\0';
      gen->totalmem = atol(stemp);
      fprintf(fptrout,"%ld tot mem is %ld\n",strlen(msgbody),gen->totalmem);
    }
    else if (gen->REgood==2) // assume this is a board version msg
    {
      strncpy(stemp,msgbody,msglength);
      stemp[msglength] = '\0';
      strcpy(gen->boardver,stemp);
      fprintf(fptrout,"%ld board version is %s\n",strlen(msgbody),gen->boardver);
    }
    else if (gen->REgood==3) // assume this is a firmware version msg
    {
      strncpy(stemp,msgbody,msglength);
      stemp[msglength] = '\0';
      strcpy(gen->firmver,stemp);
      fprintf(fptrout,"%ld firmware version is %s\n",strlen(msgbody),gen->firmver);
    }
    else // assume this is a current mem avail msg
    {
      strncpy(stemp,msgbody,msglength);
      stemp[msglength] = '\0';
      gen->availmem = atol(stemp);
      fprintf(fptrout,"%ld avl mem is %ld\n",strlen(msgbody),gen->availmem);
    }
    ++gen->REgood;
  }

  // GPS time message
  if (strncmp(msgid,"GT",2) == 0)
  {
    nq = 0;
    msglength = getmsglength(fptrin,xmsglength);
    fread((void *)msgbody,msglength,1,fptrin);
    if (passchecksum1byte(msgid,xmsglength,msgbody,msglength))
    {
      ++gen->GTgood;
      fprintf(fptrout,"GT %d\n",msglength);
      byteswap(&msgbody[0],(char *)&iwks,4);
      gen->wks = (double)iwks/1000.0;
      byteswap(&msgbody[4],(char *)&shortwkn,2);
      while (shortwkn<wknsave) shortwkn += 1024;
      wknsave = shortwkn;
      gen->wkn = shortwkn;
      fprintf(fptrout,"wkn=%d wks=%lf\n",gen->wkn,gen->wks);
      //writemsg(msgid,xmsglength,msgbody,msglength,fptrout);
      //writecrnl(fptrout);
    }
    else
    {
      ++gen->GTbad;
      fprintf(fptrout,"GT message FAILED CHECKSUM\n");
    }
  }

  // GPS-UTC offset
  else if (strncmp(msgid,"UO",2) == 0)
  {
    nq = 0;
    msglength = getmsglength(fptrin,xmsglength);
    fread((void *)msgbody,msglength,1,fptrin);
    if (passchecksum1byte(msgid,xmsglength,msgbody,msglength))
    {
      ++gen->UOgood;
      fprintf(fptrout,"UO %d\n",msglength);
      gen->leapsecs = (int)msgbody[18];
      fprintf(fptrout,"leapsecs=%d\n",gen->leapsecs);
      //writemsg(msgid,xmsglength,msgbody,msglength,fptrout);
      //writecrnl(fptrout);
    }
    else
    {
      ++gen->UObad;
      fprintf(fptrout,"UO message FAILED CHECKSUM\n");
    }
  }

  // Receiver date
  else if (strncmp(msgid,"RD",2) == 0)
  {
    nq = 0;
    msglength = getmsglength(fptrin,xmsglength);
    fread((void *)msgbody,msglength,1,fptrin);
    if (passchecksum1byte(msgid,xmsglength,msgbody,msglength))
    {
      ++gen->RDgood;
      fprintf(fptrout,"RD %d\n",msglength);
      byteswap(&msgbody[0],(char *)&shortyear,2);
      gen->year = (int)shortyear;
      gen->month = (int)msgbody[2];
      gen->day = (int)msgbody[3];
      fprintf(fptrout,"year=%d month=%d day=%d\n",gen->year,gen->month,gen->day);
      //writemsg(msgid,xmsglength,msgbody,msglength,fptrout);
      //writecrnl(fptrout);
    }
    else
    {
      ++gen->RDbad;
      fprintf(fptrout,"RD message FAILED CHECKSUM\n");
    }
  }

  // Geodetic position message
  else if (strncmp(msgid,"PG",2) == 0)
  {
    nq = 0;
    msglength = getmsglength(fptrin,xmsglength);
    fread((void *)msgbody,msglength,1,fptrin);
    if (passchecksum1byte(msgid,xmsglength,msgbody,msglength))
    {
      ++gen->PGgood;
      fprintf(fptrout,"PG %d\n",msglength);
      byteswap(&msgbody[0],(char *)&dtemp,8);
      gen->lat = dtemp;
      byteswap(&msgbody[8],(char *)&dtemp,8);
      gen->lon = dtemp;
      byteswap(&msgbody[16],(char *)&dtemp,8);
      gen->ellht = dtemp;
      fprintf(fptrout,"lat=%lf lon=%lf ",gen->lat*180.0/PI,gen->lon*180.0/PI);
      fprintf(fptrout,"ellht=%lf\n",gen->ellht);
      //writemsg(msgid,xmsglength,msgbody,msglength,fptrout);
      //writecrnl(fptrout);
    }
    else
    {
      ++gen->PGbad;
      fprintf(fptrout,"PG message FAILED CHECKSUM\n");
    }
  }

  // Geodetic velocity message
  else if (strncmp(msgid,"VG",2) == 0)
  {
    nq = 0;
    msglength = getmsglength(fptrin,xmsglength);
    fread((void *)msgbody,msglength,1,fptrin);
    if (passchecksum1byte(msgid,xmsglength,msgbody,msglength))
    {
      ++gen->VGgood;
      fprintf(fptrout,"VG %d\n",msglength);
      byteswap(&msgbody[0],(char *)&floatnvel,4);
      byteswap(&msgbody[4],(char *)&floatevel,4);
      byteswap(&msgbody[8],(char *)&floathvel,4);
      gen->nvel = (double)floatnvel;
      gen->evel = (double)floatevel;
      gen->hvel = (double)floathvel;
      fprintf(fptrout,"nvel=%lf evel=%lf ",gen->nvel,gen->evel);
      fprintf(fptrout,"hvel=%lf\n",gen->hvel);
      //writemsg(msgid,xmsglength,msgbody,msglength,fptrout);
      //writecrnl(fptrout);
    }
    else
    {
      ++gen->VGbad;
      fprintf(fptrout,"VG message FAILED CHECKSUM\n");
    }
  }

  // Dilution of precision message
  else if (strncmp(msgid,"DP",2) == 0)
  {
    nq = 0;
    msglength = getmsglength(fptrin,xmsglength);
    fread((void *)msgbody,msglength,1,fptrin);
    if (passchecksum1byte(msgid,xmsglength,msgbody,msglength))
    {
      ++gen->DPgood;
      fprintf(fptrout,"DP %d\n",msglength);
      byteswap(&msgbody[0],(char *)&floathdop,4);
      byteswap(&msgbody[4],(char *)&floatvdop,4);
      byteswap(&msgbody[8],(char *)&floattdop,4);
      gen->hdop = (double)floathdop;
      gen->vdop = (double)floatvdop;
      gen->tdop = (double)floattdop;
      fprintf(fptrout,"hdop=%lf vdop=%lf ",gen->hdop,gen->vdop);
      fprintf(fptrout,"tdop=%lf\n",gen->tdop);
      //writemsg(msgid,xmsglength,msgbody,msglength,fptrout);
      //writecrnl(fptrout);
    }
    else
    {
      ++gen->DPbad;
      fprintf(fptrout,"DP message FAILED CHECKSUM\n");
    }
  }

  // GPS to receiver time offset
  else if (strncmp(msgid,"GO",2) == 0)
  {
    nq = 0;
    msglength = getmsglength(fptrin,xmsglength);
    fread((void *)msgbody,msglength,1,fptrin);
    if (passchecksum1byte(msgid,xmsglength,msgbody,msglength))
    {
      ++gen->GOgood;
      fprintf(fptrout,"GO %d\n",msglength);
      byteswap(&msgbody[0],(char *)&dtemp,8);
      gen->gpsclockerror = dtemp;
      fprintf(fptrout,"gpsclockerror=%lf\n",gen->gpsclockerror);
      //writemsg(msgid,xmsglength,msgbody,msglength,fptrout);
      //writecrnl(fptrout);
    }
    else
    {
      ++gen->GObad;
      fprintf(fptrout,"GO message FAILED CHECKSUM\n");
    }
  }

  // Message not recognized
  else
  {
    //printf("message not recognized, resyncing, id bytes are %d and %d\n",msgid[0],msgid[1]);
    //fprintf(fptrout,"UNRECOGNIZED MESSAGE %2s\n",msgid);
    //flag = get1morebyte(fptrin,msgid);
  }


}


int passchecksum2bytes(char *msgid,char *xmsglength,char *msgbody,int msglength)
{
  char allmessage[500];
  unsigned int cksum1,cksum2;
  unsigned char cs(char *,int);

  // Extract checksum from message body
  sscanf(&msgbody[msglength-2],"%2x",&cksum1);  

  // Checksum computed on msg ID, msg length, and msg body, but not checksum itself
  strncpy(&allmessage[0],msgid,2);
  strncpy(&allmessage[2],xmsglength,3);
  strncpy(&allmessage[5],msgbody,msglength);
  cksum2 = (unsigned int)cs(allmessage,msglength+5-2);
  //printf("in passchecksum, cksum1 = %d  cksum2 = %d\n",cksum1,cksum2);

  // Return 1 if checksum passes, otherwise return 0
  if (cksum1==cksum2)
    return(1);
  else
  {
    fprintf(fptrout,"failed checksum, cksum1 = %d  cksum2 = %d\n",cksum1,cksum2);
    return(0);
  }

}


int passchecksum1byte(char *msgid,char *xmsglength,char *msgbody,int msglength)
{
  int i;
  char allmessage[500];
  unsigned char cksum1,cksum2;
  unsigned char cs(char *,int);

  // Extract checksum from message body
  cksum1 = msgbody[msglength-1];

  // Checksum computed on msg ID, msg length, and msg body, but not checksum itself
  memcpy(&allmessage[0],msgid,2);
  memcpy(&allmessage[2],xmsglength,3);
  memcpy(&allmessage[5],msgbody,msglength);
  cksum2 = cs(allmessage,msglength+5-1);
  //printf("in passchecksum, cksum1 = %d  cksum2 = %d\n",cksum1,cksum2);

  // Return 1 if checksum passes, otherwise return 0
  if (cksum1==cksum2)
    return(1);
  else
  {
    fprintf(fptrout,"failed checksum, cksum1 = %d  cksum2 = %d msglength=%d\n",cksum1,cksum2,msglength);
    return(0);
  }

}


void writenl(FILE *fptrout)
{
  char nl[1];
  nl[0] = '\n';
  fwrite((const void *)nl,1,1,fptrout);
}


void writecrnl(FILE *fptrout)
{
  char cr[1],nl[1];
  cr[0] = '\r';
  nl[0] = '\n';
  fwrite((const void *)cr,1,1,fptrout);
  fwrite((const void *)nl,1,1,fptrout);
}



void writemsg(char *msgid, char *xmsglength, char *msgbody,int msglength, FILE *fptrout)
{
  fwrite((const void *)msgid,2,1,fptrout);
  fwrite((const void *)xmsglength,3,1,fptrout);
  fwrite((const void *)msgbody,msglength,1,fptrout);
  //printf("msglength in writeidandbody is %d\n",msglength);
}


int getmsglength(FILE *fptrin,char *xmsglength)
{
  int msglength;

  fread((void *)xmsglength,3,1,fptrin);
  xmsglength[3] = '\0';
  sscanf(xmsglength,"%x",&msglength);
  return(msglength);
}


int get1morebyte(FILE *fptrin,char *msgid)
{
  nq = 2;
  msgid[0] = msgid[1];
  size_t readflag = fread((void *)(&msgid[1]),1,1,fptrin);
  if (readflag == 1)
  {
    return(0);
  }
  else
  {
    //printf("End of file reached in get1morebyte\n");
    return(-1);
  }
}


int get2bytes(FILE *fptrin,char *msgid)
{
  size_t readflag = fread((void *)&msgid[0],2,1,fptrin);
  msgid[2] = '\0';
  if (readflag == 1)
  { 
    nq = 2;
    return(0);
  }
  else
  {
    //printf("End of file reached in get2bytes\n");
    nq = 2;
    return(-1);
  }
}

void byteswap(char *in,char *out,int len)
{
  int i,len2;

  len2 = len-1;
  for (i=0;i<len;i++)
  {
    /*out[i]=in[len2-i];*/
    out[i] = in[i];
  }

}


typedef unsigned char u1;

enum 
{
  bits = 8,
  lShift = 2,
  rShift = bits - lShift
};


#define ROT_LEFT(val) ((val << lShift) | (val >> rShift))

u1 cs(char* src, int count)
{
  u1 res = 0;
  while(count--)
    res = ROT_LEFT(res) ^ *src++;
  return ROT_LEFT(res);
}


//void parse_mpc(char *mpc)
//{
//  void byteswap(char *, char *,int);

  // Parse the receive time
  //byteswap( &pbn[0],(char *)&(gen->nav.pbentime),4);
  
  // Parse the PRN number
//  int prn = (int)mpc[3];
  
  // Parse the elevation angle
//  gen->sat[prn].el = (float)((unsigned char)mpc[4]);
  
  // Parse the azimuth
//  gen->sat[prn].az = (float)(2*(unsigned char)mpc[5]);
  
  // Parse the signal to noise ratios
//  gen->sat[prn].snrca = (int)(unsigned char)mpc[10];
//  gen->sat[prn].snrp1 = (int)(unsigned char)mpc[39];
//  gen->sat[prn].snrp2 = (int)(unsigned char)mpc[68];
//  if (!strncmp(gen->rtype,"ZM",2))
//  {
//    gen->sat[prn].snrca = (int)((double)(gen->sat[prn].snrca)/4.5);
//    gen->sat[prn].snrp1 = (int)((double)(gen->sat[prn].snrp1)/4.5);
//    gen->sat[prn].snrp2 = (int)((double)(gen->sat[prn].snrp2)/4.5);
//  }
  // NOTE: The scaling on Z-12 SNRs is to bring it into scale
  // with Eurocard SNRs.  4.5 is a back-of-envelope approx.
  
  // Parse the warning flags
//  gen->sat[prn].flagca = (unsigned char)mpc[7];
//  gen->sat[prn].flagp1 = (unsigned char)mpc[36];
//  gen->sat[prn].flagp2 = (unsigned char)mpc[65];

  // Timetag the acquisition
//  gettimeofday(&(gen->sat[prn].satupdate),NULL);

//}

