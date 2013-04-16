/*
 * yaepghd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Community Edition
 */

#include "OsdObjYaepg.h"

#include "Utils.h"
#include "MenuSetupYaepg.h"
#include "ServiceStructs.h"

#include <vdr/device.h>
#include <vdr/plugin.h>
#include <vdr/remote.h>

/*
 *****************************************************************************
 * cOsdObjYaepg
 *****************************************************************************
 */
cOsdObjYaepg::cOsdObjYaepg(void) :
   theme(NULL),
   osd(NULL),
   startTime((time_t)0),
   mainBmp(NULL),
   event(NULL),
   lastInput(),
   directChan(0),
   needsRedraw(false),
   gridEvents(NULL),
   gridChans(NULL),
   gridTime(NULL),
   gridDate(NULL),
   eventTitle(NULL),
   eventInfo(NULL),
   eventTime(NULL),
   eventDesc(NULL),
   eventDate(NULL),
   eventEpgImage(NULL),
   timeLine(NULL),
   helpBar(NULL),
   recordDlg(NULL),
   messageBox(NULL)
{
   memset(&mainWin, 0, sizeof(mainWin));
   chanVec.clear();
}

cOsdObjYaepg::~cOsdObjYaepg()
{
   delete osd;
   delete mainBmp;
   delete gridEvents;
   delete gridChans;
   delete gridTime;
   delete gridDate;
   delete timeLine;
   delete eventTitle;
   delete eventInfo;
   delete eventTime;
   delete eventDesc;
   delete eventDate;
   if (eventEpgImage)
      delete eventEpgImage;
   delete helpBar;
   delete recordDlg;
   delete messageBox;
   cDevice::PrimaryDevice()->ScaleVideo(); // rescale to full size
#ifdef YAEPGHD_REEL_EHD
   reelVidWin->Close();
#endif
}

void
cOsdObjYaepg::Show(void)
{
   /* Create new OSD object */
   osd = cOsdProvider::NewOsd(0, 0);
   if (osd == NULL) {
      YAEPG_ERROR("NewOsd returned NULL!");
      return;
   }

   /* Load the theme */
   theme = cYaepgTheme::Instance();
   if (theme->Load(sThemeName) == false) {
      YAEPG_ERROR("Error loading theme %s", sThemeName.c_str());
      return;
   }

   /* Create the main window and bitmap for drawing the EPG */
   YAEPG_INFO("Main window (%d %d)", BG_IMAGE->Width(), BG_IMAGE->Height());
   mainWin.x1 = 0;
   mainWin.y1 = 0;
   mainWin.x2 = BG_IMAGE->Width() - 1;
   mainWin.y2 = BG_IMAGE->Height() - 1;
   mainWin.bpp = BG_IMAGE->Bpp();
   mainBmp = new cBitmap(BG_IMAGE->Width(),
                         BG_IMAGE->Height(),
                         BG_IMAGE->Bpp());
   osd->SetAreas(&mainWin, 1);

   /* Set up the video window parameters */
   if (VID_WIN_GEOM.w != 0 && VID_WIN_GEOM.h != 0) {
      // ask the output device to scale the video when next flushing the OSD, if it supports this
      cRect vidWinRect(
         VID_WIN_GEOM.x,
         VID_WIN_GEOM.y,
         VID_WIN_GEOM.w,
         VID_WIN_GEOM.h);
      videoWindowRect = cDevice::PrimaryDevice()->CanScaleVideo(vidWinRect);
   }

#ifdef YAEPGHD_REEL_EHD
   reelVidWin->Open(VID_WIN_GEOM);
#endif

   /* Create all the EPG widgets based on current channel/time */
   UpdateChans(Channels.GetByNumber(cDevice::CurrentChannel()));

   time_t t = time(NULL);
   gridEvents = new cYaepgGrid(chanVec, t);
   gridChans = new cYaepgGridChans(chanVec);
   gridTime = new cYaepgGridTime(t);
   gridDate = new cYaepgGridDate(t);
   timeLine = new cYaepgTimeLine(t);
   const cEvent *e = gridEvents->Event();
   eventTitle = new cYaepgEventTitle(e);
   eventInfo = new cYaepgEventInfo(e);
   eventTime = new cYaepgEventTime(e);
   eventDesc = new cYaepgEventDesc(e);
   eventDate = new cYaepgEventDate();
   if (iEpgImages)
      eventEpgImage = new cYaepgEventEpgImage(e);
   helpBar = new cYaepgHelpBar();
   recordDlg = NULL;
   messageBox = NULL;

   if (iRemoteTimer && pRemoteTimers) {
      cString errorMsg;
      if (!pRemoteTimers->Service("RemoteTimers::RefreshTimers-v1.0", &errorMsg)) {
         messageBox = new cYaepgMsg();
         messageBox->UpdateMsg((char*)*errorMsg);
         msgBoxStart = cTimeMs::Now();
      }
   }

   Draw();
}

