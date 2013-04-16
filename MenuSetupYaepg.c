/*
 * yaepghd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Community Edition
 */

#include "MenuSetupYaepg.h"
#include "GuiElements.h"
#include "Utils.h"

int iChannelChange           = CHANNEL_CHANGE_CLOSE;
int iSwitchOK                = false;
int iMenuBACK                = false;
int iMenuBackMenuItems       = 0;
int iMenuBackSubMenuItems    = 0;
int iTimeFormat              = TIME_FORMAT_12H;
int iChannelOrder            = CHANNEL_ORDER_DOWN;
int iChannelNumber           = false;
int iRecDlgRed               = false;
int iInfoSymbols             = false;
int iSwitchTimer             = false;
int iSwitchMinsBefore        = 1;
int iRemoteTimer             = false;
int iEpgImages               = false;
int iResizeImages            = 0;
int iImageExtension          = 0;

int iHideMenuEntry           = false;
char sMainMenuEntry[NAME_MAX] = "";
#if defined(MAINMENUHOOKSVERSION)
#if MAINMENUHOOKSVERSNUM >= 10001
int iReplaceOrgSchedule      = false;
#endif
#endif

std::string sThemeName    = "default";
std::string sThemeDir     = "";
std::string sEpgImagesDir = "/video/epgimages";
int iVDRSymbols              = false;
cPlugin*           pEPGSearch    = NULL;
cPlugin*           pRemoteTimers = NULL;

const char *imageExtensionTexts[3] = { "png", "jpg", "xpm" };

void cMenuSetupYaepg::Store(void)
{
   iHideMenuEntry      = iNewHideMenuEntry;
 #if defined(MAINMENUHOOKSVERSION)
 #if MAINMENUHOOKSVERSNUM >= 10001
   iReplaceOrgSchedule = iNewReplaceOrgSchedule;
 #endif
 #endif
   iChannelChange      = iNewChannelChange;
   iSwitchOK           = iNewSwitchOK;
   iMenuBACK           = iNewMenuBACK;
   iMenuBackMenuItems  = iNewMenuBackMenuItems;
   iMenuBackSubMenuItems  = iNewMenuBackSubMenuItems;
   iRecDlgRed          = iNewRecDlgRed;
   iTimeFormat         = iNewTimeFormat;
   iChannelOrder       = iNewChannelOrder;
   iChannelNumber      = iNewChannelNumber;
   iInfoSymbols        = iNewInfoSymbols;
   iSwitchTimer        = iNewSwitchTimer;
   iRemoteTimer        = iNewRemoteTimer;
   iEpgImages          = iNewEpgImages;
   iResizeImages       = iNewResizeImages;
   iImageExtension  = iNewImageExtension;
   iSwitchMinsBefore = iNewSwitchMinsBefore;
   sThemeName          = themes[iNewThemeIndex];

   if (strcmp(sMainMenuEntry, tr("YaepgHD")) == 0) {
      strcpy(sMainMenuEntry,"");
   }
   else{
      strcpy(sMainMenuEntry,sNewMainMenuEntry);
   }

   SetupStore("HideMenuEntry",      iHideMenuEntry);
   SetupStore("MainMenuEntry",      sMainMenuEntry);
 #if defined(MAINMENUHOOKSVERSION)
 #if MAINMENUHOOKSVERSNUM >= 10001
   SetupStore("ReplaceOrgSchedule", iReplaceOrgSchedule);
 #endif
 #endif
   SetupStore("ChannelChange",      iChannelChange);
   SetupStore("SwitchOk",           iSwitchOK);
   SetupStore("MenuBACK",           iMenuBACK);
   SetupStore("MenuBackMenuItems",     iMenuBackMenuItems);
   SetupStore("MenuBackSubMenuItems",  iMenuBackSubMenuItems);
   SetupStore("RecDlgRed",          iRecDlgRed);
   SetupStore("TimeFormat",         iTimeFormat);
   SetupStore("ChannelOrder",       iChannelOrder);
   SetupStore("ChannelNumber",      iChannelNumber);
   SetupStore("InfoSymbols",        iInfoSymbols);
   SetupStore("SwitchTimer",        iSwitchTimer);
   SetupStore("SwitchMinsBefore",   iSwitchMinsBefore);
   SetupStore("RemoteTimer",        iRemoteTimer);
   SetupStore("EpgImages",          iEpgImages);
   SetupStore("ResizeImages",       iResizeImages);
   SetupStore("ImageExtension",     iImageExtension);
   SetupStore("Theme",              sThemeName.c_str());
}

