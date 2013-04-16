/*
 * yaepghd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Community Edition
 */

#include "Utils.h"

#include <vdr/tools.h>

/* Logging functions */
void
yaepg_error(const char *func, const char *fmt, ...)
{
   char eMsg[128], eLine[256];
   va_list ap;

   va_start(ap, fmt);
   vsnprintf(eMsg, sizeof(eMsg), fmt, ap);
   va_end(ap);
   snprintf(eLine, sizeof(eLine), "ERROR: YaEPGHD: %s: %s", func, eMsg);
   esyslog("%s", eLine);
}

void
yaepg_info(const char *func, const char *fmt, ...)
{
   char iMsg[128], iLine[256];
   va_list ap;

   va_start(ap, fmt);
   vsnprintf(iMsg, sizeof(iMsg), fmt, ap);
   va_end(ap);
   snprintf(iLine, sizeof(iLine), "INFO: YaEPGHD: %s: %s", func, iMsg);
   isyslog("%s", iLine);
}