void
cOsdObjYaepg::AddDelTimer(void)
{
    const cEvent *event=gridEvents->Event();
    eTimerMatch timerMatch = tmNone;
    cTimer *ti;
    ti=Timers.GetMatch(event, &timerMatch);
    if (timerMatch==tmFull)
    {
        if (ti)
        {
            ti->OnOff();
        }
    }
    else
    {
        cTimer *timer = new cTimer(event);
        cTimer *t = Timers.GetTimer(timer);
        if (t) {
            t->OnOff();
            delete timer;
        }
        else {
            Timers.Add(timer);
        }
    }
    Timers.SetModified();
    eventInfo->UpdateEvent(event);
}

void
cOsdObjYaepg::AddDelSwitchTimer()
{
    const cEvent *event = gridEvents->Event();
    bool SwitchTimerExits = false;
    if (pEPGSearch && event) {
        Epgsearch_switchtimer_v1_0* serviceData = new Epgsearch_switchtimer_v1_0;
        serviceData->event = event;
        serviceData->mode = 0;
        if (pEPGSearch->Service("Epgsearch-switchtimer-v1.0", serviceData)){
            SwitchTimerExits=serviceData->success;
            delete serviceData;
        }
        else {
            delete serviceData;
            YAEPG_ERROR("Epgsearch-switchtimer-v1.0: EPGSearch does not support this service!");
            return;
        }
        if (!SwitchTimerExits) {
            serviceData = new Epgsearch_switchtimer_v1_0;
            serviceData->event = event;
            serviceData->mode = 1;
            serviceData->switchMinsBefore = iSwitchMinsBefore;
            serviceData->announceOnly = false;
            if (pEPGSearch->Service("Epgsearch-switchtimer-v1.0", serviceData)){
                if (serviceData->success) {
                    messageBox = new cYaepgMsg();
                    const char *msgtext;
                    msgtext = tr("Switch timer added");
                    messageBox->UpdateMsg(msgtext);
                    msgBoxStart = cTimeMs::Now();
                    needsRedraw = true;
                    delete serviceData;
                }
            }
            else {
                delete serviceData;
                YAEPG_ERROR("Epgsearch-switchtimer-v1.0: EPGSearch does not support this service!");
                return;
            }
        }
        else {
            serviceData = new Epgsearch_switchtimer_v1_0;
            serviceData->event = event;
            serviceData->mode = 2;
            if (pEPGSearch->Service("Epgsearch-switchtimer-v1.0", serviceData)){
                if (serviceData->success) {
                   messageBox = new cYaepgMsg();
                   const char *msgtext;
                   msgtext = tr("Switch timer deleted");
                   messageBox->UpdateMsg(msgtext);
                   msgBoxStart = cTimeMs::Now();
                   needsRedraw = true;
                   delete serviceData;
                }
            }
            else {
                delete serviceData;
                YAEPG_ERROR("Epgsearch-switchtimer-v1.0: EPGSearch does not support this service!");
                return;
            }
        }
    }
    else {
       YAEPG_ERROR("EPGSearch does not exist!");
    }
}

