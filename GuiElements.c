/*
 * yaepghd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * Community Edition
 */

#include "GuiElements.h"

#include "Utils.h"
#include "ServiceStructs.h"
#include "MenuSetupYaepg.h"

#include <locale.h>
#include <langinfo.h>

#include <vdr/plugin.h>

/*
 *****************************************************************************
 * cFontSymbols
 *****************************************************************************
 */



bool cFontSymbols::IsUTF8=false;

void cFontSymbols::InitCharSet()
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
 * cYaepgTextBox
 *****************************************************************************
 */


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


/*
 *****************************************************************************
 * cYaepgGridChans
 *****************************************************************************
 */

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
            t=ti->Recording()?cFontSymbols::Recording():cFontSymbols::Watch();
         else
            t=ti->Recording()?"R":"T";
         break;
      case tmPartial:
         if (iInfoSymbols && iVDRSymbols)
            t=ti->Recording()?cFontSymbols::Recording():cFontSymbols::WatchUpperHalf();
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
            t=(iInfoSymbols && iVDRSymbols)?cFontSymbols::ArrowCCW():"S" ;
         }
         delete serviceData;
      }
   }
   if (event->Vps() && (event->Vps() - event->StartTime()))
      v=(iInfoSymbols && iVDRSymbols)?cFontSymbols::VPS():"V";
   else
      v=" ";

   if (event->SeenWithin(30) && event->IsRunning())
      r=(iInfoSymbols && iVDRSymbols)?cFontSymbols::Running():"*";
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
    char dayStr[8], file[NAME_MAX], eventStr[256];
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
        flags, channel, dayStr, start, stop, priority, lifetime, file);
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
   endInput.UpdateTime(event->StartTime() + _event->Duration() + (Setup.MarginStop * 60));
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


