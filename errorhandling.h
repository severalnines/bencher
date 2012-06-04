#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#include <mysql.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <limits>

using namespace std;

/**
 * Helper sleep function
 */
static void
milliSleep(int m_seconds)
{

  struct timeval t;
  t.tv_sec=  m_seconds / 1000000L;
  t.tv_usec= m_seconds % 1000000L;
  select(0,0,0,0,&t); /* sleep */
}

#define PRINT_ERROR(code,msg) \
    cout << "Error in " << __FILE__ << ", line: " << __LINE__ \
	     << ", code: " << code \
	     << ", msg: " << msg << "." <<  endl
			 			 
#define MYSQLERROR(mysql) { \
   PRINT_ERROR(mysql_errno(&mysql),mysql_error(&mysql)); \
   exit(-1); }
#ifdef NDB
#define APIERROR(error) { \
   if ((error).code != 0) \
   { \
   PRINT_ERROR(error.code,error.message); \
   if (error.code != 0) { exit(-1); } } \
   }
#endif
const int millisecRecoverySleep = 10;
#ifdef NDB
bool errorHandler(int line, NdbTransaction * trans, Ndb* ndb);
#endif
#endif
