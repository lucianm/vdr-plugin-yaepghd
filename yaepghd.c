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
#include <vector>
#include <map>
#include <assert.h>
#include <Magick++.h>
#include <getopt.h>
#ifdef YAEPGHD_REEL_EHD
#include <curl/curl.h>
#endif

#include <vdr/config.h>
#include <vdr/plugin.h>
#include <vdr/osd.h>
#include <vdr/timers.h>
#include <vdr/device.h>

#if defined(APIVERSNUM) && APIVERSNUM < 10733
#error "VDR-1.7.33 API version or greater is required!"
#endif

// To avoid problems with the old MainMenuHooks-v1.0 patch uncomment next line.
// #undef MAINMENUHOOKSVERSNUM 

/**
 * Macros
 */
//#define DEBUG
#ifdef DEBUG
#define ASSERT                   assert
#define YAEPG_ERROR(...)         yaepg_error(__PRETTY_FUNCTION__, __VA_ARGS__)
#define YAEPG_INFO(...)          yaepg_info(__PRETTY_FUNCTION__, __VA_ARGS__)
#else /* !DEBUG */
#define ASSERT(_a)
#define YAEPG_ERROR(...)
#define YAEPG_INFO(...)
#endif /* DEBUG */

#ifndef MIN
#define MIN(_a, _b)              ((_a) < (_b) ? (_a) : (_b))
#endif
#ifndef MAX
#define MAX(_a, _b)              ((_a) < (_b) ? (_b) : (_a))
#endif
#define ROUND(_f)                (int)((_f) + 0.5f)

/**
 * Macros to retrieve theme values
 */
#define THEME_IMAGE(_name) cYaepgTheme::Instance()->Element(_name).u.bmp
#define THEME_FONT(_name)  cYaepgTheme::Instance()->Element(_name).u.font
#define THEME_COLOR(_name) cYaepgTheme::Instance()->Element(_name).u.color
#define THEME_GEOM(_name)  cYaepgTheme::Instance()->Element(_name).u.geom
#define THEME_IVAL(_name)  cYaepgTheme::Instance()->Element(_name).u.ival

#define BG_IMAGE                 THEME_IMAGE("bgImage")
#define GRID_EVENT_FONT          THEME_FONT("gridEventFont")
#define GRID_CHAN_FONT           THEME_FONT("gridChanFont")
#define GRID_TIME_FONT           THEME_FONT("gridTimeFont")
#define GRID_DATE_FONT           THEME_FONT("gridDateFont")
#define EVENT_TITLE_FONT         THEME_FONT("eventTitleFont")
#define EVENT_INFO_FONT          THEME_FONT("eventInfoFont")
#define EVENT_TIME_FONT          THEME_FONT("eventTimeFont")
#define EVENT_DESC_FONT          THEME_FONT("eventDescFont")
#define EVENT_DATE_FONT          THEME_FONT("eventDateFont")
#define HELP_BAR_FONT            THEME_FONT("helpFont")
#define GRID_EVENT_COLOR         THEME_COLOR("gridEventColor")
#define GRID_SEL_FG              THEME_COLOR("gridSelFg")
#define GRID_SEL_BG              THEME_COLOR("gridSelBg")
#define GRID_CHAN_COLOR          THEME_COLOR("gridChanColor")
#define GRID_TIME_COLOR          THEME_COLOR("gridTimeColor")
#define GRID_DATE_COLOR          THEME_COLOR("gridDateColor")
#define GRID_SEP_COLOR           THEME_COLOR("gridSepColor")
#define EVENT_TITLE_COLOR        THEME_COLOR("eventTitleColor")
#define EVENT_INFO_COLOR         THEME_COLOR("eventInfoColor")
#define EVENT_TIME_COLOR         THEME_COLOR("eventTimeColor")
#define EVENT_DESC_COLOR         THEME_COLOR("eventDescColor")
#define EVENT_DATE_COLOR         THEME_COLOR("eventDateColor")
#define TLINE_BOX_COLOR          THEME_COLOR("tlineBoxColor")
#define HELP_BAR_COLOR           THEME_COLOR("helpColor")
#define GRID_EVENT_GEOM          THEME_GEOM("gridEventGeom")
#define GRID_CHAN_GEOM           THEME_GEOM("gridChanGeom")
#define GRID_TIME_GEOM           THEME_GEOM("gridTimeGeom")
#define GRID_DATE_GEOM           THEME_GEOM("gridDateGeom")
#define EVENT_TITLE_GEOM         THEME_GEOM("eventTitleGeom")
#define EVENT_INFO_GEOM          THEME_GEOM("eventInfoGeom")
#define EVENT_TIME_GEOM          THEME_GEOM("eventTimeGeom")
#define EVENT_DESC_GEOM          THEME_GEOM("eventDescGeom")
#define EVENT_DATE_GEOM          THEME_GEOM("eventDateGeom")
#define EVENT_EPGIMAGE_GEOM      THEME_GEOM("eventEpgImageGeom")
#define TLINE_LOC_GEOM           THEME_GEOM("tlineLocGeom")
#define TLINE_BOX_GEOM           THEME_GEOM("tlineBoxGeom")
#define VID_WIN_GEOM             THEME_GEOM("vidWinGeom")
#define HELP_BAR_GEOM            THEME_GEOM("helpGeom")
#define GRID_NUM_CHANS           THEME_IVAL("gridNumChans")
#define LEFT_ARROW_WIDTH         THEME_IVAL("leftArrowWidth")
#define RIGHT_ARROW_WIDTH        THEME_IVAL("rightArrowWidth")
#define GRID_HORIZ_SPACE         THEME_IVAL("gridHorizSpace")
#define TEXT_BORDER              THEME_IVAL("textBorder")
#define TEXT_SPACE               THEME_IVAL("textSpace")
#define EVENT_INFO_ALIGN         THEME_IVAL("eventInfoAlign")

#define REC_DLG_IMG              THEME_IMAGE("recDlgImage")
#define REC_DLG_FONT             THEME_FONT("recDlgFont")
#define REC_DLG_COLOR            THEME_COLOR("recDlgColor")
#define REC_DLG_GEOM             THEME_GEOM("recDlgGeom")
#define REC_TITLE_GEOM           THEME_GEOM("recTitleGeom")
#define REC_TIME_GEOM            THEME_GEOM("recTimeGeom")
#define REC_START_GEOM           THEME_GEOM("recStartGeom")
#define REC_END_GEOM             THEME_GEOM("recEndGeom")
#define REC_FREQ_GEOM            THEME_GEOM("recFreqGeom")
#define REC_STINP_GEOM           THEME_GEOM("recStInpGeom")
#define REC_ENINP_GEOM           THEME_GEOM("recEnInpGeom")
#define REC_FRINP_GEOM           THEME_GEOM("recFrInpGeom")

#define MSG_BG_IMG               THEME_IMAGE("msgBgImage")
#define MSG_BOX_FONT             THEME_FONT("msgBoxFont")
#define MSG_BOX_COLOR            THEME_COLOR("msgBoxColor")
#define MSG_BOX_GEOM             THEME_GEOM("msgBoxGeom")

/* Manner in which channel is changed while in YAEPGHD */
enum eChanneChangeType {
   CHANNEL_CHANGE_CLOSE,
   CHANNEL_CHANGE_OPEN,
   CHANNEL_CHANGE_AUTOMATIC,
   CHANNEL_CHANGE_COUNT
};

enum eTimeFormatType {
   TIME_FORMAT_24H,
   TIME_FORMAT_12H,
   TIME_FORMAT_COUNT
};

#define FMT_AMPM(_hr)                  ((_hr) >= 12 ? "p" : "a")
#define FMT_12HR(_hr)                  ((_hr) % 12 == 0 ? 12 : (_hr) % 12)

/* Order of channels (UP or DOWN) */
enum eChannelOrderType {
   CHANNEL_ORDER_UP,
   CHANNEL_ORDER_DOWN,
   CHANNEL_ORDER_COUNT
};

using namespace Magick;

/**
 * Private Data
 */
static int         iHideMenuEntry           = false;
static char        sMainMenuEntry[MaxFileName] = "";
#if defined(MAINMENUHOOKSVERSION) 
#if MAINMENUHOOKSVERSNUM >= 10001
static int         iReplaceOrgSchedule      = false;
#endif
#endif
static int         iChannelChange           = CHANNEL_CHANGE_CLOSE;
static int         iSwitchOK                = false;  
static int         iTimeFormat              = TIME_FORMAT_12H;
static int         iChannelOrder            = CHANNEL_ORDER_DOWN;
static int         iChannelNumber           = false;
static int         iRecDlgRed               = false;  
static int         iInfoSymbols             = false;  
static int         iSwitchTimer             = false;  
static int         iSwitchMinsBefore        = 1;
static int         iRemoteTimer             = false;  
static int         iEpgImages               = false;  
static int         iResizeImages            = 0;
static int         iImageExtension          = 0;
static std::string sThemeName               = "default";
static std::string sThemeDir                = "";
static std::string sEpgImagesDir            = "/video/epgimages";
static int         iVDRSymbols              = false;  
cPlugin*           pEPGSearch              = NULL;
cPlugin*           pRemoteTimers           = NULL;

const char *imageExtensionTexts[3] = { "png", "jpg", "xpm" };

/**
 * Pirvate Classes/Function Prototypes
 */

/**
 * Class/Function Implementaion
 */
struct tGeom {
   int x;
   int y;
   int w;
   int h;
};

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

/*
 *****************************************************************************
 * Icons
 *****************************************************************************
 */
class Icons
{
  private:
    static bool IsUTF8;
  public:
    static void InitCharSet();
    static const char* ArrowCCW(){return IsUTF8?"\ue000":"\x80";}
    static const char* Recording(){return IsUTF8?"\ue00b":"\x8b";}
    static const char* Watch(){return IsUTF8?"\ue00c":"\x8c";}
    static const char* WatchUpperHalf(){return IsUTF8?"\ue014":"\x94";}
    static const char* WatchLowerHalf(){return IsUTF8?"\ue015":"\x95";}
    static const char* Running(){return IsUTF8?"\ue012":"\x92";}
    static const char* VPS(){return IsUTF8?"\ue013":"\x93";}
    static const char* Blank(){return IsUTF8?"\ue003":"\x83";}
};

bool Icons::IsUTF8=false;

void Icons::InitCharSet()
{
  // Taken from VDR's vdr.c
  char *CodeSet=NULL;
  if(setlocale(LC_CTYPE, ""))
    CodeSet=nl_langinfo(CODESET);
  else
  {
    char *LangEnv=getenv("LANG"); // last resort in case locale stuff isn't installed
    if(LangEnv)
    {
      CodeSet=strchr(LangEnv,'.');
      if(CodeSet)
        CodeSet++; // skip the dot
    }
  }

  if(CodeSet && strcasestr(CodeSet,"UTF-8")!=0)
    IsUTF8=true;
    
  cStringList fontlist;
  cFont::GetAvailableFontNames(&fontlist);
  if (fontlist.Find("VDRSymbols Sans:Book") > 0) {
     iVDRSymbols =true;
     YAEPG_INFO("Found VDRSymbols font. InfoSymbols enabled");
  }
}

/*
 *****************************************************************************
 * cYaepgTheme
 *****************************************************************************
 */
