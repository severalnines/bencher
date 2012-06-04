/* Copyright (C) 2012 severalnines.com

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <float.h>
#include <limits.h>
#include "base64.h"


#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <getopt.h>


#include "errorhandling.h"
#include "util.h"

#define uint2korr(A)    (uint16) (((uint16) ((unsigned char) (A)[0])) +\
                                  ((uint16) ((unsigned char) (A)[1]) << 8)) 

char  g_database[256];
char  * ndbconnectstring=0;
char  mysqlhost[256];
char  mysqluser[256];
char  mysqlpass[256];
char  * mysqlquery=0;
char  * cleanupquery=0;
char  mysqlsocket[256];

bool verbose = false;
int execForceValue = 0;
int apiNodeId = 0;

uint g_percentile=95;
int g_max_queries = 10000;
int g_max_threads = 1;
int g_max_iter = 3;
int g_port =3306;
int g_batch=1;
uint g_runtime=0;
uint g_warmup_time=5*1000*1000;
int g_multi=0;
uint g_less_than_ms=5;
bool g_write_file=false;
const int64 sec2usec=1000*1000;
const char * thisFile = __FILE__;

char g_testname[256];

const int MAX_SAVE_SAMPLES=864000;

int g_len=256;

using namespace std;

// arguments to main()

void
print_help()
{
  printf("Usage: bencher  \n"
	 " -h --mysqlhost=mysql hostname (localhost)\n"	 
	 " -u --mysqluser=mysql username (root)\n"	 
	 " -p --mysqlpassword=mysql password (\"\")\n"	 
	 " -s --mysqlsocket=mysql socket (/tmp/mysql.sock)\n"
	 " -e --mysqlquery=query  (not set)\n" 	 
	 " -i --cleanupquery=query  (not set)\n" 	 
	 " -P --mysqlport=mysql port  (3306)\n" 	 
	 " -d --database=databaseName  (test)\n" 
	 " -t --threads=N  (1)\n" 
	 " -l --loops=N (10000)\n"
	 " -b --batch=N (1)\n"
	 " -r --runtime=N (specfies how many seconds the test should run, cannot be used with --loops)\n"
	 " -o --output=<testname>  (writes stats to two files for gnuplot)\n"
	 " -T --querytime-threshold=N  (5)\n" 
	 );
  exit(1);
}



int option(int argc, char** argv)
{
  int c;
  
  while (1)
    {
      static struct option long_options[] =
	{
	  {"help", 0, 0, '?'},
	  {"database", 1, 0, 'd'},
	  {"mysqlhost", 1, 0, 'h'},
	  {"mysqluser", 1, 0, 'u'},
	  {"mysqlpassword", 1, 0, 'p'},
	  {"mysqlsocket", 1, 0, 's'},
	  {"mysqlport", 1, 0, 'P'},
	  {"mysqlquery", 1, 0, 'e'},
	  {"cleanupquery", 1, 0, 'i'},
	  {"threads", 1, 0, 't'},
	  {"loops", 1, 0, 'l'},
	  {"batch", 1, 0, 'b'},
	  {"output", 1, 0, 'l'},
	  {"querytime-threshold", 1, 0, 'T'},
	  {"runtime", 1, 0, 'r'},	  
	  {"batch", 1, 0, 'b'},
	  {0, 0, 0, 0}
	};
      /* getopt_long stores the option index here.   */
      int option_index = 0;

      c = getopt_long (argc, argv, "?ad:c:t:l:P:p:u:h:s:e:T:r:b:w:o:mi:",
		       long_options, &option_index);

      /* Detect the end of the options.   */
      if (c == -1)
	{
	  break;
	}


      switch (c)
	{
	case 0:
	  /* If this option set a flag, do nothing else now.   */
	  if (long_options[option_index].flag != 0)
	    break;
	  printf ("option %s", long_options[option_index].name);
	  if (optarg)
	    printf (" with arg %s", optarg);
	  printf ("\n");	  
	  break;
	case 'c':
	  ndbconnectstring=(char*)malloc(256* sizeof(char*));
	  memset( ndbconnectstring,0,255);
	  strcpy(ndbconnectstring,optarg);
	  break;
	case 'e':
	  mysqlquery=(char*)malloc(strlen(optarg)+1);
	  memset(mysqlquery,0,sizeof(mysqlquery));
	  strcpy(mysqlquery,optarg);
	  break;
	case 'i':
	  cleanupquery=(char*)malloc(strlen(optarg)+1);
	  memset(cleanupquery,0,sizeof(cleanupquery));
	  strcpy(cleanupquery,optarg);
	  break;
	case 'h':
	  memset(mysqlhost,0,255);
	  strcpy(mysqlhost,optarg);
	  break;
	case 'p':
	  memset(mysqlpass,0,255);
	  strcpy(mysqlpass,optarg);
	  break;
	case 'u':
	  memset(mysqluser,0,255);
	  strcpy(mysqluser,optarg);
	  break;
	case 's':
	  memset(mysqlsocket,0,255);
	  strcpy(mysqlsocket,optarg);
	  break;
	case 'P':
	  g_port=atoi(optarg);
	  break;
	case 'l':
	  g_max_queries=atoi(optarg);
	  break;
	case 'b':
	  g_batch=atoi(optarg);
	  break;
	case 'r':
	  g_runtime=atoi(optarg);
	  break;
	case 'm':
	  g_multi=1;
	  break;
	case 't':
	  g_max_threads = atoi(optarg);
	  break;
	case 'T':
	  g_less_than_ms= atoi(optarg);
	  break;	  
	case 'd':
	  memset(g_database,0,255);
	  strcpy(g_database,optarg);
	  break;
	case 'o':
	  g_write_file=true;
	  memset(g_testname,0,255);
	  strcpy(g_testname,optarg);
	  break;
	case '?':
	  {
	    print_help();
	    exit(-1);
	    break;
	  }
	default:
	  printf("Wrong options given. Try '-?' for help\n");
	  exit(-1);
	  break;
	}
    }
  return 0;  
  
}