void
cOsdObjYaepg::AddDelRemoteTimer()
{
   const cEvent *event=gridEvents->Event();
   if (pRemoteTimers) {
      RemoteTimers_GetMatch_v1_0 rtMatch;
      rtMatch.event = event;
      pRemoteTimers->Service("RemoteTimers::GetMatch-v1.0", &rtMatch);
      if (rtMatch.timerMatch == tmFull) {
         if (rtMatch.timer) {
            rtMatch.timer->OnOff();
            RemoteTimers_Timer_v1_0 rt;
            rt.timer = rtMatch.timer;
            if (!pRemoteTimers->Service("RemoteTimers::ModTimer-v1.0", &rt)) {
               messageBox = new cYaepgMsg();
               messageBox->UpdateMsg((char*)*rt.errorMsg);
               msgBoxStart = cTimeMs::Now();
               needsRedraw = true;
            }
         }
      }
      else {
         cTimer *timer = new cTimer(event);
         RemoteTimers_Timer_v1_0 rt;
         rt.timer = timer;
         pRemoteTimers->Service("RemoteTimers::GetTimer-v1.0", &rt.timer);
         if (rt.timer) {
            rt.timer->OnOff();
            if (!pRemoteTimers->Service("RemoteTimers::ModTimer-v1.0", &rt)) {
               messageBox = new cYaepgMsg();
               messageBox->UpdateMsg((char*)*rt.errorMsg);
               msgBoxStart = cTimeMs::Now();
               needsRedraw = true;
            }
            delete timer;
         }
         else {
            rt.timer = timer;
            if (!pRemoteTimers->Service("RemoteTimers::NewTimer-v1.0", &rt)) {
               messageBox = new cYaepgMsg();
               messageBox->UpdateMsg((char*)*rt.errorMsg);
               msgBoxStart = cTimeMs::Now();
               needsRedraw = true;
            }
         }
      }
      eventInfo->UpdateEvent(event);
   }
}