class cYaepgTheme {
public:
   enum eElementType {
      THEME_ELEM_FIRST,
      THEME_IMAGE = THEME_ELEM_FIRST,
      THEME_FONT,
      THEME_COLOR,
      THEME_GEOM,
      THEME_IVAL,
      THEME_ELEM_LAST = THEME_IVAL
   };

   struct tThemeElement {
      eElementType type;
      bool init;
      union {
         tColor color;
         tGeom geom;
         cFont *font;
         cBitmap *bmp;
         int ival;
      } u;
   };

private:
   static cYaepgTheme *instance;

   std::map< std::string, tThemeElement > themeMap;
   std::vector< cBitmap * > themeImages;
   std::vector <cFont * > themeFonts;
   std::map< std::string, int > fontMap;

   cYaepgTheme(void);
   ~cYaepgTheme();

   void RemoveBlanks(char *s1);
   void RemoveQuotes(char *s1);
   int LoadImage(char *Filename);
   int LoadFont(char *Font);
   tColor ParseColor(char *Color) { return (tColor)strtoul(Color, NULL, 16); }
   tGeom ParseGeom(char *Geom);
   int ParseInt(char *Int) { return (int)strtoul(Int, NULL, 0); }
   bool Check(void);
   bool AddElement(const char *name, eElementType type);

public:
   static cYaepgTheme *Instance(void);
   static void Destroy(void);
   bool Load(std::string Theme);
   static void Themes(char ***_themes, int *_numThemes);
   tThemeElement Element(const char *name) { return themeMap[std::string(name)]; }
};

cYaepgTheme *cYaepgTheme::instance = NULL;

cYaepgTheme::cYaepgTheme(void)
{
   themeImages.clear();
   themeFonts.clear();
   fontMap.clear();

   AddElement("bgImage", THEME_IMAGE);
   AddElement("gridEventFont", THEME_FONT);
   AddElement("gridChanFont", THEME_FONT);
   AddElement("gridTimeFont", THEME_FONT);
   AddElement("gridDateFont", THEME_FONT);
   AddElement("eventTitleFont", THEME_FONT);
   AddElement("eventInfoFont", THEME_FONT);
   AddElement("eventTimeFont", THEME_FONT);
   AddElement("eventDescFont", THEME_FONT);
   AddElement("eventDateFont", THEME_FONT);
   AddElement("helpFont", THEME_FONT);
   AddElement("gridEventColor", THEME_COLOR);
   AddElement("gridSelFg", THEME_COLOR);
   AddElement("gridSelBg", THEME_COLOR);
   AddElement("gridSepColor", THEME_COLOR);
   AddElement("gridChanColor", THEME_COLOR);
   AddElement("gridTimeColor", THEME_COLOR);
   AddElement("gridDateColor", THEME_COLOR);
   AddElement("eventTitleColor", THEME_COLOR);
   AddElement("eventInfoColor", THEME_COLOR);
   AddElement("eventTimeColor", THEME_COLOR);
   AddElement("eventDescColor", THEME_COLOR);
   AddElement("eventDateColor", THEME_COLOR);
   AddElement("tlineBoxColor", THEME_COLOR);
   AddElement("helpColor", THEME_COLOR);
   AddElement("gridEventGeom", THEME_GEOM);
   AddElement("gridChanGeom", THEME_GEOM);
   AddElement("gridTimeGeom", THEME_GEOM);
   AddElement("gridDateGeom", THEME_GEOM);
   AddElement("eventTitleGeom", THEME_GEOM);
   AddElement("eventInfoGeom", THEME_GEOM);
   AddElement("eventTimeGeom", THEME_GEOM);
   AddElement("eventDescGeom", THEME_GEOM);
   AddElement("eventDateGeom", THEME_GEOM);
   AddElement("eventEpgImageGeom", THEME_GEOM);
   AddElement("tlineLocGeom", THEME_GEOM);
   AddElement("tlineBoxGeom", THEME_GEOM);
   AddElement("vidWinGeom", THEME_GEOM);
   AddElement("helpGeom", THEME_GEOM);
   AddElement("gridHorizSpace", THEME_IVAL);
   AddElement("gridNumChans", THEME_IVAL);
   AddElement("leftArrowWidth", THEME_IVAL);
   AddElement("rightArrowWidth", THEME_IVAL);
   AddElement("textBorder", THEME_IVAL);
   AddElement("textSpace", THEME_IVAL);
   AddElement("eventInfoAlign", THEME_IVAL);

   AddElement("recDlgImage", THEME_IMAGE);
   AddElement("recDlgGeom", THEME_GEOM);
   AddElement("recDlgColor", THEME_COLOR);
   AddElement("recDlgFont", THEME_FONT);
   AddElement("recTitleGeom", THEME_GEOM);
   AddElement("recTimeGeom", THEME_GEOM);
   AddElement("recStartGeom", THEME_GEOM);
   AddElement("recEndGeom", THEME_GEOM);
   AddElement("recFreqGeom", THEME_GEOM);
   AddElement("recStInpGeom", THEME_GEOM);
   AddElement("recEnInpGeom", THEME_GEOM);
   AddElement("recFrInpGeom", THEME_GEOM);

   AddElement("msgBgImage", THEME_IMAGE);
   AddElement("msgBoxFont", THEME_FONT);
   AddElement("msgBoxGeom", THEME_GEOM);
   AddElement("msgBoxColor", THEME_COLOR);
}

cYaepgTheme *
cYaepgTheme::Instance(void)
{
   if (instance == NULL) {
      instance = new cYaepgTheme;
   }
   return instance;
}

void
cYaepgTheme::Destroy(void)
{
   cYaepgTheme *theme = Instance();

   if (theme != NULL) {
      std::vector< cBitmap *>::iterator it1;
      for (it1 = theme->themeImages.begin();
           it1 != theme->themeImages.end();
           it1++) {
         delete *it1;
      }
      std::vector< cFont *>::iterator it2;
      for (it2 = theme->themeFonts.begin();
           it2 != theme->themeFonts.end();
           it2++) {
         delete *it2;
      }
      instance = NULL;
   }
}

void
cYaepgTheme::Themes(char ***_themes, int *_numThemes)
{
   DIR *dir;
   struct dirent *dp;
   char **themes = NULL;
   int numThemes = 0;

   *_numThemes = 0;
   *_themes = NULL;

   dir = opendir(sThemeDir.c_str());
   if (dir == NULL) {
      perror("opendir");
      return;
   }

   while ((dp = readdir(dir)) != NULL) {
      char *ext = strrchr(dp->d_name, '.');
      if (ext == NULL) {
         continue;
      }
      if (strcmp(ext + 1, "theme") != 0) {
         continue;
      }

      *ext = '\0';
      themes = (char **) realloc(themes, sizeof(char *) * (numThemes + 1));
      themes[numThemes++] = strdup(dp->d_name);

      YAEPG_INFO("Found theme: %s", dp->d_name);
   }
   *_themes = themes;
   *_numThemes = numThemes;

   return;
}


bool
cYaepgTheme::AddElement(const char *name, eElementType type)
{
   std::string elemName(name);

   if (themeMap.find(elemName) != themeMap.end()) {
      YAEPG_ERROR("Duplicate theme element definition '%s'", elemName.c_str());
      return false;
   }
   if (type < THEME_ELEM_FIRST || type > THEME_ELEM_LAST) {
      YAEPG_ERROR("Invalid theme element type %d", type);
      return false;
   }
   themeMap[elemName].type = type;
   themeMap[elemName].init = false;

   return true;
}

bool
cYaepgTheme::Load(std::string Theme)
{
   char themeFile[128], lineBuf[128], *s, *key, *val;
   FILE *fp;

   YAEPG_INFO("Loading theme: %s", Theme.c_str());

   snprintf(themeFile, sizeof(themeFile), "%s/%s.theme", sThemeDir.c_str(), Theme.c_str());

   fp = fopen(themeFile, "r");
   if (fp == NULL) {
      YAEPG_ERROR("Could not open teme file: %s", Theme.c_str());
      return false;
   }

   while ((s = fgets(lineBuf, sizeof(lineBuf), fp)) != NULL) {
      /* Remove all whitespace and trailing \n */
      RemoveBlanks(s);

      /* Ignore comments and empty lines */
      if (*s == '#' || *s == '\0') {
         continue;
      }

      /* Split the key/value pair */
      key = s;
      val = strchr(s, '=');
      if (val == NULL) {
         continue;
      }
      *val++ = '\0';

      /* If the value has quotes remove them */
      RemoveQuotes(val);

      if (themeMap.find(std::string(key)) == themeMap.end()) {
         YAEPG_ERROR("Unknown key value '%s'", key);
         continue;
      }

      tThemeElement &e = themeMap[std::string(key)];
      int bmpIndex, fntIndex;

      /* Call the appropriate parsing function based on the type */
      switch (e.type) {
      case THEME_IMAGE:
         bmpIndex = LoadImage(val);
         if (bmpIndex == -1) {
            YAEPG_ERROR("Error loading image '%s = %s'", key, val);
            fclose(fp);
            return false;
         }
         e.u.bmp = themeImages[bmpIndex];
         break;
      case THEME_FONT:
         fntIndex = LoadFont(val);
         if (fntIndex == -1) {
            YAEPG_ERROR("Error loading font '%s = %s'", key, val);
            fclose(fp);
            return false;
         }
         e.u.font = themeFonts[fntIndex];
         break;
      case THEME_COLOR:
         e.u.color = ParseColor(val);
         break;
      case THEME_GEOM:
         e.u.geom = ParseGeom(val);
         break;
      case THEME_IVAL:
         e.u.ival = ParseInt(val);
         break;
      default:
         ASSERT(0);
         break;
      }
      e.init = true;
   }

   return true;
}

void
cYaepgTheme::RemoveBlanks(char *s1)
{
   char *s2 = s1;
   int inQuote = 0;

   while (*s1 != '\0') {
      if (*s1 == '"') {
         inQuote ^= 1;
      }
      if (inQuote || !isspace(*s1)) {
         *s2++ = *s1;
      }
      s1++;
   }
   *s2 = '\0';
   if (inQuote) {
      YAEPG_ERROR("Umatched quote %s", s1);
   }
}

void
cYaepgTheme::RemoveQuotes(char *s1)
{
   char *s2 = s1;

   while (*s1 != '\0') {
      if (*s1 != '"') {
         *s2++ = *s1;
      }
      s1++;
   }
   *s2 = '\0';
}