int comp(const void * a1,const void * b1)
{
  int * a=(int*)a1;
  int * b=(int*)b1;
  if (*a==*b)
    return 0;
  else
    if (*a < *b)
      return -1;
    else
      return 1;
}


/*typedef struct  my_threadData_t {
  my_threadData_t() :
    threadid(-1),
    numberOrRowsRetrieved(0), 
    startTimeMicrosec(0), 
    stopTimeMicrosec(0), 
    aggregateTime(0),
    tmax(0), 
    tmin(1000000), 
    less(0),
    exec_count(0),
    exec_time(0),
    elapsed_time(0),
    running(true)
  {
  }
  void addSample(uint delta);
  int threadid;
  int numberOrRowsRetrieved;
  int64 startTimeMicrosec;
  int64 stopTimeMicrosec;
  int64 startTimeMicrosecTrx;
  int64 stopTimeMicrosecTrx;
  int64 aggregateTime;
  int64 tmax;
  int64 tmin;
  int64 less;
  int64 exec_count;
  int64 exec_time;
  int elapsed_time;
  bool running; 
  int64 t_sec_avg[86400];
  int64 * t_responsetime;
}threadData_t;
*/
struct threadData_t
{
  int threadid;
  int ic;
  bool wrapped_ic;
  uint numberOrRowsRetrieved;
  uint64 startTimeMicrosec;
  uint64 stopTimeMicrosec;
  uint64 startTimeMicrosecTrx;
  uint64 stopTimeMicrosecTrx;
  uint64 aggregateTime;
  uint64 tmax;
  uint64 tmin;
  uint64 less;
  uint64 exec_count;
  uint64 exec_time;
  double sum_tps;
  double sumsum_tps;
  double average_tps;
  double var_tps;
  double var_lat;
  double average_lat;
  double sum_latency;
  double sumsum_latency;
  uint elapsed_5sec_time;
  uint elapsed_1sec_time;
  uint elapsed_time;
  uint sample_1sec_count;
  bool running; 
  int64 t_sec_avg[86400];
  int * t_responsetime;
  void init();
  void addSample(uint delta);
  void printStats(uint interval);
  uint getPercentile(uint percentile);
  void printStats2();
  void reset();
};

void threadData_t::init()
{
  threadid=-1;
  aggregateTime=numberOrRowsRetrieved=startTimeMicrosec=stopTimeMicrosec=aggregateTime=tmax=less=exec_count=exec_time=elapsed_time=sum_latency=sumsum_latency=0;
  average_tps=average_lat=var_lat=var_tps=0.0;
  sum_tps=sumsum_tps=0.0;
  tmin=10000000;
  tmax=0;
  ic=0;
  wrapped_ic=false;
  elapsed_5sec_time=0;
  elapsed_1sec_time=0;
  sample_1sec_count=0;
  running=false;
};


