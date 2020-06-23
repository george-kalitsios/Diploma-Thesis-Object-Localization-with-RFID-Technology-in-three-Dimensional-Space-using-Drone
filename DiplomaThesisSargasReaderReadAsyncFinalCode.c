/*
Diploma Thesis , June 2020
George Kalitsios ,THMMY AUTH
 */

#include <tm_reader.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#ifndef WIN32
#include <unistd.h>
#endif

/* Enable this to use transportListener */
#ifndef USE_TRANSPORT_LISTENER
#define USE_TRANSPORT_LISTENER 0
#endif

#define numberof(x) (sizeof((x))/sizeof((x)[0]))

#define usage() {errx(1, "Please provide valid reader URL, such as: reader-uri [--ant n]\n"\
                         "reader-uri : e.g., 'tmr:///COM1' or 'tmr:///dev/ttyS0/' or 'tmr://readerIP'\n"\
                         "[--ant n] : e.g., '--ant 1'\n"\
                         "Example: 'tmr:///com4' or 'tmr:///com4 --ant 1,2' \n");}

void errx(int exitval, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);

  exit(exitval);
}

void checkerr(TMR_Reader* rp, TMR_Status ret, int exitval, const char *msg)
{
  if (TMR_SUCCESS != ret)
  {
    errx(exitval, "Error %s: %s\n", msg, TMR_strerr(rp, ret));
  }
}

void serialPrinter(bool tx, uint32_t dataLen, const uint8_t data[],
                   uint32_t timeout, void *cookie)
{
  FILE *out = cookie;
  uint32_t i;

  fprintf(out, "%s", tx ? "Sending: " : "Received:");
  for (i = 0; i < dataLen; i++)
  {
    if (i > 0 && (i & 15) == 0)
      fprintf(out, "\n         ");
    fprintf(out, " %02x", data[i]);
  }
  fprintf(out, "\n");
}

void stringPrinter(bool tx,uint32_t dataLen, const uint8_t data[],uint32_t timeout, void *cookie)
{
  FILE *out = cookie;

  fprintf(out, "%s", tx ? "Sending: " : "Received:");
  fprintf(out, "%s\n", data);
}

void parseAntennaList(uint8_t *antenna, uint8_t *antennaCount, char *args)
{
  char *token = NULL;
  char *str = ",";
  uint8_t i = 0x00;
  int scans;

  /* get the first token */
  if (NULL == args)
  {
    fprintf(stdout, "Missing argument\n");
    usage();
  }

  token = strtok(args, str);
  if (NULL == token)
  {
    fprintf(stdout, "Missing argument after %s\n", args);
    usage();
  }

  while(NULL != token)
  {
    scans = sscanf(token, "%"SCNu8, &antenna[i]);
    if (1 != scans)
    {
      fprintf(stdout, "Can't parse '%s' as an 8-bit unsigned integer value\n", token);
      usage();
    }
    i++;
    token = strtok(NULL, str);
  }
  *antennaCount = i;
}

void callback(TMR_Reader *reader, const TMR_TagReadData *t, void *cookie);
void exceptionCallback(TMR_Reader *reader, TMR_Status error, void *cookie);