int
cYaepgTheme::LoadImage(char *Filename)
{
   std::vector< Magick::Image > images;
   char fullFilePath[128];
   cBitmap *bmp = NULL;
   int index = -1;

   snprintf(fullFilePath, sizeof(fullFilePath), "%s/%s", sThemeDir.c_str(), Filename);

   YAEPG_INFO("Loading image '%s'", fullFilePath);

   try {
      int w, h;
      readImages(&images, fullFilePath);
      if (images.size() == 0) {
         YAEPG_ERROR("Couldn't load %s", fullFilePath);
         return -1;
      }
      if (images.size() > 1) {
         YAEPG_ERROR("Animated images not supported %s", fullFilePath);
         return -1;
      }
      w = images[0].columns();
      h = images[0].rows();

      bmp = new cBitmap(w, h, images[0].depth());

      const Magick::PixelPacket *pix = images[0].getConstPixels(0, 0, w, h);
      for (int iy = 0; iy < h; ++iy) {  
         for (int ix = 0; ix < w; ++ix) {
            tColor col = (~(int)(pix->opacity * 255 / MaxRGB) << 24) |
                          ((int)(pix->red * 255 / MaxRGB) << 16) |
                          ((int)(pix->green * 255 / MaxRGB) << 8) |
                           (int)(pix->blue * 255 / MaxRGB);
            bmp->DrawPixel(ix, iy, col);
            ++pix;
         }
      }
      index = themeImages.size();
      themeImages.push_back(bmp);
   } catch (Magick::Exception &e) {
      YAEPG_ERROR("Couldn't load %s: %s", fullFilePath, e.what());
      delete bmp;
      return -1;
   } catch (...) {
      YAEPG_ERROR("Couldn't load %s: Unknown exception caught", fullFilePath);
      delete bmp;
      return -1;
   }

   return index;
}

int
cYaepgTheme::LoadFont(char *Font)
{
   cFont *newFont;
   std::string fontString(Font);
   char *fontName, *fontSize;
   int fontIndex;

   /* Have we already loaded this font ? */
   if (fontMap.find(fontString) != fontMap.end()) {
      return fontMap.find(fontString)->second;
   }

   /* Split the name/size fields */
   fontName = Font;
   fontSize = strchr(fontName, ';');
   if (fontSize == NULL) {
      YAEPG_ERROR("Invalid font, missing size [<font>;<size>] %s", Font);
   }
   *fontSize++ = '\0';

   /* Add the font to fontMap and fontVector */
   YAEPG_INFO("Loading font '%s'", fontName);
   newFont = cFont::CreateFont(fontName, (int)strtoul(fontSize, NULL, 10));
   if (newFont == NULL) {
      ASSERT(0);
      YAEPG_ERROR("Could not load font %s", Font);
      return -1;
   }
   fontIndex = themeFonts.size();
   themeFonts.push_back(newFont);
   fontMap[fontString] = fontIndex;

   return fontIndex;
}

tGeom
cYaepgTheme::ParseGeom(char *Geom)
{
   tGeom g = { 0, 0, 0, 0 };
   char *val, *d;

   val = Geom;
   d = strchr(val, ',');
   if (d == NULL) {
      YAEPG_ERROR("Invalid geometry %s", val);
      return g;
   }
   *d++ = '\0';
   g.x = strtoul(val, NULL, 0);

   val = d;
   d = strchr(val, ',');
   if (d == NULL) {
      YAEPG_ERROR("Invalid geometry %s", val);
      return g;
   }
   *d++ = '\0';
   g.y = strtoul(val, NULL, 0);

   val = d;
   d = strchr(val, ',');
   if (d == NULL) {
      YAEPG_ERROR("Invalid geometry %s", val);
      return g;
   }
   *d++ = '\0';
   g.w = strtoul(val, NULL, 0);
   g.h = strtoul(d, NULL, 0);

   return g;
}

/*
 *****************************************************************************
 * cYaepgTextBox
 *****************************************************************************
 */
enum eTextFlags {
   TBOX_VALIGN_LEFT     = 0x00000001,
   TBOX_VALIGN_CENTER   = 0x00000002,
   TBOX_VALIGN_RIGHT    = 0x00000004,
   TBOX_VALIGN_FLAGS    = 0x00000007,
   TBOX_HALIGN_TOP      = 0x00000008,
   TBOX_HALIGN_CENTER   = 0x00000010,
   TBOX_HALIGN_BOTTOM   = 0x00000020,
   TBOX_HALIGN_FLAGS    = 0x00000038,
   TBOX_WRAP            = 0x00000040,
   TBOX_ARROW_LEFT      = 0x00000080,
   TBOX_ARROW_RIGHT     = 0x00000100
};

class cYaepgTextBox {
private:
   struct sTextLine {
      std::string text;
      tGeom geom;
   };

   std::string text;
   cFont *font;
   tColor fgColor;
   tColor bgColor;
   cBitmap *bgImage;
   cBitmap *bitmap;
   eTextFlags flags;
   tGeom geom;
   std::vector< sTextLine > fmtText;

public:
   cYaepgTextBox(void);
   ~cYaepgTextBox() { delete bitmap; }
   void Text(const char *_text) { text.assign(_text); }
   void Font(cFont *_font) { font = _font; }
   void Flags(eTextFlags _flags) { flags = _flags; }
   eTextFlags Flags(void) { return flags; }
   void SetFlags(eTextFlags _flags) { flags = (eTextFlags)(flags | _flags); }
   void FgColor(tColor color) { fgColor = color; }
   void BgColor(tColor color) { bgColor = color; }
   void BgImage(cBitmap *bmp) { bgImage = bmp; }
   void X(int _x) { geom.x = _x; }
   void Y(int _y) { geom.y = _y; }
   void W(int _w) { geom.w = _w; }
   void H(int _h) { geom.h = _h; }
   int X(void) { return geom.x; }
   int Y(void) { return geom.y; }
   int W(void) { return geom.w; }
   int H(void) { return geom.h; }
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgTextBox::cYaepgTextBox(void) :
   text(""),
   font(NULL),
   fgColor(clrTransparent),
   bgColor(clrTransparent),
   bgImage(NULL),
   bitmap(NULL),
   flags((eTextFlags)0)
{
   geom.x = 0;
   geom.y = 0;
   geom.w = 0;
   geom.h = 0;
}

void
cYaepgTextBox::Generate(void)
{
   /* Calulate width available for text */
   int boxWidth = geom.w - (2 * TEXT_BORDER);
   if (boxWidth <= 0) {
      YAEPG_INFO("Box too small for text (%d %d %d)",
                 geom.w, boxWidth, TEXT_BORDER);
      fmtText.clear();
      return;
   }

   /* Calculate how many lines of text we can fit into the box */
   int numLines = geom.h / (font->Height() + TEXT_SPACE);
   if (numLines == 0) {
      numLines = 1;
   }

   /*
    * Allocate a temporary string for parsing.  Add 4 chars to make space for
    * adding "..." if the line does not fit in the box.
    */
   char *tokText = (char *) malloc(strlen(text.c_str()) + 4);
   memset(tokText, 0, strlen(text.c_str()) + 4);
   strcpy(tokText, text.c_str());

   /* Remove trailing spaces */
   char *s = tokText + strlen(tokText) - 1;
   while (strlen(tokText) && *s == ' ') {
      *s-- = '\0';
   }

      /* Remove newlines in descriptions */
   char *newlineText = tokText;
   strreplace(newlineText, '\n', ' ');
   
   /* Break text up into lines */
   char *line, *nextLine = tokText;

   fmtText.clear();
   nextLine = tokText;
   if ((flags & TBOX_WRAP) && (numLines > 1)) {
      char *d, *od;

      do {
         line = nextLine;
         nextLine = NULL;
         d = NULL;

         /* Move the NUL char back one word at a time */
         while (font->Width(line) > boxWidth) {
            od = d;
            d = strrchr(line, ' ');
            if (od != NULL) {
               *od = ' ';
            }
            if (d == NULL) {
               break;
            }
            *d = '\0';
            nextLine = d + 1;
         }

         /* Remove initial spaces */
         while (*line == ' ') {
            line++;
         }

         fmtText.resize(fmtText.size() + 1);
         fmtText.back().text.assign(line);
      } while (nextLine && ((int)fmtText.size() < (numLines - 1)));
   }

   if (nextLine) {
      fmtText.resize(fmtText.size() + 1);
      fmtText.back().text.assign(nextLine);
   }

   /* The code above does not format the last line */
   line = (char *) malloc(strlen(fmtText.back().text.c_str()) + 4);
   strcpy(line, fmtText.back().text.c_str());
   if (font->Width(line) > boxWidth) {
      strcpy(line + (strlen(line) - 1), "...");
      while (font->Width(line) > boxWidth) {
         switch (strlen(line)) {
         case 0:
            YAEPG_INFO("Zero length string");
            goto out;
         default:
            line[strlen(line) - 4] = '.';
         case 1:
         case 2:
         case 3:
            line[strlen(line) - 1] = '\0';
            break;
         }
      }
   }

out:
   fmtText.back().text.assign(line);
   free(line);

   /* Figure out the initial y offset */
   int yOff = 0, yDelta, boxHeight;

   boxHeight = (fmtText.size() * font->Height()) +
               ((fmtText.size() - 1) * TEXT_SPACE);
   yDelta = font->Height() + TEXT_SPACE;
   switch (flags & TBOX_HALIGN_FLAGS) {
   case TBOX_HALIGN_TOP:
      yOff = geom.y + TEXT_BORDER;
      break;
   case TBOX_HALIGN_CENTER:
      yOff = geom.y + (geom.h / 2) - (boxHeight / 2);
      break;
   case TBOX_HALIGN_BOTTOM:
      yOff = geom.y + geom.h - TEXT_BORDER - boxHeight;
      break;
   default:
      ASSERT(0);
      break;
   }

   /* Fill in the x/y coordinates for each line */
   for (int i = 0; i < (int)fmtText.size(); i++) {
      switch (flags & TBOX_VALIGN_FLAGS) {
      case TBOX_VALIGN_LEFT:
         fmtText[i].geom.x = geom.x + TEXT_BORDER;
         break;
      case TBOX_VALIGN_CENTER:
         fmtText[i].geom.x = geom.x + (geom.w / 2) -
                               (font->Width(fmtText[i].text.c_str()) / 2);
         break;
      case TBOX_VALIGN_RIGHT:
         fmtText[i].geom.x = geom.x + geom.w - TEXT_BORDER -
                             font->Width(fmtText[i].text.c_str());
         break;
      default:
         ASSERT(0);
         break;
      }
      fmtText[i].geom.y = yOff;
      yOff += yDelta;
   }

   free(tokText);

   return;
}

void
cYaepgTextBox::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing text box (%d)", fmtText.size());

   for (int i = 0; i < (int)fmtText.size(); i++) {
      /* Fill in background color */
      if (bgColor != clrTransparent) {
         bmp->DrawRectangle(geom.x, geom.y,
                            geom.x + (geom.w - 1), geom.y + (geom.h - 1),
                            bgColor);
      }

      /* Draw the text */
      YAEPG_INFO("Drawing text '%s' at (%d %d color #%08X",
                 fmtText[i].text.c_str(), fmtText[i].geom.x,
                 fmtText[i].geom.y, fgColor);
      YAEPG_INFO("Text widht %d box widht %d",
                 font->Width(fmtText[i].text.c_str()), geom.w);

      bmp->DrawText(fmtText[i].geom.x, fmtText[i].geom.y,
                    fmtText[i].text.c_str(), fgColor, bgColor, font);
   }
}

/*
 *****************************************************************************
 * cYaepgGrid
 *****************************************************************************
 */
enum eCursorDir {
   DIR_UP,
   DIR_DOWN,
   DIR_LEFT,
   DIR_RIGHT
};

class cYaepgGrid {
private:
   class cNoInfoEvent : public cEvent {
   private:
      time_t startTime;

   public:
      cNoInfoEvent(time_t);
   };

