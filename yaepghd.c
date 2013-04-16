/*
 * yaepghd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Community Edition
 */

/**
 * Includes
 */
#include <locale.h>
#include <langinfo.h>
#include <string>
#include <map>
#include <assert.h>
#include <getopt.h>
#ifdef YAEPGHD_REEL_EHD
#include <curl/curl.h>
#endif

#include <vdr/config.h>
#include <vdr/plugin.h>
#include <vdr/interface.h>


#include "MenuSetupYaepg.h"
#include "OsdObjYaepg.h"
#include "Utils.h"


#if defined(APIVERSNUM) && APIVERSNUM < 10733
#error "VDR-1.7.33 API version or greater is required!"
#endif


/*
 *****************************************************************************
 * cPluginYaepghd
 *****************************************************************************
 */
static const char *VERSION        = "0.0.5_pre1";
static const char *DESCRIPTION    = trNOOP("Yet another EPG in HD");

class cPluginYaepghd : public cPlugin {
public:
   cPluginYaepghd(void);
   virtual ~cPluginYaepghd();
   virtual const char *Version(void) { return VERSION; }
   virtual const char *Description(void) { return tr(DESCRIPTION); }
   virtual const char *CommandLineHelp(void);
   virtual bool ProcessArgs(int argc, char *argv[]);
   virtual bool Initialize(void);
   virtual bool Start(void);
   virtual void Stop(void);
   virtual void Housekeeping(void);
   virtual void MainThreadHook(void);
   virtual cString Active(void);
   virtual time_t WakeupTime(void);
   virtual const char *MainMenuEntry(void);
   virtual cOsdObject *MainMenuAction(void);
   virtual cMenuSetupPage *SetupMenu(void);
   virtual bool SetupParse(const char *Name, const char *Value);
   virtual bool Service(const char *Id, void *Data = NULL);
   virtual const char **SVDRPHelpPages(void);
   virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
};

cPluginYaepghd::cPluginYaepghd(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
}

cPluginYaepghd::~cPluginYaepghd()
{
  // Clean up after yourself!
}

const char *
cPluginYaepghd::CommandLineHelp(void)
{
   // Return a string that describes all known command line options.
     return
         "  -i <IMAGESDIR>, --epgimages=<IMAGESDIR> Set directory where epgimages are stored\n";
}

const char *cPluginYaepghd::MainMenuEntry(void)
{
   if (iHideMenuEntry)
      return NULL;
   if (isempty(sMainMenuEntry))
      return tr("YaepgHD");
   else
      return sMainMenuEntry;
}

bool
cPluginYaepghd::ProcessArgs(int argc, char *argv[])
{
   // Implement command line argument processing here if applicable.
  static const struct option long_options[] = {
    { "epgimages", required_argument, NULL, 'i' },
    { 0, 0, 0, 0 }
  };
  int c;
  while ((c = getopt_long(argc, argv, "i:", long_options, NULL)) != -1) {
    switch (c) {
      case 'i':
        sEpgImagesDir=optarg;
        break;
      default:
        return false;
    }
  }
  return true;
}

bool
cPluginYaepghd::Initialize(void)
{
   // Initialize any background activities the plugin shall perform.
   sThemeDir = cPlugin::ConfigDirectory(PLUGIN_NAME_I18N);
   return true;
}

bool
cPluginYaepghd::Start(void)
{
   // Start any background activities the plugin shall perform.
#ifdef YAEPGHD_REEL_EHD
   reelVidWin = new cReelVidWin;
   reelVidWin->Init();
#endif
   cFontSymbols::InitCharSet();
   pEPGSearch = cPluginManager::GetPlugin("epgsearch");
   if (!pEPGSearch) {
      YAEPG_ERROR("EPGSearch does not exist (switch timer disabled)!");
   }

   pRemoteTimers = cPluginManager::CallFirstService("RemoteTimers::RefreshTimers-v1.0", NULL);
   if (!pRemoteTimers) {
      YAEPG_ERROR("RemoteTimers does not exist!");
   }
   return true;
}

void
cPluginYaepghd::Stop(void)
{
   // Stop any background activities the plugin is performing.
#ifdef YAEPGHD_REEL_EHD
   delete reelVidWin;
#endif
}

void
cPluginYaepghd::Housekeeping(void)
{
   // Perform any cleanup or other regular tasks.
}