cMenuSetupYaepg::cMenuSetupYaepg(void)
{
   TIME_FORMATS[TIME_FORMAT_24H] = tr("24h");
   TIME_FORMATS[TIME_FORMAT_12H] = tr("12h");

   CH_ORDER_FORMATS[CHANNEL_ORDER_UP]   = tr("Up");
   CH_ORDER_FORMATS[CHANNEL_ORDER_DOWN] = tr("Down");

   CH_CHANGE_MODES[CHANNEL_CHANGE_CLOSE]        = tr("Close YaepgHD");
   CH_CHANGE_MODES[CHANNEL_CHANGE_OPEN] = tr("Leave YaepgHD open");
   CH_CHANGE_MODES[CHANNEL_CHANGE_AUTOMATIC]     = tr("Automatic");

   resizeImagesTexts[0] = tr("pixel algo");
   resizeImagesTexts[1] = tr("ratio algo");
   resizeImagesTexts[2] = tr("zoom image");

   cYaepgTheme::Themes(&themes, &numThemes);
   iNewThemeIndex = 0;
   if (!sThemeName.empty()) {
      for (int i = 0; i < numThemes; i++) {
         if (strcmp(sThemeName.c_str(), themes[i]) == 0) {
            iNewThemeIndex = i;
         }
      }
   } else {
      iNewThemeIndex = 0;
   }

   strcpy(sNewMainMenuEntry,sMainMenuEntry);
  if (isempty(sNewMainMenuEntry)){
     strcpy(sNewMainMenuEntry,tr("YaepgHD"));
  }

   iNewHideMenuEntry = iHideMenuEntry;
   #if defined(MAINMENUHOOKSVERSION)
   #if MAINMENUHOOKSVERSNUM >= 10001
   iNewReplaceOrgSchedule = iReplaceOrgSchedule;
   #endif
   #endif
   iNewChannelChange   = iChannelChange;
   iNewSwitchOK        = iSwitchOK;
   iNewMenuBACK        = iMenuBACK;
   iNewMenuBackMenuItems  = iMenuBackMenuItems;
   iNewMenuBackSubMenuItems  = iMenuBackSubMenuItems;
   iNewRecDlgRed       = iRecDlgRed;
   iNewTimeFormat      = iTimeFormat;
   iNewChannelOrder    = iChannelOrder;
   iNewChannelNumber   = iChannelNumber;
   iNewInfoSymbols     = iInfoSymbols;
   iNewSwitchTimer     = iSwitchTimer;
   iNewRemoteTimer     = iRemoteTimer;
   iNewEpgImages       = iEpgImages;
   iNewResizeImages    = iResizeImages;
   iNewImageExtension  = iImageExtension;
   iNewSwitchMinsBefore= iSwitchMinsBefore;

    Create();
}

void cMenuSetupYaepg::Create() {
   int current = Current(); // remember current menu item index
   Clear(); // clear old menu

   Add(new cMenuEditBoolItem (tr("Hide mainmenu entry"), &iNewHideMenuEntry));
   Add(new cMenuEditStrItem(tr("Main menu entry"), sNewMainMenuEntry, sizeof(sNewMainMenuEntry),trVDR(FileNameChars)));
   #if defined(MAINMENUHOOKSVERSION)
   #if MAINMENUHOOKSVERSNUM >= 10001
   Add(new cMenuEditBoolItem (tr("Replace original schedule"), &iNewReplaceOrgSchedule));
   #endif
   #endif
   Add(new cMenuEditStraItem (tr("Channel change"), &iNewChannelChange, CHANNEL_CHANGE_COUNT, CH_CHANGE_MODES));
   Add(new cMenuEditBoolItem (tr("Switch channel with OK"), &iNewSwitchOK));
   Add(new cMenuEditBoolItem (tr("Mainmenu with BACK"), &iNewMenuBACK));
   if (iNewMenuBACK) {
        Add(new cMenuEditIntItem( tr("   Select menu at position..."), &iNewMenuBackMenuItems, 0, 100));
        if (iNewMenuBackMenuItems) {
            Add(new cMenuEditIntItem( tr("      Select submenu at position..."), &iNewMenuBackSubMenuItems, 0, 50));
        }
   }
   Add(new cMenuEditBoolItem (tr("Record dialog with red button"), &iNewRecDlgRed));
   Add(new cMenuEditStraItem (tr("Time format"), &iNewTimeFormat, TIME_FORMAT_COUNT, TIME_FORMATS));
   Add(new cMenuEditStraItem (tr("Channel order"), &iNewChannelOrder, CHANNEL_ORDER_COUNT, CH_ORDER_FORMATS));
   Add(new cMenuEditBoolItem (tr("Channel number"), &iNewChannelNumber));

   if (iVDRSymbols){
      Add(new cMenuEditBoolItem (tr("Info symbols"), &iNewInfoSymbols));
   }
   Add(new cMenuEditBoolItem (tr("EPG images"), &iNewEpgImages));
   Add(new cMenuEditStraItem(tr("  Resize images"), &iNewResizeImages, 3, resizeImagesTexts));
   Add(new cMenuEditStraItem(tr("  Image format"), &iNewImageExtension, 3, imageExtensionTexts));

   if (pEPGSearch) {
      Add(new cMenuEditBoolItem (tr("Switch timer"), &iNewSwitchTimer));
      Add(new cMenuEditIntItem( tr("  Switch ... minutes before start"), &iNewSwitchMinsBefore));
   }
   else {
      YAEPG_INFO("EPGSearch not found (Switch Timer disabled)!");
   }
   if (pRemoteTimers) {
      Add(new cMenuEditBoolItem (tr("Remote timer"), &iNewRemoteTimer));
   }
   else {
      YAEPG_INFO("RemoteTimers not found (Remote Timer disabled)!");
   }

   if (numThemes > 0) {
      Add(new cMenuEditStraItem (trVDR("Setup.OSD$Theme"), &iNewThemeIndex, numThemes, themes));
   }

   SetCurrent(Get(current)); // restore previously selected menu entry
   Display(); //show newly built menu
}

cMenuSetupYaepg::~cMenuSetupYaepg()
{
   for (int i = 0; i < numThemes; i++) {
      free(themes[i]);
   }
   if (themes) {
      free(themes);
   }
}

eOSState cMenuSetupYaepg::ProcessKey(eKeys key)
{
    int iOldMenuBACK = iNewMenuBACK;
    int iOldNewMenuBackMenuItems = iNewMenuBackMenuItems;
    eOSState state = cMenuSetupPage::ProcessKey(key);
    if ( iOldMenuBACK != iNewMenuBACK ||
         (  iOldNewMenuBackMenuItems * iNewMenuBackMenuItems == 0 &&
            iOldNewMenuBackMenuItems + iNewMenuBackMenuItems != 0 ) ) {
        Create(); // re-create setup menu only if necessary, if the 2 values changed from 0 to non-zero and vice-versa
    }
    return state;
}