   struct tYaepgEvent {
      const cEvent *event;
      cYaepgTextBox box;
   };

   cYaepgTextBox leftBox;
   cYaepgTextBox rightBox;

   tGeom geom;
   int startTime;
   int horizSpace;
   float gridRowHeight;
   float gridPixPerMin;
   std::vector< cChannel * > &chanVec;
   std::vector< std::vector< tYaepgEvent > > events;
   std::vector< cYaepgTextBox > leftArrows;
   std::vector< cYaepgTextBox > rightArrows;
   std::vector< const cEvent * > noInfoEvents;
   int curX;
   int curY;

   void FixCursor(void);

public:
   cYaepgGrid(std::vector< cChannel * > &chans, int time);
   ~cYaepgGrid();
   void UpdateTime(time_t newTime) { startTime = newTime; Generate(); }
   void UpdateChans(std::vector< cChannel * > &chans) { chanVec = chans; Generate(); }
   bool MoveCursor(eCursorDir dir);
   const cEvent *Event(void) { return events[curY][curX].event; }
   void Row(int row);
   void Col(int col);
   int Row(void) { return curY; }
   int Col(void) { return curX; }
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgGrid::cNoInfoEvent::cNoInfoEvent(time_t startTime) :
   cEvent(0)
{
   SetStartTime(startTime);
   SetDuration(9000);
   SetTitle(tr("No Info"));
   SetDescription(tr("No Info"));
}

cYaepgGrid::cYaepgGrid(std::vector< cChannel *> &chans, int time) :
   startTime(time),
   chanVec(chans),
   curX(0),
   curY(0)
{
   noInfoEvents.clear();
   geom = GRID_EVENT_GEOM;
   horizSpace = GRID_HORIZ_SPACE;
   gridRowHeight = (float)(geom.h - ((chanVec.size() - 1) * horizSpace)) /
                   (float)chanVec.size();
   gridPixPerMin = (float)geom.w / (float)90;
   leftArrows.resize(chanVec.size());
   rightArrows.resize(chanVec.size());
   Generate();
}

cYaepgGrid::~cYaepgGrid()
{
   std::vector< const cEvent *>::iterator it;

   for (it = noInfoEvents.begin();
        it != noInfoEvents.end();
        it ++) {
      delete *it;
   }
   noInfoEvents.clear();
}

void
cYaepgGrid::FixCursor(void)
{
   if (curY < 0) {
      curY = 0;
   }
   if (curY >= (int)events.size()) {
      curY = events.size() - 1;
   }

   if (curX < 0) {
      curX = 0;
   }
   if (curX >= (int)events[curY].size()) {
      curX = events[curY].size() - 1;
   }
}

void
cYaepgGrid::Row(int row)
{
   curY = row;
   FixCursor();
}

void
cYaepgGrid::Col(int col)
{
   curX = col;
   FixCursor();
}

void
cYaepgGrid::Generate(void)
{
   YAEPG_INFO("Generating grid");

   const cSchedule *curSched;
   const cEvent *curEvent;
   time_t curTime, endTime;
   time_t evStart, evDuration;
   eTextFlags evFlags;
   time_t gridStart;

   gridStart = startTime - (startTime % 1800);
   events.clear();
   events.resize(chanVec.size());
   cSchedulesLock SchedulesLock;
   const cSchedules* Schedules = cSchedules::Schedules(SchedulesLock);
   for (int i = 0; i < (int)chanVec.size(); i++) {
      curSched = Schedules->GetSchedule(chanVec[i]->GetChannelID());
      curTime = gridStart;
      endTime = curTime + 5400;
      int j = 0;

      while (curTime < endTime) {
         events[i].resize(events[i].size() + 1);
         if (curSched != NULL) {
            curEvent = curSched->GetEventAround(curTime);
            if ((curEvent != NULL) &&
                (curEvent->StartTime() + curEvent->Duration()) <= curTime) {
               curEvent = NULL;
            }
         } else {
            curEvent = NULL;
         }

         if (curEvent == NULL) {
            curEvent = new cNoInfoEvent(curTime);
            noInfoEvents.push_back(curEvent);
         }

         evFlags = (eTextFlags)0;
         evStart = curEvent->StartTime();
         evDuration = curEvent->Duration();
         if (evStart < gridStart) {
            evFlags = (eTextFlags)(evFlags | TBOX_ARROW_LEFT);
            evStart = gridStart;
            evDuration -= gridStart - curEvent->StartTime();
         }
         if ((evStart + evDuration) > endTime) {
            evFlags = (eTextFlags)(evFlags | TBOX_ARROW_RIGHT);
            evDuration = endTime - evStart;
         }

         ASSERT(evDuration <= 5400);
         ASSERT(evStart >= curTime);
         ASSERT(evStart + evDuration <= endTime);

         events[i][j].event = curEvent;
         events[i][j].box.Text(curEvent->Title());
         events[i][j].box.Font(GRID_EVENT_FONT);
         events[i][j].box.FgColor(GRID_EVENT_COLOR);
         events[i][j].box.BgColor(clrTransparent);
         events[i][j].box.Flags((eTextFlags)(evFlags | TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
         events[i][j].box.X(geom.x + ROUND((float)((evStart - gridStart) / 60) * gridPixPerMin));
         events[i][j].box.Y(geom.y + ROUND(((float)i * (gridRowHeight + (float)horizSpace))));
         events[i][j].box.W(ROUND((float)(evDuration / 60) * gridPixPerMin));
         events[i][j].box.H(ROUND(gridRowHeight));
         events[i][j].box.Generate();

         YAEPG_INFO("Event [%d][%d] (%d %d, %d %d) '%s'", i, j,
                    events[i][j].box.X(), events[i][j].box.Y(),
                    events[i][j].box.W(), events[i][j].box.H(),
                    curEvent->Title());

         curTime = curEvent->StartTime() + curEvent->Duration();
         j++;
      }

      /*
       * Generate the arrows
       * XXX Most of the arrow initialization could be done once when the grid is constructed
       */
      leftArrows[i].Text("<");
      leftArrows[i].Font(GRID_EVENT_FONT);
      leftArrows[i].FgColor(GRID_EVENT_COLOR);
      leftArrows[i].BgColor(clrTransparent);
      leftArrows[i].Flags((eTextFlags)(TBOX_VALIGN_RIGHT | TBOX_HALIGN_CENTER));
      leftArrows[i].X(geom.x - LEFT_ARROW_WIDTH);
      leftArrows[i].Y(geom.y + ROUND(((float)i * (gridRowHeight + (float)horizSpace))));
      leftArrows[i].W(LEFT_ARROW_WIDTH);
      leftArrows[i].H(ROUND(gridRowHeight));
      leftArrows[i].Generate();

      rightArrows[i].Text(">");
      rightArrows[i].Font(GRID_EVENT_FONT);
      rightArrows[i].FgColor(GRID_EVENT_COLOR);
      rightArrows[i].BgColor(clrTransparent);
      rightArrows[i].Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
      rightArrows[i].X(geom.x + geom.w);
      rightArrows[i].Y(geom.y + ROUND(((float)i * (gridRowHeight + (float)horizSpace))));
      rightArrows[i].W(RIGHT_ARROW_WIDTH);
      rightArrows[i].H(ROUND(gridRowHeight));
      rightArrows[i].Generate();
   }

   FixCursor();
}

bool
cYaepgGrid::MoveCursor(eCursorDir dir)
{
   switch (dir) {
   case DIR_UP:
      if (curY == 0) {
         return false;
      }
      curY--;
      break;
   case DIR_DOWN:
      if (curY == (int)(events.size() - 1)) {
         return false;
      }
      curY++;
      break;
   case DIR_LEFT:
      if (curX == 0) {
         return false;
      }
      curX--;
      break;
   case DIR_RIGHT:
      if (curX == (int)(events[curY].size() - 1)) {
         return false;
      }
      curX++;
      break;
   default:
      ASSERT(0);
      break;
   }

   if (curX >= (int)events[curY].size()) {
      curX = events[curY].size() - 1;
   }

   ASSERT(curY >= 0 && curY < (int)events.size());
   ASSERT(curX >= 0 && curX < (int)events[curY].size());

   return true;
}

void
cYaepgGrid::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing grid at (%d %d)", geom.x, geom.y);

   for (int i = 0; i < (int)events.size(); i++) {
      for (int j = 0; j < (int)events[i].size(); j++) {
         /* Is this the currently selected event */
         if (i == curY && j == curX) {
            events[i][j].box.FgColor(GRID_SEL_FG);
            events[i][j].box.BgColor(GRID_SEL_BG);

            /* Check to see if the arrow is "selected" */
            if (j == 0) {
               leftArrows[i].FgColor(GRID_SEL_FG);
               leftArrows[i].BgColor(GRID_SEL_BG);
            } else {
               leftArrows[i].FgColor(GRID_EVENT_COLOR);
               leftArrows[i].BgColor(clrTransparent);
            }
            if (j == (int)(events[i].size() - 1)) {
               rightArrows[i].FgColor(GRID_SEL_FG);
               rightArrows[i].BgColor(GRID_SEL_BG);
            } else {
               rightArrows[i].FgColor(GRID_EVENT_COLOR);
               rightArrows[i].BgColor(clrTransparent);
            }
         } else {
            events[i][j].box.FgColor(GRID_EVENT_COLOR);
            events[i][j].box.BgColor(clrTransparent);
            leftArrows[i].FgColor(GRID_EVENT_COLOR);
            leftArrows[i].BgColor(clrTransparent);
            rightArrows[i].FgColor(GRID_EVENT_COLOR);
            rightArrows[i].BgColor(clrTransparent);
         }
         events[i][j].box.Draw(bmp);

         /* Draw the arrow boxes */
         if (events[i][j].box.Flags() & TBOX_ARROW_LEFT) {
            leftArrows[i].Text("<");
         } else {
            leftArrows[i].Text("");
         }
         leftArrows[i].Generate();
         leftArrows[i].Draw(bmp);

         if (events[i][j].box.Flags() & TBOX_ARROW_RIGHT) {
            rightArrows[i].Text(">");
         } else {
            rightArrows[i].Text("");
            rightArrows[i].BgColor(clrTransparent);
         }
         rightArrows[i].Generate();
         rightArrows[i].Draw(bmp);

         /* Draw a separator if there is no right arrow */
         if ((events[i][j].box.Flags() & TBOX_ARROW_RIGHT) == 0) {
            YAEPG_INFO("Drawing separator at (%d %d, %d %d)",
                       events[i][j].box.X() + events[i][j].box.W() - 1,
                       events[i][j].box.Y(),
                       events[i][j].box.X() + events[i][j].box.W(),
                       events[i][j].box.Y() + ROUND(gridRowHeight));

            bmp->DrawRectangle(events[i][j].box.X() + events[i][j].box.W() - 1,
                               events[i][j].box.Y(),
                               events[i][j].box.X() + events[i][j].box.W(),
                               events[i][j].box.Y() + ROUND(gridRowHeight),
                               GRID_SEP_COLOR);
         }
      } /* for j < events[i].size() */
   } /* for i < events.size() */
}

bool
cYaepgTheme::Check(void)
{
   std::map< std::string, tThemeElement >::iterator it;
   bool valid = true;

   for (it = themeMap.begin(); it != themeMap.end(); it++) {
      if (it->second.init == false) {
         YAEPG_ERROR("%s not defined!", it->first.c_str());
         valid = false;
      }
   }

   return valid;
}

/*
 *****************************************************************************
 * cYaepgGridChans
 *****************************************************************************
 */
class cYaepgGridChans {
private:
   struct tYaepgChan {
      cChannel *c;
      cYaepgTextBox numBox;
      cYaepgTextBox nameBox;
   };

   tGeom geom;
   std::vector< cChannel * > &chanVec;
   std::vector< tYaepgChan > chanInfo;
   float chanRowHeight;
   int horizSpace;

public:
   cYaepgGridChans(std::vector< cChannel * > &chans);
   void UpdateChans(std::vector< cChannel * > &chans) { chanVec = chans; Generate(); }
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgGridChans::cYaepgGridChans(std::vector< cChannel * > &chans) :
   chanVec(chans)
{
   geom = GRID_CHAN_GEOM;
   horizSpace = GRID_HORIZ_SPACE;
   chanRowHeight = (float)(geom.h - ((chanVec.size() - 1) * horizSpace)) /
                   (float)chanVec.size();
   chanInfo.resize(GRID_NUM_CHANS);
   Generate();
}

void
cYaepgGridChans::Generate(void)
{
   char numStr[16];

   for (int i = 0; i < GRID_NUM_CHANS; i++) {
      snprintf(numStr, sizeof(numStr), "%d", chanVec[i]->Number());
      chanInfo[i].numBox.Text(numStr);
      chanInfo[i].numBox.Font(GRID_CHAN_FONT);
      chanInfo[i].numBox.FgColor(GRID_CHAN_COLOR);
      chanInfo[i].numBox.BgColor(clrTransparent);
      chanInfo[i].numBox.Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
      chanInfo[i].numBox.X(geom.x);
      chanInfo[i].numBox.Y(geom.y + ROUND((float)i * (chanRowHeight + (float)horizSpace)));
      chanInfo[i].numBox.W(geom.w / 2);
      chanInfo[i].numBox.H(ROUND(chanRowHeight));
      chanInfo[i].numBox.Generate();
      chanInfo[i].nameBox.Text(chanVec[i]->Name());
      chanInfo[i].nameBox.Font(GRID_CHAN_FONT);
      chanInfo[i].nameBox.FgColor(GRID_CHAN_COLOR);
      chanInfo[i].nameBox.BgColor(clrTransparent);
      chanInfo[i].nameBox.Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
      
	  if (iChannelNumber)
	  {
	     chanInfo[i].nameBox.X(geom.x + (geom.w / 2));
		 chanInfo[i].nameBox.W(geom.w / 2);
	  }
	  else
	  {
	     chanInfo[i].nameBox.X(geom.x);
		 chanInfo[i].nameBox.W(geom.w );
	  }
	  
      chanInfo[i].nameBox.Y(geom.y + ROUND((float)i * (chanRowHeight + (float)horizSpace)));
      chanInfo[i].nameBox.H(ROUND(chanRowHeight));
      chanInfo[i].nameBox.Generate();

      YAEPG_INFO("Chan [%d] (%d %d, %d %d) '%s %s'", i,
                 chanInfo[i].numBox.X(), chanInfo[i].numBox.Y(),
                 chanInfo[i].numBox.W() + chanInfo[i].nameBox.W(),
                 chanInfo[i].numBox.H() + chanInfo[i].nameBox.H(),
                 numStr, chanVec[i]->Name());
   }
}

void
cYaepgGridChans::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing grid channels at (%d %d)", geom.x, geom.y);

   for (int i = 0; i < (int)chanInfo.size(); i++) {
       if (iChannelNumber)
	      chanInfo[i].numBox.Draw(bmp);
       chanInfo[i].nameBox.Draw(bmp);
   }
}

/*
 *****************************************************************************
 * cYaepgGridTime
 *****************************************************************************
 */
class cYaepgGridTime {
private:
   time_t startTime;
   std::vector< cYaepgTextBox > times;
   tGeom geom;

public:
   cYaepgGridTime(time_t _startTime);
   void UpdateTime(time_t _startTime) { startTime = _startTime; Generate(); }
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgGridTime::cYaepgGridTime(time_t _startTime) :
   startTime(_startTime)
{
   geom = GRID_TIME_GEOM;
   times.resize(3);
   Generate();
}

void
cYaepgGridTime::Generate(void)
{
   time_t curTime = startTime;
   struct tm locTime;
   char timeStr[32];
   int timeWidth;

   timeWidth = geom.w / 3;
   for (int i = 0; i < 3; i++) {
      localtime_r(&curTime, &locTime);
      locTime.tm_min = (locTime.tm_min >= 30) ? 30 : 0;
      if (iTimeFormat == TIME_FORMAT_24H) {
         snprintf(timeStr, sizeof(timeStr), "%02d:%02d",
                  locTime.tm_hour, locTime.tm_min);
      } else {
         snprintf(timeStr, sizeof(timeStr), "%d:%02d%s",
                  FMT_12HR(locTime.tm_hour), locTime.tm_min,
                  FMT_AMPM(locTime.tm_hour));
      }
      times[i].Text(timeStr);
      times[i].Font(GRID_TIME_FONT);
      times[i].FgColor(GRID_TIME_COLOR);
      times[i].BgColor(clrTransparent);
      times[i].Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
      times[i].X(geom.x + (i * timeWidth));
      times[i].Y(geom.y);
      times[i].W(timeWidth);
      times[i].H(geom.h);
      times[i].Generate();
      curTime += 1800;
   }
}

void
cYaepgGridTime::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing grid times at (%d %d)", geom.x, geom.y);

   for (int i = 0; i < (int)times.size(); i++) {
      times[i].Draw(bmp);
   }
}

/*
 *****************************************************************************
 * cYaepgGridDate
 *****************************************************************************
 */
class cYaepgGridDate {
private:
   tGeom geom;
   time_t t;
   char chanStr[16];
   char dateStr[16];
   cYaepgTextBox box;

public:
   cYaepgGridDate(time_t _t);
   void UpdateTime(time_t _t);
   void UpdateChan(int chanNum);
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgGridDate::cYaepgGridDate(time_t _t)
{
   geom = GRID_DATE_GEOM;
   chanStr[0] = '\0';
   UpdateTime(_t);
}

void
cYaepgGridDate::UpdateTime(time_t _t)
{
   sprintf(dateStr,"%s", *DateString(_t));
   Generate();
}

void
cYaepgGridDate::UpdateChan(int chanNum)
{
   if (chanNum == 0) {
      chanStr[0] = '\0';
   } else {
      snprintf(chanStr, sizeof(chanStr), "%d-", chanNum);
   }
   Generate();
}

void
cYaepgGridDate::Generate(void)
{
   box.Text(strlen(chanStr) ? chanStr : dateStr);
   box.Font(GRID_DATE_FONT);
   box.FgColor(GRID_DATE_COLOR);
   box.BgColor(clrTransparent);
   box.Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
   box.X(geom.x);
   box.Y(geom.y);
   box.W(geom.w);
   box.H(geom.h);
   box.Generate();
}

void
cYaepgGridDate::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing grid date at (%d %d)", geom.x, geom.y);

   box.Draw(bmp);
}

/*
 *****************************************************************************
 * cYaepgEventTitle
 *****************************************************************************
 */
class cYaepgEventTitle {
private:
   tGeom geom;
   const cEvent *event;
   cYaepgTextBox box;

public:
   cYaepgEventTitle(const cEvent *_event);
   void UpdateEvent(const cEvent *_event) { event = _event; Generate(); }
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgEventTitle::cYaepgEventTitle(const cEvent *_event) :
   event(_event)
{
   geom = EVENT_TITLE_GEOM;
   Generate();
}

void
cYaepgEventTitle::Generate(void)
{
   box.Text(event->Title());
   box.Font(EVENT_TITLE_FONT);
   box.FgColor(EVENT_TITLE_COLOR);
   box.BgColor(clrTransparent);
   box.Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_BOTTOM | TBOX_WRAP));
   box.X(geom.x);
   box.Y(geom.y);
   box.W(geom.w);
   box.H(geom.h);
   box.Generate();
}

void
cYaepgEventTitle::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing event title at (%d %d)", geom.x, geom.y);

