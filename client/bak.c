#include <curses.h>
#include <menu.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <locale.h>
#include <unistd.h>
#include <term.h>
#include <string.h>
#include <wchar.h>
#include <panel.h>

char *options[] = {"Red",   "Blue",   "Green",  "Orange", "Brown", "White",
                   "Black", "Yellow", "Violet", "Purple", "Aqua",  "Slate"};
MENU *menu;
ITEM **items;

int32_t __inline__ __attribute((pure)) min(int32_t o1, int32_t o2) {
  return o1 < o2 ? o1 : o2;
}
int32_t __inline__ __attribute((pure)) max(int32_t o1, int32_t o2) {
  return o1 > o2 ? o1 : o2;
}

void help() {
  fprintf(stderr, "./kms <display text>\n");
  exit(1);
}

PANEL *tp, *sp;
WINDOW *tw, *sw;
int32_t wx, wy;
wchar_t *__restrict cstr;
uint32_t cstrl;
uint8_t sel = 0;
int8_t sell = 0;

void del_win(WINDOW *w) {
  wborder(w, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(w);
  delwin(w);
}

int32_t cpp = 0;
int32_t cp = 2;
int32_t textl;

void setupWindows() {
  erase();
  getmaxyx(curscr, wy, wx);

  textl = (cstrl * 2) / (wx - 4);
  tw = newwin(textl + 3, wx - 2, 0, 1);
  box(tw, 0, 0);
  
  {
    uint32_t coff = 0;
    uint32_t yoff = 0;
    while (coff < cstrl) {
      mvwaddnwstr(tw, 1 + yoff, 1, cstr + coff, min((wx - 4) / 2, cstrl - coff));
      coff += (wx - 4) / 2;
      ++yoff;
    }
  }
  tp = new_panel(tw);

  sw = newwin(4, wx - 2, textl + 3, 1);
  box(sw, 0, 0);
  sp = new_panel(sw);

  update_panels();

  doupdate();
}

void update() {
  {
    int32_t i;
    for(i = 1; i <= textl + 1; ++i) {
      mvwchgat(tw, i, 1, wx - 4, A_NORMAL, 0, NULL);
    }
  }
  cpp = min(max(cpp, 0), cstrl - 1);
  if (sell + cpp > cstrl) {
    sell = cstrl - cpp;
  }
  uint32_t cy = cpp / ((wx - 4) / 2);
  uint32_t cx = cpp - (cy * ((wx - 4) / 2));
    
  int32_t crem = ((wx - 4) / 2) - cx;
  uint8_t upp = 1;
  int32_t csell = sell;
  int32_t y = 0;
  if (sel) {
    if (csell >= crem) {
      mvwchgat(tw, upp + cy, cx * 2 + 1, crem * 2, A_REVERSE, 0, NULL);
      y |= 1;
      ++upp;
      csell -= crem;
    }
    while (csell > 0) {
      if (upp == 1) {
        mvwchgat(tw, upp + cy, cx * 2 + 1, min(csell * 2, wx - 4), A_REVERSE, 0, NULL);
        y |= 2;
      } else {
        mvwchgat(tw, upp + cy, 1, min(csell * 2, wx - 5), A_REVERSE, 0, NULL);
        y |= 4;
      }
      ++upp;
      csell -= (wx - 4) / 2;
    }
    //mvwchgat(tw, 1 + cy, cx * 2 + 1, sell, A_REVERSE, 0, NULL);
  } else {
    mvwchgat(tw, 1 + cy, cx * 2 + 1, 1, A_REVERSE, 0, NULL);
  }

  char a[100] = "";
  snprintf(a, sizeof(a), "Cp: %i; Wx: %i; Cpp: %i; Sel: %i; Sell: %i; Crem: %i; Cx: %u; Cy: %u; Cstrl: %i    ", cp, wx - 4, cpp, sel, sell, crem, cx, cy, cstrl);
  mvwaddstr(sw, 1, 1, a);
  
  update_panels();
  doupdate();
}

uint32_t runel(char *__restrict str) {
  char *__restrict os = str;
  for (++str; (*str & 0xc0) == 0x80; ++str);
  return (uint32_t) (str - os);
}

uint32_t utf8_to_unicode(char *__restrict str, uint32_t l) {
  uint32_t res = 0;
  switch (l) {
    case 4:
      res |= *str & 0x7;
      break;
    case 3:
      res |= *str & 0xF;
      break;
    case 2:
      res |= *str & 0x1F;
      break;
    case 1:
      res |= *str & 0x7F;
      break;
  }

  --l;
  while (l) {
    ++str;
    res <<= 6;
    res |= *str & 0x3F;
    --l;
  }

  return res;
}

void utf2wch(char *__restrict s, wchar_t *__restrict t, uint32_t *__restrict tl) {
  *tl = 0;
  uint32_t cl;
  while (*s) {
    cl = runel(s);
    *t = utf8_to_unicode(s, cl);
    ++*tl;
    ++t;
    s += cl;
  }
  *t = L'\0';
  cstr = realloc(cstr, *tl * sizeof(cstr[0]));
}

int main(int argc, char **argv) {
  /*if (argc < 2) {
    help();
  }*/
  char a[] = "マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠";
  cstr = malloc(sizeof(cstr[0]) * strlen(a));
  utf2wch(a, cstr, &cstrl);

  setlocale(LC_ALL, "");

  initscr();
  curs_set(0);
  cbreak();
  nodelay(curscr, 1);
  noecho();
  clear();

  getmaxyx(curscr, wy, wx);
  setupWindows();
  update();

  char ch;
  while ((ch = wgetch(sw)) != 27) {
    switch(ch) {
      case 'h':
        if (sel) {
          --sell;
        } else {
          --cpp;
        }
        break;
      case 'l':
        if (sel) {
          ++sell;
        } else {
          ++cpp;
        }
        break;
      case 'j':
        if (sel) {
          sell += (wx - 4) / 2;
        } else {
          cpp += (wx - 4) / 2;
        }
        break;
      case 'k':
        if (sel) {
          sell -= (wx - 4) / 2;
        } else {
          cpp -= (wx - 4) / 2;
        }
        break;
      case 'v':
        sel ^= 1;
        sell = sel;
        break;
    }
    cpp = min(max(cpp, 0), cstrl);
    update();
    doupdate();
  }
  endwin();


  return 0;

  /*
  items = (ITEM **)calloc(12, sizeof(ITEM *));
  {
    int i;
    for (i = 0; i < 12; i++) {
      items[i] = new_item(options[i], NULL);
    }
  }

  char ch;
  int i;
  set_menu_win(menu, w);
  set_menu_sub(menu, derwin(w, 3, 20, 1, 1));
  set_menu_mark(menu, " "); string used as menu marker 
  set menu format - no of items to be displayed 
  set_menu_format(menu, 3, 2);
  post the menu 
  post_menu(menu);
  wrefresh(w);
  while ((ch = wgetch(w)) != 27) 27 is the key code for ESC 
    switch (ch) {
      case 'j':
        menu_driver(menu, REQ_DOWN_ITEM);
        break;
      case 'k':
        menu_driver(menu, REQ_UP_ITEM);
        break;
    }
    wrefresh(w);
  }
  unpost_menu(menu);
  free_menu(menu);
  for (i = 0; i < 12; i++) {
    free_item(items[i]);
  }*/
  endwin();
  return 0;
}
