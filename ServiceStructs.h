/*
 * yaepghd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Community Edition
 */

#pragma once

#include <vdr/epg.h>
#include <vdr/tools.h>

// Data structure for service "Epgsearch-switchtimer-v1.0"
struct Epgsearch_switchtimer_v1_0
{
// in
      const cEvent* event;
      int mode;                  // mode (0=query existance, 1=add/modify, 2=delete)
// in/out
      int switchMinsBefore;
      int announceOnly;
// out
      bool success;              // result
};

// Data structure for RemoteTimers services
struct RemoteTimers_Event_v1_0 {
//in
        const cEvent    *event;
//out
        cTimer          *timer;
        cString         errorMsg;
};

struct RemoteTimers_GetMatch_v1_0 {
//in
	const cEvent	*event;
//out
	cTimer		*timer;
	int		timerMatch;
	int		timerType;
	bool		isRemote;
};

struct RemoteTimers_Timer_v1_0 {
//in+out
	cTimer		*timer;
//out
	cString		errorMsg;
};