   box.Draw(bmp);
}

/*
 *****************************************************************************
 * cYaepgEventInfo
 *****************************************************************************
 */
class cYaepgEventInfo {
private:
   tGeom geom;
   const cEvent *event;
   cYaepgTextBox boxes[3];

public:
   cYaepgEventInfo(const cEvent *_event);
   void UpdateEvent(const cEvent *_event) { event = _event; Generate(); }
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgEventInfo::cYaepgEventInfo(const cEvent *_event) :
   event(_event)
{
   geom = EVENT_INFO_GEOM;
   Generate();
}

void
cYaepgEventInfo::Generate(void)
{
   cString buffer[3];
   const char* t=NULL;
   const char* v=NULL;
   const char* r=NULL;
   
   eTimerMatch timerMatch=tmNone; 
   cTimer* ti;

   if (iRemoteTimer && pRemoteTimers && event) {  
      RemoteTimers_GetMatch_v1_0 rtMatch;
      rtMatch.event = event;
      pRemoteTimers->Service("RemoteTimers::GetMatch-v1.0", &rtMatch);
      timerMatch = (eTimerMatch)rtMatch.timerMatch;
      ti = rtMatch.timer;
   }
   else
      ti=Timers.GetMatch(event, &timerMatch);
   
   switch (timerMatch) {
      case tmFull:
         if (iInfoSymbols && iVDRSymbols) 
            t=ti->Recording()?Icons::Recording():Icons::Watch();    
         else 
            t=ti->Recording()?"R":"T";  
         break;
      case tmPartial:
         if (iInfoSymbols && iVDRSymbols) 
            t=ti->Recording()?Icons::Recording():Icons::WatchUpperHalf();    
         else
            t=ti->Recording()?"R":"t";
         break;
      default:
         t=" ";
         break;
   }   
      
   if (iSwitchTimer && pEPGSearch && event) {
      Epgsearch_switchtimer_v1_0* serviceData = new Epgsearch_switchtimer_v1_0;
	  serviceData->event = event;
	  serviceData->mode = 0;
      if (pEPGSearch->Service("Epgsearch-switchtimer-v1.0", serviceData)){
         if(serviceData->success){
            t=(iInfoSymbols && iVDRSymbols)?Icons::ArrowCCW():"S" ;
		 }
         delete serviceData;
      }   
   }   
   if (event->Vps() && (event->Vps() - event->StartTime()))
      v=(iInfoSymbols && iVDRSymbols)?Icons::VPS():"V";
   else
      v=" ";
         
   if (event->SeenWithin(30) && event->IsRunning())
      r=(iInfoSymbols && iVDRSymbols)?Icons::Running():"*";
   else
      r=" " ;
      
   switch (EVENT_INFO_ALIGN){
      case 2:
         buffer[0] = cString::sprintf("%s", r);
         buffer[1] = cString::sprintf("%s", v);
         buffer[2] = cString::sprintf("%s", t);
         break;
      case 1:
      default:
         buffer[0] = cString::sprintf("%s", t);
         buffer[1] = cString::sprintf("%s", v);
         buffer[2] = cString::sprintf("%s", r);
         break;
   }
   
   int boxWidth = geom.w / 3;
   for (int i = 0; i < 3; i++) {
      boxes[i].Text(buffer[i]);
      boxes[i].Font(EVENT_INFO_FONT);
      boxes[i].FgColor(EVENT_INFO_COLOR);
      boxes[i].BgColor(clrTransparent);
      boxes[i].Flags((eTextFlags)(TBOX_VALIGN_CENTER | TBOX_HALIGN_CENTER));
      boxes[i].X(geom.x + (i * boxWidth));
      boxes[i].Y(geom.y);
      boxes[i].W(boxWidth );
      boxes[i].H(geom.h);
      boxes[i].Generate();
   }
}

void
cYaepgEventInfo::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing event info at (%d %d)", geom.x, geom.y);
   