eOSState
cOsdObjYaepg::ProcessKey(eKeys key)
{
    eOSState state = cOsdObject::ProcessKey(key);

    needsRedraw = false;

    if (recordDlg != NULL && state == osUnknown) {
    state=recordDlg->ProcessKey(key);
        if (state == osContinue) {
            needsRedraw = true;
        }
        switch (key & ~k_Repeat) {
            case kOk:
            {
                const char *msgtext;
                msgtext = recordDlg->AddTimer() ? tr("Timer added") : tr("Failed to add timer");
                delete recordDlg;
                recordDlg = NULL;
                messageBox = new cYaepgMsg();
                messageBox->UpdateMsg(msgtext);
                msgBoxStart = cTimeMs::Now();
                eventInfo->UpdateEvent(event);
                needsRedraw = true;
                state = osContinue;
                break;
            }
            case kBack:
            {
                delete recordDlg;
                recordDlg = NULL;
                needsRedraw = true;
                state = osContinue;
                break;
            }
            default:
                state = osContinue;
                break;
        }
    }

    if (state == osUnknown) {
        switch (key & ~k_Repeat) {
        case kBack:
            if (iMenuBACK) {
                cRemote::Put(kMenu);
                if(Setup.MenuKeyCloses) {
                    cRemote::Put(kMenu);
                }
                int i;
                for (i = 1; i < iMenuBackMenuItems; i++) {
                    cRemote::Put(kDown);
                }
                if (iMenuBackSubMenuItems) {
                    cRemote::Put(kOk);
                    for (i = 1; i < iMenuBackSubMenuItems; i++) {
                        cRemote::Put(kDown);
                    }
                }
             } else {
                state = osEnd;
             }
             break;
        case kLeft:
             MoveCursor(DIR_LEFT);
             needsRedraw = true;
             state = osContinue;
             break;
        case kRight:
             MoveCursor(DIR_RIGHT);
             needsRedraw = true;
             state = osContinue;
             break;
        case kUp:
             MoveCursor(DIR_UP);
             needsRedraw = true;
             if (iChannelChange == CHANNEL_CHANGE_AUTOMATIC) {
                SwitchToCurrentChannel();
             }
             state = osContinue;
             break;
        case kDown:
             MoveCursor(DIR_DOWN);
             needsRedraw = true;
             if (iChannelChange == CHANNEL_CHANGE_AUTOMATIC) {
                SwitchToCurrentChannel();
             }
             state = osContinue;
             break;
        case kOk:
             if (iSwitchOK){
                SwitchToCurrentChannel(true);
             if (iChannelChange == CHANNEL_CHANGE_OPEN)
                state = osContinue;
             else
                state = osEnd;
             }
             else {
                 eTimerMatch timerMatch = tmNone;
                 Timers.GetMatch(event, &timerMatch);
                 if (!(timerMatch==tmFull)){
                    if (iRemoteTimer && pRemoteTimers) {
                        RemoteTimers_Event_v1_0 rtEvent;
                        rtEvent.event = event;
                        pRemoteTimers->Service("RemoteTimers::GetTimerByEvent-v1.0", &rtEvent);
                        if (!(rtEvent.timer)){
                          recordDlg = new cYaepgRecDlg();
                          recordDlg->UpdateEvent(gridEvents->Event());
                        }
                        else {
                            AddDelRemoteTimer();  // delete remote timer
                            pRemoteTimers->Service("RemoteTimers::GetTimerByEvent-v1.0", &rtEvent);
                            if (!(rtEvent.timer)) {
                                messageBox = new cYaepgMsg();
                                messageBox->UpdateMsg(tr("Remote timer deactivated"));
                                msgBoxStart = cTimeMs::Now();
                                needsRedraw = true;
                            }
                        }
                    }
                    else {
                        recordDlg = new cYaepgRecDlg();
                        recordDlg->UpdateEvent(gridEvents->Event());
                    }
                }
                else {
                    AddDelTimer();  // delete timer
                    eTimerMatch timerMatch = tmNone;
                    Timers.GetMatch(event, &timerMatch);
                    if (timerMatch==tmNone){
                        messageBox = new cYaepgMsg();
                        messageBox->UpdateMsg(tr("Timer deactivated"));
                        msgBoxStart = cTimeMs::Now();
                        needsRedraw = true;
                    }
                }
                needsRedraw = true;
                state = osContinue;
            }
            break;
        case kRed:
            if (event && event->EventID()!=0){
                if (iRecDlgRed) {
                    eTimerMatch timerMatch = tmNone;
                    Timers.GetMatch(event, &timerMatch);
                    if (!(timerMatch==tmFull)){
                        if (iRemoteTimer && pRemoteTimers) {
                            RemoteTimers_Event_v1_0 rtEvent;
                            rtEvent.event = event;
                            pRemoteTimers->Service("RemoteTimers::GetTimerByEvent-v1.0", &rtEvent);
                            if (!(rtEvent.timer)){
                                recordDlg = new cYaepgRecDlg();
                                recordDlg->UpdateEvent(gridEvents->Event());
                            }
                            else{
                                AddDelRemoteTimer();  // delete remote timer
                                pRemoteTimers->Service("RemoteTimers::GetTimerByEvent-v1.0", &rtEvent);
                                if (!(rtEvent.timer)) {
                                    messageBox = new cYaepgMsg();
                                    messageBox->UpdateMsg(tr("Remote timer deactivated"));
                                    msgBoxStart = cTimeMs::Now();
                                    needsRedraw = true;
                                }
                            }
                        }
                        else {
                            recordDlg = new cYaepgRecDlg();
                            recordDlg->UpdateEvent(gridEvents->Event());
                        }
                    }
                    else {
                        AddDelTimer();  // delete timer
                        eTimerMatch timerMatch = tmNone;
                        Timers.GetMatch(event, &timerMatch);
                        if (timerMatch==tmNone){
                            messageBox = new cYaepgMsg();
                            messageBox->UpdateMsg(tr("Timer deactivated"));
                            msgBoxStart = cTimeMs::Now();
                        }
                    }
                }
                else {
                    if (iRemoteTimer && pRemoteTimers)
                        AddDelRemoteTimer();
                    else
                        AddDelTimer();
                }
                needsRedraw = true;
            }
            state = osContinue;
            break;
        case kGreen:
            UpdateChans((iChannelOrder == CHANNEL_ORDER_UP ? 1 : -1) * GRID_NUM_CHANS);
            needsRedraw = true;
            state = osContinue;
            break;
        case kYellow:
            UpdateChans((iChannelOrder == CHANNEL_ORDER_UP ? -1 : 1) * GRID_NUM_CHANS);
            needsRedraw = true;
            state = osContinue;
            break;
        case kBlue:
            if (iSwitchTimer && pEPGSearch){
                if (event && (event->EventID() != 0)){
                    if (event->IsRunning(true)){
                        SwitchToCurrentChannel(true);
                        if (iChannelChange == CHANNEL_CHANGE_OPEN)
                            state = osContinue;
                        else
                            state = osEnd;
                    }
                    else if (iSwitchTimer){
                        AddDelSwitchTimer();
                        needsRedraw = true;
                        state = osContinue;
                    }
                }
            }
            else {
                SwitchToCurrentChannel(true);
                if (iChannelChange == CHANNEL_CHANGE_OPEN )
                    state = osContinue;
                else
                    state = osEnd;
            }
            break;
        case kFastFwd:
            // +24 hours
            UpdateTime(+86400);
            needsRedraw = true;
            state = osContinue;
            break;
        case kFastRew:
            // -24 hours
            UpdateTime(-86400);
            needsRedraw = true;
            state = osContinue;
            break;
        case k0 ... k9:
            if (directChan || (key != k0)) {
                directChan = ((directChan * 10) + ((key & ~k_Repeat) - k0)) % 100000;
                gridDate->UpdateChan(directChan);
                lastInput.Set(1000);
                needsRedraw = true;
            }
        default:
        break;
        }
    }

   /* Channel input timeout */
   if (directChan && lastInput.TimedOut()) {
      YAEPG_INFO("Direct input timed out, channel %d", directChan);

      /* Look for a channel close to what the user entered */
      cChannel *chan = NULL;
      for (int i = 0; i < 500; i++) {
         if ((chan = Channels.GetByNumber(directChan + i)) != NULL) {
            break;
         }
         if ((chan = Channels.GetByNumber(directChan - i)) != NULL) {
            break;
         }
      }
      if (chan != NULL) {
         UpdateChans(chan);
      }

      /* Reset the direct input value */
      directChan = 0;
      gridDate->UpdateChan(0);

      /* Position the cursor on the channel entered */
      gridEvents->Row(0);

      needsRedraw = true;
   }

   /* Uptate the grid time */
   time_t now = time(NULL);
   struct tm locNow, locStart;
   localtime_r(&now, &locNow);
   localtime_r(&startTime, &locStart);
   if (locNow.tm_min != locStart.tm_min) {
      needsRedraw = true;
      if (now > startTime) {
         SetTime(now);
      }
   }

   if (messageBox != NULL && (cTimeMs::Now() - msgBoxStart) > 1000) {
       delete messageBox;
       messageBox = NULL;
       needsRedraw = true;
   }

   /* Redraw the screen if needed */
   if (needsRedraw) {
      Draw();
   }

   return state;
}

