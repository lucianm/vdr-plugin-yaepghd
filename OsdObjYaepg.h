/*
 * yaepghd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Community Edition
 */

#pragma once

#include "GuiElements.h"

/*
 *****************************************************************************
 * cOsdObjYaepg
 *****************************************************************************
 */
class cOsdObjYaepg : public cOsdObject {
private:
   cYaepgTheme *theme;
   cOsd *osd;
   time_t startTime;
   tArea mainWin;
   cBitmap *mainBmp;
   cRect videoWindowRect;
   std::vector< cChannel * > chanVec;
   const cEvent *event;
   cTimeMs lastInput;
   int directChan;
   bool needsRedraw;

   cYaepgGrid *gridEvents;
   cYaepgGridChans *gridChans;
   cYaepgGridTime *gridTime;
   cYaepgGridDate *gridDate;
   cYaepgEventTitle *eventTitle;
   cYaepgEventInfo *eventInfo;
   cYaepgEventTime *eventTime;
   cYaepgEventDesc *eventDesc;
   cYaepgEventDate *eventDate;
   cYaepgEventEpgImage *eventEpgImage;
   cYaepgTimeLine *timeLine;
   cYaepgHelpBar *helpBar;
   cYaepgRecDlg *recordDlg;
   cYaepgMsg *messageBox;
   uint64_t msgBoxStart;

public:
   cOsdObjYaepg(void);
   ~cOsdObjYaepg();
   virtual void Show(void);
   virtual eOSState ProcessKey(eKeys key);
   void SetTime(time_t newTime);
   void UpdateChans(cChannel *c);
   void UpdateChans(int change);
   void UpdateTime(int change);
   void UpdateEvent(const cEvent *newEvent);
   void MoveCursor(eCursorDir dir);
   void SwitchToCurrentChannel(bool closeVidWin = false);
   void AddDelTimer(void);
   void AddDelSwitchTimer(void);
   void AddDelRemoteTimer(void);
   void Draw(void);
};