   for (int i = 0; i < 3; i++) {
      boxes[i].Draw(bmp);
   }
}

/*
 *****************************************************************************
 * cYaepgEventEpgImage
 *****************************************************************************
 */
class cYaepgEventEpgImage {
private:
   tGeom geom;
   const cEvent *event;
   cBitmap *EpgImage;

public:
   cYaepgEventEpgImage(const cEvent *_event);
   void UpdateEvent(const cEvent *_event) { event = _event; Generate(); }
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgEventEpgImage::cYaepgEventEpgImage(const cEvent *_event) :
   event(_event), EpgImage(NULL)
{
   geom = EVENT_EPGIMAGE_GEOM;
   Generate();
}

void
cYaepgEventEpgImage::Generate(void)
{
   Magick::Image image;
  
   char *strFilename = NULL;
   
   if (-1 == asprintf(&strFilename, "%s/%d.%s",sEpgImagesDir.c_str(),event->EventID(), imageExtensionTexts[iImageExtension]))
   {
	   YAEPG_ERROR("Couldn't built epg image filename!");
	   delete strFilename;
	   return;
   }
   
   try {
	  Geometry geo;
      image.read(strFilename);
      geo = image.size();
      int w = geo.width();
      int h = geo.height();
      if (geom.h != h || geom.w != w) {
         switch (iResizeImages) {
         case 0:
            image.sample(Geometry(geom.w, geom.h));
            break;
         case 1:
            image.scale(Geometry(geom.w, geom.h));
            break;
         case 2:
            image.zoom(Geometry(geom.w, geom.h));
            break;
         default:
             YAEPG_ERROR("ERROR: unknown resize mode %d", iResizeImages);
             break;
         }
         geo = image.size();
         w = geo.width();
         h = geo.height();
      }
      
      image.opacity(OpaqueOpacity);
      image.backgroundColor(Color(0, 0, 0, 0));
      image.quantizeColorSpace(RGBColorspace);
      image.quantizeColors(255);
      image.quantize();
     
      EpgImage = new cBitmap(geom.w, geom.h, image.depth());
      
      // center image
      int x = 0;
      int y = 0;
      x += ((geom.w - w) / 2);
      y += ((geom.h - h) / 2);

      const Magick::PixelPacket *pix = image.getConstPixels(0, 0, w, h);
      for (int iy = 0; iy < h; ++iy) {
        for (int ix = 0; ix < w; ++ix) {
          tColor col = (~(int)(pix->opacity * 255 / MaxRGB) << 24)
                     | ((int)(pix->red * 255 / MaxRGB) << 16)
                     | ((int)(pix->green * 255 / MaxRGB) << 8)
                     | (int)(pix->blue * 255 / MaxRGB);
          EpgImage->DrawPixel(x + ix, y + iy, col);
          ++pix;
        }
      }
      delete strFilename;
   } catch (Magick::Exception &e) {
      YAEPG_ERROR("Couldn't load epg image %s, %s ", fullFilePath, e.what());
      delete strFilename;
      delete EpgImage;
      EpgImage=NULL;
   } catch (...) {
      YAEPG_ERROR("Couldn't load %s: Unknown exception caught", fullFilePath);
      delete strFilename;
      delete EpgImage;
      EpgImage=NULL;
   }
}

void
cYaepgEventEpgImage::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing event epg image at (%d %d)", geom.x, geom.y);
   if (EpgImage)
      bmp->DrawBitmap(geom.x, geom.y, *EpgImage);
}

/*
 *****************************************************************************
 * cYaepgEventTime
 *****************************************************************************
 */
class cYaepgEventTime {
private:
   const cEvent *event;
   cYaepgTextBox box;
   tGeom geom;

public:
   cYaepgEventTime(const cEvent *_event);
   void UpdateEvent(const cEvent *_event) { event = _event; Generate(); }
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgEventTime::cYaepgEventTime(const cEvent *_event) :
   event(_event)
{
   geom = EVENT_TIME_GEOM;
   Generate();
}

void
cYaepgEventTime::Generate(void)
{
   struct tm locStart, locEnd;
   time_t t;
   char timeStr[32];

   t = event->StartTime();
   localtime_r(&t, &locStart);
   t += event->Duration();
   localtime_r(&t, &locEnd);
   if (iTimeFormat == TIME_FORMAT_24H) {
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d - %02d:%02d",
               locStart.tm_hour, locStart.tm_min,
               locEnd.tm_hour, locEnd.tm_min);
   } else {
      snprintf(timeStr, sizeof(timeStr), "%d:%02d%s - %d:%02d%s",
               FMT_12HR(locStart.tm_hour), locStart.tm_min,
               FMT_AMPM(locStart.tm_hour),
               FMT_12HR(locEnd.tm_hour), locEnd.tm_min,
               FMT_AMPM(locEnd.tm_hour));
   }

   box.Text(timeStr);
   box.Font(EVENT_TIME_FONT);
   box.FgColor(EVENT_TIME_COLOR);
   box.BgColor(clrTransparent);
   box.Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
   box.X(geom.x);
   box.Y(geom.y);
   box.W(geom.w);
   box.H(geom.h);
   box.Generate();
}

void
cYaepgEventTime::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing event time at (%d %d)", geom.x, geom.y);

   box.Draw(bmp);
}

/*
 *****************************************************************************
 * cYaepgEventDesc
 *****************************************************************************
 */
class cYaepgEventDesc {
private:
   const cEvent *event;
   cYaepgTextBox box;
   tGeom geom;

public:
   cYaepgEventDesc(const cEvent *_event);
   void UpdateEvent(const cEvent *_event) { event = _event; Generate(); }
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgEventDesc::cYaepgEventDesc(const cEvent *_event) :
   event(_event)
{
   geom = EVENT_DESC_GEOM;
   Generate();
}

void
cYaepgEventDesc::Generate(void)
{
   box.Text(event->Description() ? event->Description() : (event->ShortText() ? event->ShortText() : ""));
   box.Font(EVENT_DESC_FONT);
   box.FgColor(EVENT_DESC_COLOR);
   box.BgColor(clrTransparent);
   box.Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_TOP | TBOX_WRAP));
   box.X(geom.x);
   box.Y(geom.y);
   box.W(geom.w);
   box.H(geom.h);
   box.Generate();
}

void
cYaepgEventDesc::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing event description at (%d %d)", geom.x, geom.y);

   box.Draw(bmp);
}

/*
 *****************************************************************************
 * cYaepgEventDate
 *****************************************************************************
 */
class cYaepgEventDate {
private:
   cYaepgTextBox box;
   tGeom geom;

public:
   cYaepgEventDate(void);
   void Update(void) { Generate(); }
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgEventDate::cYaepgEventDate(void)
{
   geom = EVENT_DATE_GEOM;
   Generate();
}

void
cYaepgEventDate::Generate(void)
{
   char timeStr[128];
   struct tm locTime;

   time_t t = time(NULL);
   localtime_r(&t, &locTime);
   if (iTimeFormat == TIME_FORMAT_24H) {
      snprintf(timeStr, sizeof(timeStr), "%s %02d:%02d",
               *WeekDayName((locTime.tm_wday + 7) % 7), locTime.tm_hour, locTime.tm_min);
   } else {
      snprintf(timeStr, sizeof(timeStr), "%s %d:%02d%s",
               *WeekDayName((locTime.tm_wday + 7) % 7),
               FMT_12HR(locTime.tm_hour), locTime.tm_min,
               FMT_AMPM(locTime.tm_hour));
   }
   box.Text(timeStr);
   box.Font(EVENT_DATE_FONT);
   box.FgColor(EVENT_DATE_COLOR);
   box.BgColor(clrTransparent);
   box.Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
   box.X(geom.x);
   box.Y(geom.y);
   box.W(geom.w);
   box.H(geom.h);
   box.Generate();
}

void
cYaepgEventDate::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing event date at (%d %d)", geom.x, geom.y);

   box.Draw(bmp);
}

/*
 *****************************************************************************
 * cYaepgTimeLine
 *****************************************************************************
 */
class cYaepgTimeLine {
private:
   tGeom locGeom;
   tGeom boxGeom;
   tColor boxColor;
   time_t startTime;
   float pixPerMin;
   int xOff;
   bool hidden;

public:
   cYaepgTimeLine(time_t _startTime);
   void UpdateTime(time_t _startTime);
   void Generate(void);
   void Draw(cBitmap *bmp);
};

cYaepgTimeLine::cYaepgTimeLine(time_t _startTime) :
   hidden(true)
{
   locGeom = TLINE_LOC_GEOM;
   boxGeom = TLINE_BOX_GEOM;
   boxColor = TLINE_BOX_COLOR;
   pixPerMin = (float)locGeom.w / (float)90;
   UpdateTime(_startTime);
}

void
cYaepgTimeLine::UpdateTime(time_t _startTime)
{
   startTime = _startTime - (_startTime % 1800);
   Generate();
}

void
cYaepgTimeLine::Generate(void)
{
   if (startTime > time(NULL)) {
      hidden = true;
   } else {
      int minOff = (time(NULL) - startTime) / 60;
      ASSERT(minOff <= 30);
      xOff = locGeom.x + ROUND((float)minOff * pixPerMin) - (boxGeom.w / 2);
      hidden = false;
      YAEPG_INFO("Time line minOff %d pixPerMin %d xOff %d",
                 minOff, ROUND(pixPerMin), xOff);
   }
}

void
cYaepgTimeLine::Draw(cBitmap *bmp)
{
   YAEPG_INFO("Drawing time line at (%d %d) (%d %d) %d",
              xOff, locGeom.y,
              xOff + boxGeom.w, locGeom.y + (boxGeom.h - 1),
              hidden);

   if (!hidden) {
      bmp->DrawRectangle(xOff, locGeom.y,
                         xOff + boxGeom.w, locGeom.y + (boxGeom.h - 1),
                         boxColor);
   }
}

/*
 *****************************************************************************
 * cYaepgHelpBar
 *****************************************************************************
 */
class cYaepgHelpBar {
private:
   cYaepgTextBox boxes[4];
   static const char *helpStrs[4];
   tGeom geom;

public:
   cYaepgHelpBar(void);
   void Draw(cBitmap *bmp);
};

const char *cYaepgHelpBar::helpStrs[4] = {
   trVDR("Button$Record"),
   trNOOP("Page up"),
   trNOOP("Page down"),
   trVDR("Button$Switch")
};