void
cPluginYaepghd::MainThreadHook(void)
{
   // Perform actions in the context of the main program thread.
   // WARNING: Use with great care - see PLUGINS.html!
}

cString
cPluginYaepghd::Active(void)
{
   // Return a message string if shutdown should be postponed
   return NULL;
}

time_t
cPluginYaepghd::WakeupTime(void)
{
   // Return custom wakeup time for shutdown script
   return 0;
}

cOsdObject *
cPluginYaepghd::MainMenuAction(void)
{
   return new cOsdObjYaepg;
}

cMenuSetupPage *
cPluginYaepghd::SetupMenu(void)
{
   // Return a setup menu in case the plugin supports one.
   return new cMenuSetupYaepg;
}

bool
cPluginYaepghd::SetupParse(const char *Name, const char *Value)
{
   char themeName[4 * MaxThemeName];

   // Parse your own setup parameters and store their values.
   if      (!strcasecmp(Name, "HideMenuEntry")) { iHideMenuEntry = atoi(Value); }
   if      (!strcasecmp(Name, "MainMenuEntry")) { strcpy(sMainMenuEntry, Value); }
   #if defined(MAINMENUHOOKSVERSION)
   #if MAINMENUHOOKSVERSNUM >= 10001
   else if (!strcasecmp(Name, "ReplaceOrgSchedule")) { iReplaceOrgSchedule = atoi(Value); }
   #endif
   #endif
   else if (!strcasecmp(Name, "ChannelChange")) { iChannelChange = atoi(Value); }
   else if (!strcasecmp(Name, "SwitchOk")) { iSwitchOK = atoi(Value); }
   else if (!strcasecmp(Name, "MenuBACK")) { iMenuBACK = atoi(Value); }
   else if (!strcasecmp(Name, "MenuBackMenuItems")) { iMenuBackMenuItems = atoi(Value); }
   else if (!strcasecmp(Name, "MenuBackSubMenuItems")) { iMenuBackSubMenuItems = atoi(Value); }
   else if (!strcasecmp(Name, "RecDlgRed")) { iRecDlgRed = atoi(Value); }
   else if (!strcasecmp(Name, "TimeFormat"))    { iTimeFormat = atoi(Value); }
   else if (!strcasecmp(Name, "ChannelOrder"))  { iChannelOrder = atoi(Value); }
   else if (!strcasecmp(Name, "ChannelNumber")) { iChannelNumber = atoi(Value); }
   else if (!strcasecmp(Name, "InfoSymbols"))   { iInfoSymbols = atoi(Value); }
   else if (!strcasecmp(Name, "SwitchTimer"))   { iSwitchTimer = atoi(Value); }
   else if (!strcasecmp(Name, "SwitchMinsBefore")){ iSwitchMinsBefore = atoi(Value); }
   else if (!strcasecmp(Name, "RemoteTimer"))   { iRemoteTimer = atoi(Value); }
   else if (!strcasecmp(Name, "EpgImages"))     { iEpgImages = atoi(Value); }
   else if (!strcasecmp(Name, "ResizeImages"))    { iResizeImages = atoi(Value); }
   else if (!strcasecmp(Name, "ImageExtension"))  { iImageExtension = atoi(Value); }
   else if (!strcasecmp(Name, "Theme"))         { Utf8Strn0Cpy(themeName, Value, sizeof(themeName)); sThemeName = themeName; }
   else                                         { return false; }

   return true;
}

bool
cPluginYaepghd::Service(const char *Id, void *Data)
{
#if defined(MAINMENUHOOKSVERSION)
#if MAINMENUHOOKSVERSNUM >= 10001
   if (strcmp(Id, "MainMenuHooksPatch-v1.0::osSchedule") == 0  && iReplaceOrgSchedule)
   {
      if (!Data) {
         return true;
      }
      cOsdObject **osd = (cOsdObject**)Data;
      if (osd) {
         *osd = (cOsdObject*)MainMenuAction();
      }
      return true;
   }
#endif
#endif
   return false;
}

const char **
cPluginYaepghd::SVDRPHelpPages(void)
{
   // Return help text for SVDRP commands this plugin implements
   return NULL;
}

cString
cPluginYaepghd::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
   // Process SVDRP commands this plugin implements
   return NULL;
}

VDRPLUGINCREATOR(cPluginYaepghd); // Don't touch this!