int main(int argc, char *argv[])
{
if (remove("/home/debian/mercuryapi-1.31.3.36/c/src/samples//program.txt") == 0) 
  printf("Deleted successfully \n"); 
else
  printf("Unable to delete the file \n"); 

#ifndef TMR_ENABLE_BACKGROUND_READS
  errx(1, "This sample requires background read functionality.\n"
          "Please enable TMR_ENABLE_BACKGROUND_READS in tm_config.h\n"
          "to run this codelet\n");
  return -1;
#else

  TMR_Reader r, *rp;
  TMR_Status ret;
  TMR_Region region;
  TMR_ReadPlan plan;
  TMR_ReadListenerBlock rlb;
  TMR_ReadExceptionListenerBlock reb;
  uint8_t *antennaList = NULL;
  uint8_t buffer[20];
  uint8_t i;
  uint8_t antennaCount = 0x0;
#if USE_TRANSPORT_LISTENER
  TMR_TransportListenerBlock tb;
#endif

  if (argc < 2)
  {
    usage(); 
  }

  for (i = 2; i < argc; i+=2)
  {
    if(0x00 == strcmp("--ant", argv[i]))
    {
      if (NULL != antennaList)
      {
        fprintf(stdout, "Duplicate argument: --ant specified more than once\n");
        usage();
      }
      parseAntennaList(buffer, &antennaCount, argv[i+1]);
      antennaList = buffer;
    }
    else
    {
      fprintf(stdout, "Argument %s is not recognized\n", argv[i]);
      usage();
    }
  }
  
  rp = &r;
  ret = TMR_create(rp, argv[1]);
  checkerr(rp, ret, 1, "creating reader");

#if USE_TRANSPORT_LISTENER

  if (TMR_READER_TYPE_SERIAL == rp->readerType)
  {
    tb.listener = serialPrinter;
  }
  else
  {
    tb.listener = stringPrinter;
  }
  tb.cookie = stdout;

  TMR_addTransportListener(rp, &tb);
#endif

  ret = TMR_connect(rp);
  checkerr(rp, ret, 1, "connecting reader");

  region = TMR_REGION_NONE;
  ret = TMR_paramGet(rp, TMR_PARAM_REGION_ID, &region);
  checkerr(rp, ret, 1, "getting region");

  if (TMR_REGION_NONE == region)
  {
    TMR_RegionList regions;
    TMR_Region _regionStore[32];
    regions.list = _regionStore;
    regions.max = sizeof(_regionStore)/sizeof(_regionStore[0]);
    regions.len = 0;

    ret = TMR_paramGet(rp, TMR_PARAM_REGION_SUPPORTEDREGIONS, &regions);
    checkerr(rp, ret, __LINE__, "getting supported regions");

    if (regions.len < 1)
    {
      checkerr(rp, TMR_ERROR_INVALID_REGION, __LINE__, "Reader doesn't supportany regions");
    }
    region = regions.list[0];
    ret = TMR_paramSet(rp, TMR_PARAM_REGION_ID, &region);
    checkerr(rp, ret, 1, "setting region");  
  }
  /**
   * Checking the software version of the sargas.
   * The antenna detection is supported on sargas from software version of 5.3.x.x.
   * If the Sargas software version is 5.1.x.x then antenna detection is not supported.
   * User has to pass the antenna as arguments.
   */
  {
    ret = isAntDetectEnabled(rp, antennaList);
    if(TMR_ERROR_UNSUPPORTED == ret)
    {
      fprintf(stdout, "Reader doesn't support antenna detection. Please provide antenna list.\n");
      usage();
    }
    else
    {
      checkerr(rp, ret, 1, "Getting Antenna Detection Flag Status");
    }
  }
  /**
  * for antenna configuration we need two parameters
  * 1. antennaCount : specifies the no of antennas should
  *    be included in the read plan, out of the provided antenna list.
  * 2. antennaList  : specifies  a list of antennas for the read plan.
  **/ 

  //uint32_t ontime = 250;
  uint32_t offtime = 0;
  //ret = TMR_paramSet(rp, TMR_PARAM_READ_ASYNCONTIME, &ontime);
  ret = TMR_paramSet(rp, TMR_PARAM_READ_ASYNCOFFTIME,&offtime);
  
  // initialize the read plan 
  ret = TMR_RP_init_simple(&plan, antennaCount, antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
  checkerr(rp, ret, 1, "initializing the read plan");
  
  /* Commit read plan */
  ret = TMR_paramSet(rp, TMR_PARAM_READ_PLAN, &plan);
  checkerr(rp, ret, 1, "setting read plan");

  //valueList = { 865700,866300,866900,867500 };
  // This to set the Frequency 
  TMR_uint32List value1;
  uint32_t valueList[1] = { 865700};
  value1.max = numberof(valueList);
  value1.len = numberof(valueList);
  value1.list = valueList;
  ret = TMR_paramSet(rp, TMR_PARAM_REGION_HOPTABLE, &value1);
  checkerr(rp, ret, 1, "Setting Hoptable");

  // This to set the Power
  int32_t readpower=3000;

  rlb.listener = callback;
  rlb.cookie = NULL;

  reb.listener = exceptionCallback;
  reb.cookie = NULL;

  ret = TMR_addReadListener(rp, &rlb);
  checkerr(rp, ret, 1, "adding read listener");

  ret = TMR_addReadExceptionListener(rp, &reb);
  checkerr(rp, ret, 1, "adding exception listener");
  
  int value ;
  int menu1choice;
  printf("Hello. Welcome to the program.\n");
  printf("Menu \n");
  printf("------------------------------------\n");
  printf("1. Read Continuously for  5 seconds \n");
  printf("2. Read Continuously for  10 seconds \n");
  printf("3. Enter a different amount of time  for Continuous Reading \n");
  printf("4. 10 seconds Continuous Reading , Enter a Transmit power in range [0,30] \n");
  printf("5. 30 seconds Continuous Reading , Enter a Transmit power in range [0,30] \n");
  printf("6. Enter a different amount of time  for Continuous Reading , Enter a Transmit power in range [0,30] \n");
  scanf("%d", &menu1choice);
  switch ( menu1choice ) {
  case 1:
	value=5000;
	break;
  case 2:
        value=10000;
	break;
  case 3:
    printf("Enter the amount of time in seconds you want to Read Continuously:\n");
	scanf("%d", &value);
	value=value*1000; 
	break;  
  case 4:
    printf("Enter Transmit power [0,30]:\n");
	scanf("%d", &readpower);
	readpower=readpower*100;
	value=10000;
	break;
  case 5:
    printf("Enter Transmit power [0,30]:\n");
	scanf("%d", &readpower);
	readpower=readpower*100;
	value=30000;
	break;
  case 6:
    printf("Enter Transmit power [0,30]:\n");
	scanf("%d", &readpower);
	readpower=readpower*100;
    printf("Enter the amount of time in seconds you want to Read Continuously:\n");
	scanf("%d", &value);
	value=value*1000; 
	break; 	   
  default:
           printf("ERROR");
  }
  ret = TMR_paramSet(rp, TMR_PARAM_RADIO_READPOWER, &readpower);
  checkerr(rp, ret, 1, "setting read power");
  printf("Start Continuous Reading  \n");

  ret = TMR_startReading(rp);

  checkerr(rp, ret, 1, "starting reading");

  tmr_sleep(value);
  ret = TMR_stopReading(rp);
  checkerr(rp, ret, 1, "stopping reading");
  TMR_destroy(rp);
  int rett,req,tim;
  char oldname[] = "/home/debian/mercuryapi-1.31.3.36/c/src/samples//program.txt";
  char newname[100];
  req=readpower/100;
  tim=value/1000;
  sprintf(newname,"/home/debian/mercuryapi-1.31.3.36/c/src/samples//%dsecondsContinousReadingwith%ddBm.txt",tim, req);

  rett = rename(oldname, newname);
	
  if(rett == 0) {
     printf("File renamed successfully \n");
  } else {
     printf("Error: unable to rename the file \n");
  }
  return 0;


#endif /* TMR_ENABLE_BACKGROUND_READS */
}


void
callback(TMR_Reader *reader, const TMR_TagReadData *t, void *cookie)
{
   FILE *fptr;
   fptr = fopen("/home/debian/mercuryapi-1.31.3.36/c/src/samples//program.txt","a");
   if(fptr == NULL)
   {
      printf("Error!");   
      exit(1);             
   }  

  char epcStr[128];
  char timeStr[128];

      uint8_t shift;
      uint64_t timestamp;
      time_t seconds;
      int micros;
      char* timeEnd;
      char* end;

      shift = 32;
      timestamp = ((uint64_t)t->timestampHigh<<shift) | t->timestampLow;
      seconds = timestamp / 1000;
      micros = (timestamp % 1000) * 1000;
      timeEnd = timeStr + sizeof(timeStr)/sizeof(timeStr[0]);
      end = timeStr;
      end += strftime(end, timeEnd-end, "%F  %H:%M:%S", localtime(&seconds));
      end += snprintf(end, timeEnd-end, ".%06d", micros);
    
  	TMR_bytesToHex(t->tag.epc, t->tag.epcByteCount, epcStr);
  	printf("Antenna: %u	Frequency(kHz):%u     RSSI(dBm):%i     Read Count:%u      Date & Time:%s   Phase:%u     EPC:%s    \n" ,t->antenna ,t->frequency ,t->rssi ,t->readCount,timeStr,t->phase , epcStr );
  	fprintf(fptr,"%llu,%u,%s,%i,%u,%u \n",timestamp,t->antenna,epcStr,t->rssi,t->phase,t->readCount);
       fclose(fptr);
}

void 
exceptionCallback(TMR_Reader *reader, TMR_Status error, void *cookie)
{
  fprintf(stdout, "Error:%s\n", TMR_strerr(reader, error));
}