cYaepgHelpBar::cYaepgHelpBar(void) 
{
   geom = HELP_BAR_GEOM;
   int boxWidth = geom.w / 4;
   for (int i = 0; i < 4; i++) {
      boxes[i].Text(tr(helpStrs[i]));
      boxes[i].Font(HELP_BAR_FONT);
      boxes[i].FgColor(HELP_BAR_COLOR);
      boxes[i].BgColor(clrTransparent);
      boxes[i].Flags((eTextFlags)(TBOX_VALIGN_CENTER | TBOX_HALIGN_CENTER));
      boxes[i].X(geom.x + (i * boxWidth));
      boxes[i].Y(geom.y);
      boxes[i].W(boxWidth );
      boxes[i].H(geom.h);
      boxes[i].Generate();
   }
}

void
cYaepgHelpBar::Draw(cBitmap *bmp)
{
   if (geom.w < 40) {
      return;
   }
   
   for (int i = 0; i < 4; i++) {
      boxes[i].Draw(bmp);
   }
}

/*
 *****************************************************************************
 * cYaepgInputTime
 *****************************************************************************
 */
class cYaepgInputTime : public cYaepgTextBox {
private:
   time_t t;

public:
   cYaepgInputTime(void) : t(0) {}
   void UpdateTime(time_t _t);
   eOSState ProcessKey(eKeys key);
   struct tm recTime;   
};

void
cYaepgInputTime::UpdateTime(time_t _t)
{
   struct tm locTime;
   char timeStr[32];

   t = _t;
   localtime_r(&t, &locTime);
   recTime = locTime;
   if (iTimeFormat == TIME_FORMAT_24H) {
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d",
               locTime.tm_hour, locTime.tm_min);
   } else {
      snprintf(timeStr, sizeof(timeStr), "%d:%02d%s",
               FMT_12HR(locTime.tm_hour), locTime.tm_min,
               FMT_AMPM(locTime.tm_hour));
   }
   Text(timeStr);
   Generate();
}

eOSState
cYaepgInputTime::ProcessKey(eKeys key)
{
   eOSState state = osContinue;

   switch (key & ~k_Repeat) {
   case kLeft:
      UpdateTime(t - 60);
      break;
   case kRight:
      UpdateTime(t + 60);
      break;
   default:
      state = osUnknown;
      break;
   }

   return state;
}

/*
 *****************************************************************************
 * cYaepgInputStra
 *****************************************************************************
 */
class cYaepgInputStra : public cYaepgTextBox {
private:
   std::vector< std::string > stra;
   int index;

   void UpdateIndex(int _index);

public:
   cYaepgInputStra(void) : index(0) {}
   void UpdateStra(char **_stra);
   eOSState ProcessKey(eKeys key);
};

void
cYaepgInputStra::UpdateStra(char **_stra)
{
   int i = 0;

   stra.clear();
   while (_stra[i] != NULL) {
      stra.push_back(std::string(_stra[i++]));
   }
   UpdateIndex(0);
}

void
cYaepgInputStra::UpdateIndex(int _index)
{
   if (_index < 0) {
      index = 0;
   }
   if (_index >= (int)stra.size()) {
      index = (int)stra.size() - 1;
   }
   Text(stra[index].c_str());
   Generate();
}

eOSState
cYaepgInputStra::ProcessKey(eKeys key)
{
   eOSState state = osContinue;

   switch (key & ~k_Repeat) {
   case kLeft:
      UpdateIndex(index - 1);
      break;
   case kRight:
      UpdateIndex(index + 1);
      break;
   default:
      state = osUnknown;
      break;
   }

   return state;
}

/*
 *****************************************************************************
 * cYaepgRecDlg
 *****************************************************************************
 */
class cYaepgRecDlg {
private:
   const cEvent *event;
   tGeom geom;
   cYaepgTextBox titleBox;
   cYaepgTextBox timeBox;
   cYaepgTextBox startBox;
   cYaepgTextBox endBox;
   cYaepgInputTime startInput;
   cYaepgInputTime endInput;
   int curY;

public:
   cYaepgRecDlg(void);
   void UpdateEvent(const cEvent *_event);
   eOSState ProcessKey(eKeys key);
   void Draw(cBitmap *bmp);
   bool AddTimer(void);
};

cYaepgRecDlg::cYaepgRecDlg(void) :
   event(NULL),
   curY(0)
{
   geom = REC_DLG_GEOM;

   /*
    * The record dialog box widget locations are relative to the record dialog
    * box location.
    */
   titleBox.Text("");
   titleBox.Font(REC_DLG_FONT);
   titleBox.FgColor(REC_DLG_COLOR);
   titleBox.BgColor(clrTransparent);
   titleBox.Flags((eTextFlags)(TBOX_VALIGN_CENTER | TBOX_HALIGN_CENTER));
   titleBox.X(REC_DLG_GEOM.x + REC_TITLE_GEOM.x);
   titleBox.Y(REC_DLG_GEOM.y + REC_TITLE_GEOM.y);
   titleBox.W(REC_TITLE_GEOM.w);
   titleBox.H(REC_TITLE_GEOM.h);

   timeBox.Text("");
   timeBox.Font(REC_DLG_FONT);
   timeBox.FgColor(REC_DLG_COLOR);
   timeBox.BgColor(clrTransparent);
   timeBox.Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
   timeBox.X(REC_DLG_GEOM.x + REC_TIME_GEOM.x);
   timeBox.Y(REC_DLG_GEOM.y + REC_TIME_GEOM.y);
   timeBox.W(REC_TIME_GEOM.w);
   timeBox.H(REC_TIME_GEOM.h);

   startBox.Text(tr("Start"));
   startBox.Font(REC_DLG_FONT);
   startBox.FgColor(REC_DLG_COLOR);
   startBox.BgColor(clrTransparent);
   startBox.Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
   startBox.X(REC_DLG_GEOM.x + REC_START_GEOM.x);
   startBox.Y(REC_DLG_GEOM.y + REC_START_GEOM.y);
   startBox.W(REC_START_GEOM.w);
   startBox.H(REC_START_GEOM.h);

   endBox.Text(tr("Stop"));
   endBox.Font(REC_DLG_FONT);
   endBox.FgColor(REC_DLG_COLOR);
   endBox.BgColor(clrTransparent);
   endBox.Flags((eTextFlags)(TBOX_VALIGN_LEFT | TBOX_HALIGN_CENTER));
   endBox.X(REC_DLG_GEOM.x + REC_END_GEOM.x);
   endBox.Y(REC_DLG_GEOM.y + REC_END_GEOM.y);
   endBox.W(REC_END_GEOM.w);
   endBox.H(REC_END_GEOM.h);

   startInput.Text("");
   startInput.Font(REC_DLG_FONT);
   startInput.FgColor(REC_DLG_COLOR);
   startInput.BgColor(clrTransparent);
   startInput.Flags((eTextFlags)(TBOX_VALIGN_RIGHT | TBOX_HALIGN_CENTER));
   startInput.X(REC_DLG_GEOM.x + REC_STINP_GEOM.x);
   startInput.Y(REC_DLG_GEOM.y + REC_STINP_GEOM.y);
   startInput.W(REC_STINP_GEOM.w);
   startInput.H(REC_STINP_GEOM.h);

   endInput.Text("");
   endInput.Font(REC_DLG_FONT);
   endInput.FgColor(REC_DLG_COLOR);
   endInput.BgColor(clrTransparent);
   endInput.Flags((eTextFlags)(TBOX_VALIGN_RIGHT | TBOX_HALIGN_CENTER));
   endInput.X(REC_DLG_GEOM.x + REC_ENINP_GEOM.x);
   endInput.Y(REC_DLG_GEOM.y + REC_ENINP_GEOM.y);
   endInput.W(REC_ENINP_GEOM.w);
   endInput.H(REC_ENINP_GEOM.h);

}

bool
cYaepgRecDlg::AddTimer(void)
{
    cTimer *recTimer;
    char dayStr[8], file[MaxFileName], eventStr[256];
    int flags, channel, start, stop, priority, lifetime;

    /* Create the timer */
    recTimer = new cTimer;

    /* Construct the string that represent the event */
    flags = tfActive;
    channel = Channels.GetByChannelID(event->ChannelID(), true)->Number();
    snprintf(dayStr, 8, "%d", startInput.recTime.tm_mday);
    start = (startInput.recTime.tm_hour * 100) + startInput.recTime.tm_min;
    stop = (endInput.recTime.tm_hour * 100) + endInput.recTime.tm_min;
    priority = Setup.DefaultPriority;
    lifetime = Setup.DefaultLifetime; 
    *file = '\0';
    if (!isempty(event->Title())) {
         strncpy(file, event->Title(), sizeof(file));
    }
    snprintf(eventStr, 256, "%d:%d:%s:%04d:%04d:%d:%d:%s:",
             flags, channel, dayStr, start, stop,
	     priority, lifetime, file);
    if (recTimer->Parse(eventStr) == false) {
      delete recTimer;
      return false;
    }
    if (iRemoteTimer && pRemoteTimers) {
       RemoteTimers_Timer_v1_0 rt;
       rt.timer = recTimer;
       if (!pRemoteTimers->Service("RemoteTimers::NewTimer-v1.0", &rt)) {
          YAEPG_ERROR(*rt.errorMsg);
          return false;
       }
    }
    else {
      Timers.Add(recTimer);
      Timers.Save();
    }    
    return true;
}


eOSState
cYaepgRecDlg::ProcessKey(eKeys key)
{
   eOSState state = osUnknown;

   /* First let the input boxes process the key */
   switch (curY) {
   case 0:
      state = startInput.ProcessKey(key);
      break;
   case 1:
      state = endInput.ProcessKey(key);
      break;
   default:
      ASSERT(0);
      break;
   }

   /* Key was not consumed by one of the input boxes */
   if (state == osUnknown) {
      switch (key & ~k_Repeat) {
      case kUp:
         if (curY > 0) {
           curY--;
           state = osContinue;
       	 }
         break;
      case kDown:
         if (curY < 1) {
           curY++;
           state = osContinue;
      	 }
         break;
      default:
         state = osUnknown;
         break;
      }
   }

   return state;
}

void
cYaepgRecDlg::UpdateEvent(const cEvent *_event)
{
   event = _event;

   /* Update the event title */
   titleBox.Text(event->Title());
   titleBox.Generate();
   
   startBox.Generate();
   endBox.Generate();

   /* Update the start/end time */
   struct tm locStart, locEnd;
   time_t t;
   char timeStr[32];

   t = event->StartTime();
   localtime_r(&t, &locStart);
   t += event->Duration();
   localtime_r(&t, &locEnd);
   if (iTimeFormat == TIME_FORMAT_24H) {
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d - %02d:%02d",
               locStart.tm_hour, locStart.tm_min,
               locEnd.tm_hour, locEnd.tm_min);
   } else {
      snprintf(timeStr, sizeof(timeStr), "%d:%02d%s - %d:%02d%s",
               FMT_12HR(locStart.tm_hour), locStart.tm_min,
               FMT_AMPM(locStart.tm_hour),
               FMT_12HR(locEnd.tm_hour), locEnd.tm_min,
               FMT_AMPM(locEnd.tm_hour));
   }
   timeBox.Text(timeStr);
   timeBox.Generate();

   /* Fill in initial values for start/end input */
   startInput.UpdateTime(event->StartTime() - (Setup.MarginStart * 60));
   endInput.UpdateTime(event->StartTime() + _event->Duration() + (Setup.MarginStart * 60));

}