void
cOsdObjYaepg::UpdateChans(cChannel *c)
{
   chanVec.resize(GRID_NUM_CHANS);
   chanVec[0] = c;
   for (int i = 1; i < GRID_NUM_CHANS; i++) {
      if (iChannelOrder == CHANNEL_ORDER_UP) {
         while ((c = (cChannel *)c->Prev()) && (c->GroupSep()));
         if (c == NULL) {
            c = Channels.Last();
            while (c && c->GroupSep()) {
               c = (cChannel *)c->Prev();
            }
         }
      } else {
         while ((c = (cChannel *)c->Next()) && (c->GroupSep()));
         if (c == NULL) {
            c = Channels.First();
            while (c && (c->GroupSep())) {
               c = (cChannel *)c->Next();
            }
         }
      }
      chanVec[i] = c;
   }

   /* On first update, widgets haven't been created yet */
   if (gridEvents == NULL) {
      return;
   }

   gridEvents->UpdateChans(chanVec);
   gridChans->UpdateChans(chanVec);
   UpdateEvent(gridEvents->Event());
}

void
cOsdObjYaepg::UpdateChans(int change)
{
   cChannel *c = chanVec[0];

   YAEPG_INFO("Scrolling %d, current channel %d", change, c->Number());

   if (change > 0) {
      for (int i = 0; i < change; i++) {
         while ((c = (cChannel *)c->Next()) && (c->GroupSep()));
         if (c == NULL) {
            c = Channels.First();
            while (c && c->GroupSep()) {
               c = (cChannel *)c->Next();
            }
         }
      }
   } else if (change < 0) {
      for (int i = 0; i > change; i--) {
         while ((c = (cChannel *)c->Prev()) && (c->GroupSep()));
         if (c == NULL) {
            c = Channels.Last();
            while (c && c->GroupSep()) {
               c = (cChannel *)c->Prev();
            }
         }
      }
   }

   YAEPG_INFO("New channel %d", c->Number());

   UpdateChans(c);
}

