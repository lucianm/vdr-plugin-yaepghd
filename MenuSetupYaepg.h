/*
 * yaepghd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Community Edition
 */

#pragma once

#include <string>

#include <vdr/menuitems.h>

enum eTimeFormatType {
   TIME_FORMAT_24H,
   TIME_FORMAT_12H,
   TIME_FORMAT_COUNT
};

/* Order of channels (UP or DOWN) */
enum eChannelOrderType {
   CHANNEL_ORDER_UP,
   CHANNEL_ORDER_DOWN,
   CHANNEL_ORDER_COUNT
};

/* Manner in which channel is changed while in YAEPGHD */
enum eChanneChangeType {
   CHANNEL_CHANGE_CLOSE,
   CHANNEL_CHANGE_OPEN,
   CHANNEL_CHANGE_AUTOMATIC,
   CHANNEL_CHANGE_COUNT
};

extern int iChannelChange;
extern int iSwitchOK;
extern int iMenuBACK;
extern int iMenuBackMenuItems;
extern int iMenuBackSubMenuItems;
extern int iTimeFormat;
extern int iChannelOrder;
extern int iChannelNumber;
extern int iRecDlgRed;
extern int iInfoSymbols;
extern int iSwitchTimer;
extern int iSwitchMinsBefore;
extern int iRemoteTimer;
extern int iEpgImages;
extern int iResizeImages;
extern int iImageExtension;
extern int iHideMenuEntry;
extern char sMainMenuEntry[NAME_MAX];
#if defined(MAINMENUHOOKSVERSION)
#if MAINMENUHOOKSVERSNUM >= 10001
extern int iReplaceOrgSchedule;
#endif
#endif

extern std::string sThemeName;
extern std::string sThemeDir;
extern std::string sEpgImagesDir;
extern int iVDRSymbols;
extern cPlugin* pEPGSearch;
extern cPlugin* pRemoteTimers;

const extern char *imageExtensionTexts[3];

/*
 *****************************************************************************
 * cMenuSetupYaepg
 *****************************************************************************
 */
class cMenuSetupYaepg : public cMenuSetupPage {
private:
   int iNewHideMenuEntry;
   char sNewMainMenuEntry[NAME_MAX];
#if defined(MAINMENUHOOKSVERSION)
#if MAINMENUHOOKSVERSNUM >= 10001
   int iNewReplaceOrgSchedule;
#endif
#endif
   int iNewChannelChange;
   int iNewSwitchOK;
   int iNewMenuBACK;
   int iNewMenuBackMenuItems;
   int iNewMenuBackSubMenuItems;
   int iNewRecDlgRed;
   int iNewTimeFormat;
   int iNewChannelOrder;
   int iNewChannelNumber;
   int iNewInfoSymbols;
   int iNewSwitchTimer;
   int iNewSwitchMinsBefore;
   int iNewRemoteTimer;
   int iNewEpgImages;
   int iNewResizeImages;
   int iNewImageExtension;
   int iNewThemeIndex;
   char **themes;
   int numThemes;
   const char *TIME_FORMATS[TIME_FORMAT_COUNT];
   const char *CH_ORDER_FORMATS[CHANNEL_ORDER_COUNT];
   const char *CH_CHANGE_MODES[CHANNEL_CHANGE_COUNT];
   const char *resizeImagesTexts[3];

private:
    void Create(void);
protected:
   virtual void Store(void);

public:
   cMenuSetupYaepg(void);
   ~cMenuSetupYaepg();
   virtual eOSState ProcessKey(eKeys);
};