void threadData_t::addSample(uint delta)
{            
  exec_count++;
  aggregateTime+=delta;
  if( ic< 864000)
    {
      t_responsetime[ic]=delta;
      ic++;
    }
  else
    {
      wrapped_ic=true;
      ic=0;
    }
  if(delta < g_less_than_ms*1000)
    less++;
  if(tmax < delta)
    tmax=delta;
  if(tmin > delta)
      tmin=delta;
  elapsed_time+=delta;
  elapsed_5sec_time+=delta;
  elapsed_1sec_time+=delta;
  exec_time+=delta;    
  //  if(elapsed_1sec_time > 1000*1000)
  // { 
  //   double tps=(double)exec_count/(double)(aggregateTime/1000000);
  //   sum+=tps;
  //    sumsum+=sum*sum;      
  //   elapsed_1sec_time=0;
  //   sample_1sec_count++;
  //  } 
  double x=1000000.00/(double)delta;      
  sum_tps+=x;
  if(exec_count>0)
    average_tps=(double)sum_tps/(double)exec_count;    
  double xx=(x-average_tps);  
  sumsum_tps+=xx*xx;
  sum_latency+=delta;
  if(exec_count>0)
    average_lat=(double)aggregateTime/(double)exec_count;  
  double yy=(delta-average_lat);  
  sumsum_latency+=yy*yy;

  var_tps=sumsum_tps/(double)(exec_count);
  var_lat=sumsum_latency/(double)(exec_count);  
}  

uint threadData_t::getPercentile(uint percentile)
{
  int cnt=0;
  if(wrapped_ic)
    cnt=MAX_SAVE_SAMPLES;
  else
    cnt=ic;
  qsort(t_responsetime,cnt,sizeof(int),comp) ;      
  uint pos=cnt*percentile/100;
  return   t_responsetime[pos];
  
}
void threadData_t::printStats(uint interval)
{
  double tps=(double)exec_count/(double)(aggregateTime/1000000);
  double lat=(double)(aggregateTime)/(double)exec_count;
  if(elapsed_5sec_time > interval*1000*1000)
    {

      fprintf(stderr,"Thread %3d - tps: %2.2f (stdev=%2.2f) latency: %2.2f us (stdev=%2.2f) %dth: %d us (averages measured after %llu secs) exec_count=%llu aggregate_time=%llu\n", 
	      threadid, 
	      tps,
	      sqrt(var_tps),
	      lat,
	      sqrt(var_lat),
	      g_percentile,
	      getPercentile(g_percentile),
	      aggregateTime/(unsigned long long)1000000,
	      exec_count,
	      aggregateTime
	      );  
      elapsed_5sec_time=0;
    }
}



void threadData_t::printStats2()
{
  double tps=(double)exec_count/(double)(aggregateTime/1000000);
  double lat=(double)(aggregateTime)/(double)exec_count;
  fprintf(stderr, "Thread %3d - tps: %2.2f (stdev=%2.2f) avg: %2.2f (stdev=%2.2f), %dth: %d us, max: %llu us, min %llu us, "
	  "less than %d ms: %llu of %llu\n", threadid ,
	  tps,	   
	  sqrt(var_tps),	  
	  lat,
	  sqrt(var_lat),
	  g_percentile,
	  getPercentile(g_percentile),
	  tmax,
	  tmin ,
	  g_less_than_ms,
	  less,
	  g_runtime ? exec_count : g_max_queries);
}



void threadData_t::reset()
  {             
    aggregateTime=0;
    less=0;
    tmax=0;
    tmin=0;
    sum_tps=0;
    sumsum_tps=0;
    elapsed_time=0;
    aggregateTime=numberOrRowsRetrieved=startTimeMicrosec=stopTimeMicrosec=aggregateTime=tmax=less=exec_count=exec_time=elapsed_time=sum_latency=sumsum_latency=0;
    average_tps=average_lat=var_lat=var_tps=0.0;
    sum_tps=sumsum_tps=0.0;
    tmin=10000000;
    tmax=0;
    ic=0;
    exec_count=0;
    exec_time=0;
    elapsed_5sec_time=0;
    elapsed_1sec_time=0;
    sample_1sec_count=0;
  }  


typedef struct my_iteration_stats_t
{
  double tps;
  double lat;
  double stdev_tps;
  double stdev_lat;
  int64 tmax; 
  int64 tmin;
  uint percentile;
} iteration_stats_t;


// macro and data to help strncasecmp look for an argument prefix
int argPrefixLen;
#define strncasecmpARGPREFIX(arg) (arg), (argPrefixLen = strlen(arg))