void
cOsdObjYaepg::SetTime(time_t newTime)
{
   startTime = newTime;

   gridEvents->UpdateTime(startTime);
   gridTime->UpdateTime(startTime);
   gridDate->UpdateTime(startTime);
   timeLine->UpdateTime(startTime);
   eventDate->Update();
   UpdateEvent(gridEvents->Event());
}

void
cOsdObjYaepg::UpdateTime(int change)
{
   startTime += change;
   if (startTime < time(NULL)) {
      startTime = time(NULL);
   }

   gridEvents->UpdateTime(startTime);
   gridTime->UpdateTime(startTime);
   gridDate->UpdateTime(startTime);
   timeLine->UpdateTime(startTime);
   eventDate->Update();
   UpdateEvent(gridEvents->Event());
}

void
cOsdObjYaepg::UpdateEvent(const cEvent *newEvent)
{
   YAEPG_INFO("Updating event widgets");

   if (event == newEvent) {
      return;
   }
   event = newEvent;
   eventTitle->UpdateEvent(event);
   eventInfo->UpdateEvent(event);
   eventTime->UpdateEvent(event);
   eventDesc->UpdateEvent(event);
   if (iEpgImages)
      eventEpgImage->UpdateEvent(event);
}

void
cOsdObjYaepg::SwitchToCurrentChannel(bool closeVidWin)
{
   const cChannel *gridChan = chanVec[gridEvents->Row()];

   if (gridChan && (gridChan->Number() != cDevice::CurrentChannel())) {
      /*
       * The eHD card doesn't seem to like changing channels while the video
       * plane is scaled down.  To get around this problem we close/reopen the
       * video window across channel changes.
       */
#ifdef YAEPGHD_REEL_EHD
      reelVidWin->Close();
      cCondWait::SleepMs(100);
#endif

      Channels.SwitchTo(gridChan->Number());

#ifdef YAEPGHD_REEL_EHD
      if (closeVidWin == false) {
         reelVidWin->Open(VID_WIN_GEOM);
      }
#endif
   }
   return;
}

void
cOsdObjYaepg::MoveCursor(eCursorDir dir)
{
   if (gridEvents->MoveCursor(dir)) {
      UpdateEvent(gridEvents->Event());
      return;
   }

   /* Need to scroll */
   switch (dir) {
   case DIR_UP:
      UpdateChans(1 * (iChannelOrder == CHANNEL_ORDER_UP ? 1 : -1));
      break;
   case DIR_DOWN:
      UpdateChans(-1 * (iChannelOrder == CHANNEL_ORDER_UP ? 1 : -1));
      break;
   case DIR_LEFT:
      UpdateTime(-3600);
      break;
   case DIR_RIGHT:
      UpdateTime(3600);
      break;
   default:
      ASSERT(0);
      break;
   }
}

void
cOsdObjYaepg::Draw(void)
{
   mainBmp->DrawBitmap(0, 0, *BG_IMAGE);

   gridEvents->Draw(mainBmp);
   gridChans->Draw(mainBmp);
   gridTime->Draw(mainBmp);
   gridDate->Draw(mainBmp);
   timeLine->Draw(mainBmp);
   eventTitle->Draw(mainBmp);
   eventInfo->Draw(mainBmp);
   eventTime->Draw(mainBmp);
   eventDesc->Draw(mainBmp);
   eventDate->Draw(mainBmp);
   if (iEpgImages)
      eventEpgImage->Draw(mainBmp);
   helpBar->Draw(mainBmp);
   if (recordDlg != NULL) {
       recordDlg->Draw(mainBmp);
   }
   if (messageBox != NULL) {
       messageBox->Draw(mainBmp);
   }

   osd->DrawBitmap(0, 0, *mainBmp);
   cDevice::PrimaryDevice()->ScaleVideo(videoWindowRect); // scale to our desired video window size if supported
   osd->Flush();
}