void
cYaepgRecDlg::Draw(cBitmap *bmp)
{
   bmp->DrawBitmap(geom.x, geom.y, *REC_DLG_IMG);
   titleBox.Draw(bmp);
   timeBox.Draw(bmp);
   startBox.Draw(bmp);
   endBox.Draw(bmp);
   
   switch (curY) {
   case 0:
      endInput.BgColor(clrTransparent);
      startInput.BgColor(GRID_SEL_BG);
      break;
   case 1:
      startInput.BgColor(clrTransparent);
      endInput.BgColor(GRID_SEL_BG);
      break;
   }

   startInput.Draw(bmp);
   endInput.Draw(bmp);
}

/*
 *****************************************************************************
 * cYaepgMsg
 *****************************************************************************
 */
class cYaepgMsg {
private:
   cYaepgTextBox msgBox;
   tGeom geom;

public:
   cYaepgMsg(void);
   void UpdateMsg(const char *msg);
   void Draw(cBitmap *bmp);
};

cYaepgMsg::cYaepgMsg(void)
{
   geom = MSG_BOX_GEOM;
   msgBox.Text("");
   msgBox.Font(MSG_BOX_FONT);
   msgBox.FgColor(MSG_BOX_COLOR);
   msgBox.BgColor(clrTransparent);
   msgBox.Flags((eTextFlags)(TBOX_VALIGN_CENTER | TBOX_HALIGN_CENTER));
   msgBox.X(geom.x);
   msgBox.Y(geom.y);
   msgBox.W(MSG_BG_IMG->Width());
   msgBox.H(MSG_BG_IMG->Height());
}

void
cYaepgMsg::UpdateMsg(const char *msg)
{
   msgBox.Text(msg);
   msgBox.Generate();
}

void
cYaepgMsg::Draw(cBitmap *bmp)
{
   bmp->DrawBitmap(geom.x, geom.y, *MSG_BG_IMG);
   msgBox.Draw(bmp);
}

#ifdef YAEPGHD_REEL_EHD
/*
 *****************************************************************************
 * cReelVidWin
 *
 * For the Reel eHD card I use a telnet connectin to issue fbset comands on
 * the card to adjust the video plane.  This is quite buggy, the eHD card
 * doesn't seem to like changing channels while the video plane is scaled and
 * many resolutions for HD streams end up with a distoted image.
 *
 *****************************************************************************
 */
#define REEL_CURL_CMD            "fbset -mmp %d %d -rmp %d %d\n"
#define VIDPLANE_HORI            1920
#define VIDPLANE_VERT            1080

class cReelVidWin : private cThread {
private:
   FILE *readFp, *writeFp;

   static size_t CurlWrite(void *ptr, size_t size, size_t nmemb, void *stream);
   void SendCmd(const char *cmd);

protected:
   virtual void Action(void);

public:
   cReelVidWin(void);
   ~cReelVidWin();
   bool Init(void);
   void Update(int x, int y, int w, int h);
   void Open(tGeom geom);
   void Close(void);
};

cReelVidWin::cReelVidWin(void) :
   readFp(NULL),
   writeFp(NULL)
{
}

cReelVidWin::~cReelVidWin()
{
   Close();
   SendCmd("exit\n");
   if (Running())
      Cancel(3);
   (void) close(fileno(readFp));
   (void) close(fileno(writeFp));
   (void) fclose(readFp);
   (void) fclose(writeFp);
}

size_t
cReelVidWin::CurlWrite(void *ptr, size_t size, size_t nmemb, void *stream)
{
   return (size * nmemb);
}


void cReelVidWin::Action(void)
{
   CURL *hdl;
   CURLcode status;

   hdl = curl_easy_init();
   if (hdl == NULL) {
      YAEPG_ERROR("curl_easy_init");
      return;
   }

   status = curl_easy_setopt(hdl, CURLOPT_URL, "telnet://192.168.99.129/");
   if (status != CURLE_OK) {
      YAEPG_ERROR("curl_easy_setopt");
      return;
   }

   /* Set the read FILE * to the read end of the pipe */
   status = curl_easy_setopt(hdl, CURLOPT_READDATA, readFp);
   if (status != CURLE_OK) {
      YAEPG_ERROR("curl_easy_setopt");
      return;
   }

   /* Setup our own write function so we can discard output */
   status = curl_easy_setopt(hdl, CURLOPT_WRITEFUNCTION,
                             (curl_write_callback)&CurlWrite);
   if (status != CURLE_OK) {
      YAEPG_ERROR("curl_easy_setopt");
      return;
   }

   /* Connect to eHD card, this call blocks until the connection is closed */
   status = curl_easy_perform(hdl);
   if (status != CURLE_OK) {
      YAEPG_ERROR("curl_easy_perform");
      return;
   }

   YAEPG_INFO("Thread exit!");
   return;
}

void
cReelVidWin::SendCmd(const char *cmd)
{
   char errStr[128];

   YAEPG_INFO("curl cmd '%s'", cmd);
   if (fwrite(cmd, 1, strlen(cmd), writeFp) != strlen(cmd)) {
      snprintf(errStr, sizeof(errStr), "fwrite: %s", strerror(errno));
      YAEPG_ERROR(errStr);
   }
}

bool
cReelVidWin::Init(void)
{
   int pfds[2];
   char errStr[128];

   /* Init the curl library */
   curl_global_init(CURL_GLOBAL_ALL);

   /* Create a pipe to communciate with the telnet connection */
   if (pipe(pfds) == -1) {
      snprintf(errStr, sizeof(errStr), "pipe: %s", strerror(errno));
      YAEPG_ERROR(errStr);
      return false;
   }
   readFp = fdopen(pfds[0], "r");
   if (readFp == NULL) {
      snprintf(errStr, sizeof(errStr), "fdopen: %s", strerror(errno));
      YAEPG_ERROR(errStr);
      return false;
   }
   writeFp = fdopen(pfds[1], "w");
   if (writeFp == NULL) {
      snprintf(errStr, sizeof(errStr), "fdopen: %s", strerror(errno));
      YAEPG_ERROR(errStr);
      return false;
   }

   /* Make the write side of the pipe unbufferd */
   if (setvbuf(writeFp, NULL, _IONBF, 0) == -1) {
      snprintf(errStr, sizeof(errStr), "setvbuf: %s", strerror(errno));
      YAEPG_ERROR(errStr);
      return false;
   }

   /* Start a thread to handle the telnet connection */
   Start();

   return true;
}

void
cReelVidWin::Open(tGeom geom)
{
   tGeom scaled;
   char cmd[128];

   /*
    * For the Reel eHD card we set up the video window by executing an "fbset"
    * command on the eHD card.  The video window geometry is specified using
    * coordinates of the OSD plane but the fbset command works on the video
    * plane.  We need to map the geometry from the OSD plane to the video plane.
    */
   scaled.x = ROUND((float)VID_WIN_GEOM.x * ((float)VIDPLANE_HORI / 720.0f));
   scaled.y = ROUND((float)VID_WIN_GEOM.y * ((float)VIDPLANE_VERT / 576.0f));
   scaled.w = ROUND((float)VID_WIN_GEOM.w * ((float)VIDPLANE_HORI / 720.0f));
   scaled.h = ROUND((float)VID_WIN_GEOM.h * ((float)VIDPLANE_VERT / 576.0f));

   /*
    * For some read if we set the video plane width to < 577 while in 1080i mode
    * we end up with a blue screen.  To prevent this adjust the width to be >=
    * 577 even though it may not be what the user wants.  Not sure if this holds
    * true while video is running in 720p.
    */
   scaled.w = MAX(scaled.w, 577);
   scaled.h = MAX(scaled.h, 325);

   snprintf(cmd, sizeof(cmd), REEL_CURL_CMD,
            scaled.x, scaled.y, scaled.w, scaled.h);
   SendCmd(cmd);
}

void
cReelVidWin::Close(void)
{
   char cmd[128];
   snprintf(cmd, sizeof(cmd), REEL_CURL_CMD,
            0, 0, VIDPLANE_HORI, VIDPLANE_VERT);
   YAEPG_INFO("closing video window");
   SendCmd(cmd);
}

static cReelVidWin *reelVidWin = NULL;
#endif

/*
 *****************************************************************************
 * cYaepgHd
 *****************************************************************************
 */
class cYaepghd : public cOsdObject {
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
   cYaepghd(void);
   ~cYaepghd();
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

cYaepghd::cYaepghd(void) :
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

cYaepghd::~cYaepghd()
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
cYaepghd::Show(void)
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
cYaepghd::AddDelTimer(void)
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
cYaepghd::AddDelSwitchTimer()
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
cYaepghd::AddDelRemoteTimer()
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
cYaepghd::ProcessKey(eKeys key)
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
         state = osEnd;
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
         else{
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
cYaepghd::UpdateChans(cChannel *c)
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
cYaepghd::UpdateChans(int change)
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
cYaepghd::SetTime(time_t newTime)
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
cYaepghd::UpdateTime(int change)
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
cYaepghd::UpdateEvent(const cEvent *newEvent)
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
cYaepghd::SwitchToCurrentChannel(bool closeVidWin)
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
cYaepghd::MoveCursor(eCursorDir dir)
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
cYaepghd::Draw(void)
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

/*
 *****************************************************************************
 * cMenuSetupYaepg
 *****************************************************************************
 */
class cMenuSetupYaepg : public cMenuSetupPage {
private:
   int iNewHideMenuEntry;
   char sNewMainMenuEntry[MaxFileName];
#if defined(MAINMENUHOOKSVERSION) 
#if MAINMENUHOOKSVERSNUM >= 10001
   int iNewReplaceOrgSchedule;
#endif
#endif
   int iNewChannelChange;
   int iNewSwitchOK;
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
   
protected:
   virtual void Store(void);

public:
   cMenuSetupYaepg(void);
   ~cMenuSetupYaepg();
};


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
   SetupStore("RecDlgRed",           iRecDlgRed);
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

   Add(new cMenuEditBoolItem (tr("Hide mainmenu entry"), &iNewHideMenuEntry));
   Add(new cMenuEditStrItem(tr("Main menu entry"), sNewMainMenuEntry, sizeof(sNewMainMenuEntry),trVDR(FileNameChars)));
   #if defined(MAINMENUHOOKSVERSION) 
   #if MAINMENUHOOKSVERSNUM >= 10001
   Add(new cMenuEditBoolItem (tr("Replace original schedule"), &iNewReplaceOrgSchedule));
   #endif
   #endif
   Add(new cMenuEditStraItem (tr("Channel change"), &iNewChannelChange, CHANNEL_CHANGE_COUNT, CH_CHANGE_MODES));
   Add(new cMenuEditBoolItem (tr("Switch channel with OK"), &iNewSwitchOK));
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

/*
 *****************************************************************************
 * cPluginYaepghd
 *****************************************************************************
 */
static const char *VERSION        = "0.0.4_pre20130317";
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
   Icons::InitCharSet();
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
   return new cYaepghd;
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
   char themeName[MaxThemeName];

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