const int64 MAX_INT64 = 9223372036854775807LL;
const int64 MIN_INT64 = -9223372036854775807LL; 
   // number of bytes for a date field

int64 startOfProgramMicrosec;
int64 startOfThreadsMicrosec;


int get_no_clusterconnections()
{
  MYSQL mysql;
  
  mysql_init(&mysql);
  int cluster_conn=0;
  
  
  if(!mysql_real_connect(&mysql, 
			mysqlhost,
			 mysqluser,
			 mysqlpass,
			g_database,
			 g_port, 
			 mysqlsocket,
			 0))     
    {
      fprintf(stderr,"%s\n", mysql_error(&mysql));
      exit(0);      
    }
  char query[128];
  sprintf(query,"select variable_value from information_schema.global_variables where variable_name='NDB_CLUSTER_CONNECTION_POOL'");
  if(mysql_real_query( &mysql, query, strlen(query) ))
    {
      fprintf(stderr,"%s\n", mysql_error(&mysql));
      exit(0);      
    }
  else
    {
      MYSQL_RES * result= mysql_store_result(&mysql);
      if (result)  // there are rows                                                                                                                               
	{

	  MYSQL_ROW row;
	  char msg[6400];
	  memset(msg,0,sizeof(msg));
	  unsigned long long num_rows = mysql_affected_rows(&mysql);
	  if((int)num_rows>0)
	    {
	      while ((row = mysql_fetch_row(result)))
		{
		  cluster_conn=atoi(row[0]);
		}
	    }
	}
      if(result)
	mysql_free_result(result);
    }
  mysql_close(&mysql);
  return cluster_conn;
}


