/*
 * yaepghd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Community Edition
 */

#pragma once

#include <Magick++.h>
#include <vector>

#include <vdr/osd.h>
#include <vdr/osdbase.h>
#include <vdr/timers.h>

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

#define FMT_AMPM(_hr)                  ((_hr) >= 12 ? "p" : "a")
#define FMT_12HR(_hr)                  ((_hr) % 12 == 0 ? 12 : (_hr) % 12)

using namespace Magick;

struct tGeom {
   int x;
   int y;
   int w;
   int h;
};


/*
 *****************************************************************************
 * cFontSymbols
 *****************************************************************************
 */
class cFontSymbols
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
#endif