int db_cleanup_before_iter( char * query)
{
  MYSQL mysql;
  
  mysql_init(&mysql);
  int cluster_conn=0;
  
  
  if(!mysql_real_connect(&mysql, 
			mysqlhost,
			 mysqluser,
			 mysqlpass,
			g_database,
			 g_port, 
			 mysqlsocket,
			 0))     
    {
      fprintf(stderr,"%s\n", mysql_error(&mysql));
      exit(0);      
    }
  if(mysql_real_query( &mysql, query, strlen(query) ))
    {
      fprintf(stderr,"%s\n", mysql_error(&mysql));
      exit(0);      
    }
  else
    {
      int status=0;
      MYSQL_RES * result;
      /* process each statement result */
      do {
	/* did current statement return data? */
	result = mysql_store_result(&mysql);
	if (result)
	  {
	    //  fprintf(stderr,"%lld rows affected  id=%i\n",
	    //   mysql_affected_rows(&mysql),i);
	    /* yes; process rows and free the result set */
	    mysql_free_result(result);
	  }
	else          /* no result set or error */
	  {
	    if (mysql_field_count(&mysql) == 0)
	      {
		///    fprintf(stderr,"%lld rows affected  id=%i\n",
		//   mysql_affected_rows(&mysql),i);
	      }
		else  /* some error occurred */
		  {
		    fprintf(stderr,"Could not retrieve result set\n");
		    break;
		  }
	  }
	/* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
	if ((status = mysql_next_result(&mysql)) > 0)
	  fprintf(stderr,"Could not execute statement\n");
      } while (status == 0);      
    }
  mysql_close(&mysql);
  return cluster_conn;
}


void printSummary()
{

}


 // @return 0 when successful; non-0 otherwise.
 // exits for database problem
void *
thread_runner_mysql (void * t)
{
  threadData_t * ctx = (threadData_t *)t;
  bool stopTest=false;
  int delta=0;
  ctx->startTimeMicrosec=JULIANTIMESTAMP();	 
  MYSQL mysql;

  bool no_cmdline_query=false;

  char * mysqlquery2=0;
  char * query=0;
  if(!mysqlquery)
    {      
      mysqlquery2=(char*)malloc(512);
      no_cmdline_query=true;
    }
    
  int i=0;
  int len=g_len;
  char data[len];
  memset(data, 'a', sizeof(data));
 /*
create table t1(id integer primary key auto_increment, tid integer, data1 varchar(512), data2 varchar(512)) engine=ndb;
*/
  int warmup=1;
  
  if(warmup)
    fprintf(stderr,"Thread %d - warming up for %d seconds\n",ctx->threadid, g_warmup_time/(1000*1000));
  ctx->exec_count=0;
  ctx->aggregateTime=0;

  while(true)
    {
      ctx->startTimeMicrosecTrx=JULIANTIMESTAMP();	 
    retry:
      mysql_init(&mysql);
      
      
      if(!mysql_real_connect(&mysql, 
			     mysqlhost,
			     mysqluser,
			     mysqlpass,
			     g_database,
			     g_port, 
			     mysqlsocket,
			     CLIENT_MULTI_STATEMENTS))
	{
	  
	  fprintf(stderr,"%s\n", mysql_error(&mysql));
	  stopTest=true;
	  usleep(500*1000);
	  goto retry;
	}
      
      

      if(i==g_max_queries && !g_runtime)
	break;
      

      
      if(no_cmdline_query)
	{
	  /**
	     your code goes here
	   */
	  int random=1 + (int) (1000000.0 * (rand() / (RAND_MAX + 1.0)));
	  bzero(mysqlquery2, 512);
	  sprintf(mysqlquery2, "SELECT %d", random);        
	  query=mysqlquery2;
	}
      else
	{
	  query=mysqlquery;
	}
      if(!mysql_real_query( &mysql, query, strlen(query) ))
	{
	  int status=0;
	  MYSQL_RES * result;
	  /* process each statement result */
	  do {
	    /* did current statement return data? */
	    result = mysql_store_result(&mysql);
	    if (result)
	      {
		//  fprintf(stderr,"%lld rows affected  id=%i\n",
		//   mysql_affected_rows(&mysql),i);
		/* yes; process rows and free the result set */
		mysql_free_result(result);
	      }
	    else          /* no result set or error */
	      {
		if (mysql_field_count(&mysql) == 0)
		  {
		    ///    fprintf(stderr,"%lld rows affected  id=%i\n",
		    //   mysql_affected_rows(&mysql),i);
		  }
		else  /* some error occurred */
		  {
		    fprintf(stderr,"Could not retrieve result set\n");
		    break;
		  }
	      }
	    /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
	    if ((status = mysql_next_result(&mysql)) > 0)
	      fprintf(stderr,"Could not execute statement\n");
	  } while (status == 0);
	}       
      else // failed
	{
	  int errno=mysql_errno(&mysql);
	  ctx->stopTimeMicrosecTrx=JULIANTIMESTAMP();
	  fprintf(stderr,"%s (%d) - Query failed - problematic query: %s\n",
		 mysql_error(&mysql),
		 errno, 
		 query);
	  mysql_close(&mysql);
	  if(errno == 1297)
	    {
	      /*temporary error - retry*/
	      fprintf(stderr,"Thread %d - retrying transaction\n", ctx->threadid);
	      usleep(20*1000);
	      goto retry;
	    }
	  usleep(500*1000);
	  goto retry;
	}
      

      /*
       * Below is for statistics collection - no queries after this line
       */
      
      

      if(!warmup)
	mysql_close(&mysql);  
      
      ctx->stopTimeMicrosecTrx=JULIANTIMESTAMP();
      delta = (ctx->stopTimeMicrosecTrx - ctx->startTimeMicrosecTrx);

      ctx->addSample(delta);            

      if(warmup)
	{
	  if(ctx->aggregateTime>g_warmup_time)
	    {
	      fprintf(stderr,"Thread %d - warmup complete\n",ctx->threadid);
	      //ctx->aggregateTime=0;
	      warmup=0;
	      //elapsed_time=0;	
	      ctx->reset();
	    }
	  goto retry;
	}
      
      ctx->printStats(5);
      
      if(g_runtime)
	{
	  //	  uint64 current=JULIANTIMESTAMP();
	  
	  //	  uint runtime = (uint)((micros - ctx->startTimeMicrosec)/1000*1000);
	  //	if(exec_time_secs>g_runtime)
	  uint runtime=ctx->aggregateTime / 1000 /1000;
	  if(runtime>g_runtime)
	    {	  
	      break;
	    }
	}
      i++;
    }

  ctx->stopTimeMicrosec=JULIANTIMESTAMP();	 
  return 0;    
  
}



int 
main (int argc, char ** argv)
{
  startOfProgramMicrosec = JULIANTIMESTAMP();

  strcpy(g_database,"test");
  strcpy(mysqlhost, "localhost");
  strcpy(mysqluser, "root");
  strcpy(mysqlpass, "");
  strcpy(mysqlsocket, "/tmp/mysql.sock");

  option(argc,argv);

  /**
   * This should be in shared mem area of Apache
   * Once for each apache instance, not for each child
   */
  
  vector<pthread_t> threads;
  threads.resize (g_max_threads);
  
  vector<threadData_t> tdata;
  tdata.resize (g_max_threads);

  iteration_stats_t iteration_stats[g_max_iter];

    for(int i=0; i<g_max_threads ;i++) 
      {
	threadData_t & thistdata = tdata[i];
	thistdata.threadid=i;
	thistdata.numberOrRowsRetrieved=0;
	thistdata.startTimeMicrosec = MAX_INT64;
	thistdata.stopTimeMicrosec = MIN_INT64;
	for(int k=0;k<86400;k++)
	  {
	    thistdata.t_sec_avg[k]=0LL;	
	  }
	thistdata.t_responsetime=(int*)malloc(sizeof(int)*MAX_SAVE_SAMPLES);
	memset(thistdata.t_responsetime,0, sizeof(int)*MAX_SAVE_SAMPLES);
      }
    
    int iter=0;
    while(iter < g_max_iter)
      {
	if(cleanupquery!=0)
	  db_cleanup_before_iter(cleanupquery);
	
	fprintf(stderr,"\n------- Starting Benchmark - iteration  (%d / %d) ----------\n", iter+1, g_max_iter);
	for(int i=0; i<g_max_threads;i++) 
	  {
	      pthread_create(&threads[i], NULL, thread_runner_mysql, &tdata[i]);	
	  }      
	
	startOfThreadsMicrosec=JULIANTIMESTAMP();
	for (int i = 0; i < g_max_threads; i++)
	  {
	    pthread_join(threads[i], NULL);
	  }
	
	uint64 iter_tmax=0LL;
	uint64 iter_tmin=ULLONG_MAX;
	double totTps=0;	
	double totLat=0;	
	double totTpsDev=0;	
	double totLatDev=0;	
	double totPercentile=0;
	fprintf(stderr,"\n------- Benchmark Finished - iteration (%d / %d )----------\n", iter+1, g_max_iter);
	for (int i = 0; i < g_max_threads; ++i) 
	  {
	    if(tdata[i].tmin < iter_tmin)
	      iter_tmin=tdata[i].tmin;
	    if(tdata[i].tmax > iter_tmax)
	      iter_tmax=tdata[i].tmax;

	    tdata[i].printStats2();
	    
	    totTps+=(double)tdata[i].exec_count/(double)(tdata[i].aggregateTime/1000000);
	    totTpsDev+=sqrt(tdata[i].var_tps);
	    totLat+=(double)(tdata[i].aggregateTime)/(double)tdata[i].exec_count;
	    totLatDev+=sqrt(tdata[i].var_lat);
	    totPercentile+=tdata[i].getPercentile(g_percentile);
	    
	  }
	iteration_stats[iter].tmax=iter_tmax;
	iteration_stats[iter].tmin=iter_tmin;
	iteration_stats[iter].tps=totTps;
	iteration_stats[iter].percentile=totPercentile/(double)g_max_threads;
	iteration_stats[iter].lat=totLat/(double)g_max_threads;
	iteration_stats[iter].stdev_tps=totTpsDev/(double)g_max_threads;
	iteration_stats[iter].stdev_lat=totLatDev/(double)g_max_threads;
	iter++;
      }
    double tot_tps_average=0;
    double tot_lat_average=0;
    double tot_stdev_lat_average=0;
    double tot_stdev_tps_average=0;
    double tot_percentile_average=0;

    for(int i=0; i<g_max_iter; i++)
      {	
	//	fprintf(stderr,"%2.2f\n",iteration_stats[i].tps);
	tot_tps_average+=iteration_stats[i].tps;
	tot_lat_average+=iteration_stats[i].lat;
	tot_stdev_tps_average+=iteration_stats[i].stdev_tps;
	tot_stdev_lat_average+=iteration_stats[i].stdev_lat;
	tot_percentile_average+=iteration_stats[i].percentile;
      }
    tot_tps_average=tot_tps_average/(double)g_max_iter;
    tot_lat_average=tot_lat_average/(double)g_max_iter;
    tot_percentile_average=tot_percentile_average/(double)g_max_iter;
    tot_stdev_tps_average=tot_stdev_tps_average/(double)g_max_iter;
    tot_stdev_lat_average=tot_stdev_lat_average/(double)g_max_iter;

    
    fprintf(stderr,"\nSummary:\n");
    fprintf(stderr,"--------------------------\n");
    fprintf(stderr, "Average Throughput = %2.2f tps (stdev=%2.2f)\nAverage Latency (us)=%2.2f (stdev=%2.2f)\n%dth Percentile (us)=%2.2f\n\n", tot_tps_average, tot_stdev_tps_average , tot_lat_average, 
	    tot_stdev_lat_average, g_percentile,tot_percentile_average );
    

    int ndb_cluster_conn=get_no_clusterconnections();
    
    if(ndb_cluster_conn==0)
      {
	fprintf(stderr, "Could not retrieve the number of cluster connections used\n");
	exit(-1);
      }
    bool empty_file=false;
    if(g_write_file)
     { 

       struct stat stat_record;
       FILE * fp = fopen(g_testname, "a");
       int fd=0;
       char line[128];
       memset(line,0,sizeof(line));
       if(fp==0)
	 {
	   fprintf(stderr,"Could not open file\n");
	   exit(1);
	 }
       fd=fileno(fp);
       if (fstat(fd, &stat_record) == 0)
	 {
	   if (stat_record.st_size == 0) empty_file=true;
	 }
       

       //multiconn, threads, avg tps, stdv
       if(empty_file)
	 {
	   sprintf(line,"#clusterconnections,threads,average tps, stdev tps, average latency, stdev lat,%dth percentile\n", 
		   g_percentile);
	   fputs(line,fp);
	 }
       sprintf(line,"%d,%d,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f\n",ndb_cluster_conn,g_max_threads,tot_tps_average,tot_stdev_tps_average,
	       tot_lat_average,tot_stdev_lat_average,tot_percentile_average);	
       fputs(line,fp);
       fclose(fp);
       fprintf(stderr,"\nWrote output to '%s'.\n", g_testname);
     }
    else
      {
	fprintf(stderr,"For spreadsheats (use -o <filename> to write data into a csv file):\n");
	fprintf(stdout,"#clusterconnections,threads,average tps, stdev tps, average latency, stdev lat, %dth percentile\n",
		g_percentile);
	fprintf(stdout, "%d,%d,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f\n",ndb_cluster_conn,g_max_threads,tot_tps_average,tot_stdev_tps_average,
		tot_lat_average,tot_stdev_lat_average,tot_percentile_average);	
      }
    
#if 0
    
    fprintf(stderr, "\nTotal throughput = %2.2f tps\n", totQps);
    fprintf(stderr, "Average exec time per thread = %2.2f secs\n\n",
	    (double)totTime/(double)(1000*1000*g_max_threads));

    if(g_write_file)
     { 
        int fp = fopen(g_testname, "wa");
	char line[128];
	memset(line,0,sizeof(line));
	if(fp==0)
	  {
	    fprintf(stderr,"Could not open file\n");
	    exit(1);
	  }
	//multiconn, threads, avg tps, stdv
	sprintf(line,"%d,%d,%2.2%f,%2.2f\n",ndb_cluster_conn,g_max_threads,tot_tps_average,tot_stdev_tps_average);	
	fputs(line,fp);
#if 0
	char fspath[128];
	char filename1[128];
	char filename2[128];
	char filename3[128];
	char fname1[128];
	char fname2[128];
	char fname3[128];
	strcpy(fspath,"./");
	sprintf(filename1,"%s/bencher_t%d_%s_tp.dat", fspath, g_max_threads, g_testname);
	sprintf(filename2,"%s/bencher_t%d_%s_rt.dat", fspath, g_max_threads, g_testname);
	sprintf(filename3,"%s/gnuplot_t%d_%s.script", fspath, g_max_threads, g_testname);
	sprintf(fname1,"bencher_t%d_%s_tp.dat", g_max_threads, g_testname);
	sprintf(fname2,"bencher_t%d_%s_rt.dat", g_max_threads, g_testname);
	sprintf(fname3,"gnuplot_t%d_%s.script", g_max_threads, g_testname);
	FILE *fp1, *fp2, *fp3;
	char line1[128];
	char line2[128];
	char line3[128];
	fp1 = fopen(filename1, "w");
	fp2 = fopen(filename2, "w");
	fp3 = fopen(filename3, "w");
	if(fp1==0)
	  {
	    fprintf(stderr,"Could not open file\n");
	    exit(1);
	  }
	if(fp2==0)
	  {
	    fprintf(stderr,"Could not open file\n");
	    exit(1);
	  }

	if(fp3==0)
	  {
	    fprintf(stderr,"Could not open file\n");
	    exit(1);
	  }
	
	int runtime=0;    
	if(g_runtime)
	  runtime=g_runtime;
	else
	  runtime=total_time;    
	int64 aggregate_tp[runtime];
	int64 aggregate_rt[runtime*10];
	memset(aggregate_tp,0,runtime*sizeof(int64));
	memset(aggregate_rt,0,10*runtime*sizeof(int64));
	int64 min= MAX_INT64;
	int64 max= MIN_INT64;
	int64 avg_lat=0;
	int64 overall_max=MIN_INT64;
	
	for (int j=0;j<runtime;j++)
	  {
	    for (int i = 0; i < g_max_threads; i++) 
	      {       
		aggregate_tp[j]+=tdata[i].t_sec_avg[j];
	      }
	    sprintf(line1,"%d\t%llu\n", j,aggregate_tp[j]);
	    fputs(line1,fp1);
	  }
	
	for (int j=0;j<(runtime*10);j++)
	  {
	    for (int i = 0; i < g_max_threads; i++) 
	      {       
		aggregate_rt[j]+=tdata[i].t_responsetime[j];
		if(tdata[i].t_responsetime[j]>max && tdata[i].t_responsetime[j]>0)
		  max=tdata[i].t_responsetime[j];
		if(tdata[i].t_responsetime[j]<min && tdata[i].t_responsetime[j]>0)
		  min=tdata[i].t_responsetime[j];
	      }
	    avg_lat=aggregate_rt[j]/g_max_threads;
	    sprintf(line2,"%d\t%llu\t%llu\t%llu\n", j,
		    avg_lat,min,max);
	    fputs(line2,fp2);
	    if(max>overall_max)
	      overall_max=max;
	    min= MAX_INT64;
	    max= MIN_INT64;
	    
	  }
	fclose(fp1);
	fclose(fp2);

	float tmp_max_lat=(float)overall_max*1.2;
	float tmp_tot_qps=(float)totQps*1.2;
	
	fputs("set term png small\n",fp3);
	fputs("set data style lines\n",fp3);
	fputs("set grid\nset multiplot\n",fp3);
	fputs("set size 1,0.5\n",fp3);
	fputs("set origin 0,0.5\n",fp3);
	sprintf(line3,"set yrange [ 0 : %d ]\n", (int) (tmp_tot_qps));
	sprintf(line3,"set xrange [ 0 : %d ]\n", (int) (runtime-1));
	fputs(line3,fp3);
	sprintf(line3,"set title \"Test case: %s - throughput\"\n", g_testname);
	fputs(line3,fp3);
	fputs("set xlabel \"seconds\"\n",fp3);
	fputs("set ylabel \"tps\"\n",fp3);
	sprintf(line3,"plot \"%s\" using 1:($2) with lines title \"throughput\"\\\n", fname1);
	fputs(line3,fp3);
	fputs("\n\nset size 1,0.5\n",fp3);
	fputs("set origin 0,0\n",fp3);
	sprintf(line3,"set yrange [ 0.1 : %lld ]\n", (unsigned long long) (tmp_max_lat));
	fputs(line3,fp3);
	sprintf(line3,"set title \"Test case: %s - latency\"\n", g_testname);
	fputs(line3,fp3);
	fputs("set logscale y\n",fp3);
	fputs("set xlabel \"seconds\"\n",fp3);
	fputs("set ylabel \"latency (us)\"\n",fp3);

	sprintf(line3,"plot \"%s\" using ($1/10):2 with points title \"avg\"\\\n", fname2);
	fputs(line3,fp3);
	sprintf(line3,", \"%s\" using ($1/10):3 with points title \"min\"\\\n", fname2);
	fputs(line3,fp3);
	sprintf(line3,", \"%s\" using ($1/10):4 with points title \"max\"\\\n", fname2);
	fputs(line3,fp3);
	
	/*sprintf(stderr,line3,"plot \"%s\" using 1:($2) with points title \"avg\"\\\n", fname2);
	fputs(line3,fp3);
	sprintf(stderr,line3,", \"%s\" using 1:($3) with points title \"min\"\\\n", fname2);
	fputs(line3,fp3);
	sprintf(stderr,line3,", \"%s\" using 1:($4) with points title \"max\"\\\n", fname2);
	fputs(line3,fp3);
*/

	fclose(fp3);
	fprintf(stderr,"Wrote '%s' and '%s'\n", filename1, filename2);
	fprintf(stderr,"The '*_tp.dat' contains throughput data,"
	       "and '*_rt.dat' contains response time data\n");
	fprintf(stderr,"Wrote '%s' to be used with gnuplot ( 'gnuplot %s > %s.png') \n",
	       filename3,filename3,g_testname);
#endif
      }
#endif
    exit(0); 
}

    
