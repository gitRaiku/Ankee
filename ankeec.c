#include <curses.h>
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
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>


int32_t __inline__ __attribute((pure)) min(int32_t o1, int32_t o2) {
  return o1 < o2 ? o1 : o2;
}

int32_t __inline__ __attribute((pure)) max(int32_t o1, int32_t o2) {
  return o1 > o2 ? o1 : o2;
}

void help() {
  fprintf(stderr, "./kms <display text> <path/to/audio/file>\n");
  exit(1);
}

struct ws {
  wchar_t *__restrict s;
  uint32_t l;
};

struct mean {
  uint32_t tl;
  struct ws *__restrict t;
  uint32_t tls;

  uint32_t pl;
  struct ws *__restrict p;
  uint32_t pls;
};

struct resp {
  uint32_t tl;
  struct ws *__restrict t;
  uint32_t rl;
  struct ws *__restrict r;
  uint32_t ml;
  struct mean *__restrict m;
};

uint32_t respsl;
int32_t cs;
struct resp *__restrict resps;

struct wp {
  WINDOW *w;
  PANEL *p;
};

struct entr {
  struct wp t;
  struct wp r;
  struct wp m;
};
struct entr *__restrict entrs;

char *apath;
PANEL *tp, *sp, *rp;
WINDOW *tw, *sw, *rw;
int32_t wx, wy;
wchar_t *__restrict cstr;
uint32_t cstrl;
uint8_t sel = 0;
int8_t sell = 0;
int32_t cpp = 0;
int32_t cp = 2;
int32_t textl;
uint32_t cws = 0;
uint32_t *__restrict cus[3];
uint32_t cu;
uint32_t cwss = 0;
uint8_t EXIT = 0;
uint8_t COPY = 0;
uint32_t tsp = 0;

void del_win(WINDOW *w) {
  wborder(w, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(w);
  delwin(w);
}

void chg(int cp, int cl, int mod) {
  uint32_t cy = cp / ((wx - 4) / 2);
  uint32_t cx = cp - (cy * ((wx - 4) / 2));
  int32_t crem = ((wx - 4) / 2) - cx;
  uint8_t upp = 1;
  int32_t csell = cl;
  if (csell >= crem) {
    mvwchgat(tw, upp + cy, cx * 2 + 1, crem * 2, mod, 0, NULL);
    ++upp;
    csell -= crem;
  }
  while (csell > 0) {
    if (upp == 1) {
      mvwchgat(tw, upp + cy, cx * 2 + 1, min(csell * 2, wx - 4), mod, 0, NULL);
    } else {
      mvwchgat(tw, upp + cy, 1, min(csell * 2, wx - 5), mod, 0, NULL);
    }
    ++upp;
    csell -= (wx - 4) / 2;
  }
}

struct selem {
  struct ws s;
  uint32_t sp;
  uint32_t sl;
};
struct selem *__restrict selems;
uint32_t selemsl;

void clear_resp_win(uint32_t cw) {
  int32_t i;
  if (resps[cw].tl) {
    for(i = 0; i < resps[cw].tl; ++i) {
      mvwchgat(entrs[cw].t.w, 1 + i, 1, resps[cw].t[i].l * 2, A_NORMAL, 0, NULL);
    }
  }
  for(i = 0; i < resps[cw].rl; ++i) {
    mvwchgat(entrs[cw].r.w, 1 + i, 1, resps[cw].r[i].l * 2, A_NORMAL, 0, NULL);
  }
  for(i = 0; i < resps[cw].ml; ++i) {
    mvwchgat(entrs[cw].m.w, 1 + i, 1, resps[cw].m[i].tls + resps[cw].m[i].pls + 3, A_NORMAL, 0, NULL);
  }
}

void sel_resp_win(uint32_t cw) {
  clear_resp_win(cw);

  int32_t i;
  if (resps[cw].tl) {
    for(i = 0; i < resps[cw].tl; ++i) {
      if (cus[0][i] || cwss == 0) {
        mvwchgat(entrs[cw].t.w, 1 + i, 1, resps[cw].t[i].l   * 2, (cus[0][i] * A_REVERSE) | ((cwss == 0 && cu == i) * (A_BOLD | A_UNDERLINE)), 0, NULL);
      }
    }
  }
  
  for(i = 0; i < resps[cw].rl; ++i) {
    if (cus[1][i] || cwss == 1) {
      mvwchgat(entrs[cw].r.w, 1 + i, 1, resps[cw].r[i].l   * 2, (cus[1][i] * A_REVERSE) | ((cwss == 1 && cu == i) * (A_BOLD | A_UNDERLINE)), 0, NULL);
    }
  }

  for(i = 0; i < resps[cw].ml; ++i) {
    if (cus[2][i] || cwss == 2) {
      mvwchgat(entrs[cw].m.w, 1 + i, 1, resps[cw].m[i].tls + resps[cw].m[i].pls + 3, (cus[2][i] * A_REVERSE) | ((cwss == 2 && cu == i) * (A_BOLD | A_UNDERLINE)), 0, NULL);
    }
  }
}

void update() {
  chg(0, cstrl, A_NORMAL);

  if (cws == 0) {
    cpp = min(max(cpp, 0), cstrl - 1);
    if (sell + cpp > cstrl - tsp) {
      sell = cstrl - cpp - tsp;
    }
    sell = max(sell, 1);
      
    if (sel) {
      chg(cpp, sell, A_REVERSE | A_UNDERLINE);
    } else {
      chg(cpp, 1, A_REVERSE);
    }
    if (resps) {
      clear_resp_win(0);
    }
  } else {
    if (resps) {
      cws = min(max(cws, 0), respsl);
      cu = min(max(cu, 0), (cwss == 0 ? (resps[cws - 1].tl ? resps[cws - 1].tl : 99) : (cwss == 1 ? resps[cws - 1].rl : resps[cws - 1].ml)) - 1);
      cwss = min(max(cwss, resps[cws - 1].tl ? 0 : 1), 2);
      if (cws > 1) {
        clear_resp_win(cws - 2);
      }

      sel_resp_win(cws - 1);
      if (cws < respsl) {
        clear_resp_win(cws - 0);
      }
    }
  }

  /*
  char a[100] = "";
  snprintf(a, sizeof(a), "tsp: %u          ", tsp);
  mvwaddstr(sw, wy - textl - 11, 1, a);*/
  
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

uint8_t iswide(wchar_t c) { // TODO: FIX
  if (c == L'…' || c == L'”' || c == L'“') {
    return 0;
  }
  return 1;
}

uint32_t mkwide(wchar_t *__restrict s, wchar_t c) {
  if (c < 255) {
    if (c == ' ') {
      c = 0x3000;
    } else {
      c += 0xFEE0;
    }
  }
  s[0] = c;
  if (!iswide(c)) {
    ++tsp;
    s[1] = L' ';
    return 2;
  }
  return 1;
}

void utf2wwch(char *__restrict s, wchar_t *__restrict t, uint32_t *__restrict tl) {
  *tl = 0;
  uint32_t cl;
  while (*s) {
    cl = runel(s);
    *tl += mkwide(t + *tl, utf8_to_unicode(s, cl));
    s += cl;
  }
  t[*tl] = L'\0';
  t = realloc(t, *tl * sizeof(t[0]));
}

void utf2wch(char *__restrict s, wchar_t *__restrict t, uint32_t *__restrict tl) {
  *tl = 0;
  uint32_t cl;
  while (*s) {
    cl = runel(s);
    t[*tl] = utf8_to_unicode(s, cl);
    ++*tl;
    s += cl;
  }
  t[*tl] = L'\0';
  t = realloc(t, *tl * sizeof(t[0]));
}

uint32_t wchutf8(char *__restrict outs, wchar_t *__restrict s, uint32_t sl) {
  int32_t i;
  uint32_t ol = 0;
  for(i = 0; i < sl; ++i) {
    if (s[i] <= 0x7F) {
      outs[ol    ] =         s[i];
      ++ol;
    } else if (s[i] <= 0x7FF) {
      outs[ol    ] = 0xC0 | (s[i] & 0x7C0) >> 6;
      outs[ol + 1] = 0x80 | (s[i] & 0x3F);
      ol += 2;
    } else if (s[i] <= 0xFFFF) {
      outs[ol    ] = 0xE0 | (s[i] & 0xF000) >> 12;
      outs[ol + 1] = 0x80 | (s[i] & 0xFC0) >> 6;
      outs[ol + 2] = 0x80 | (s[i] & 0x3F);
      ol += 3;
    } else {
      outs[ol    ] = 0xF0 | (s[i] & 0x1C0000) >> 18;
      outs[ol + 1] = 0x80 | (s[i] & 0x3F000) >> 12;
      outs[ol + 2] = 0x80 | (s[i] & 0xFC0) >> 6;
      outs[ol + 3] = 0x80 | (s[i] & 0x3F);
      ol += 4;
    }
  }
  return ol;
}

void clear_resp() {
  if (resps == NULL) {
    return;
  }
  int32_t i, j, k;
  for(i = 0; i < respsl; ++i) {
    for(j = 0; j < resps[i].tl; ++j) {
      free(resps[i].t[j].s);
    }
    free(resps[i].t);

    for(j = 0; j < resps[i].rl; ++j) {
      free(resps[i].r[j].s);
    }
    free(resps[i].r);

    for(j = 0; j < resps[i].ml; ++j) {
      for(k = 0; k < resps[i].m[j].tl; ++k) {
        free(resps[i].m[j].t[k].s);
      }
      free(resps[i].m[j].t);

      for(k = 0; k < resps[i].m[j].pl; ++k) {
        free(resps[i].m[j].p[k].s);
      }
      free(resps[i].m[j].p);
    }
    free(resps[i].m);
  }
  free(resps);
  resps = NULL;
  werase(sw);
  box(sw, 0, 0);
}

void req(wchar_t *__restrict s) {
  //wchar_t s[] = L"人間";//きのこ";
  char ss[1024] = "";
  uint32_t sl = wchutf8(ss + 3, s, wcslen(s));
  ss[0] = (sl & 0xFF00) >> 8;
  ss[1] = (sl & 0x00FF);
  ss[2] = 0;
  /*{
    int32_t i;
    for(i = 0; i < 10; ++i) {
      fprintf(stdout, "%u ", sstr[i]);
    }
  }*/
  send(cs, ss, sl + 3, 0);

#define R(x) \
    recv(cs, ss, 1, MSG_WAITALL); \
    x ##l=ss[0]; \
    x = malloc(x ##l * sizeof(x[0]))

#define S(x) \
    recv(cs, ss, 1, 0); \
    x .l = ss[0]; \
    x .s = malloc((x .l + 1) * sizeof(x .s[0])); \
    recv(cs, ss, x .l, MSG_WAITALL); \
    ss[x .l] = '\0'; \
    utf2wch(ss, x .s, &x .l)
    
/*
#define S(x) \
    recv(cs, ss, 1, 0); \
    sl=ss[0]; \
    x = malloc((sl + 1) * sizeof(x[0])); \
    recv(cs, ss, sl, MSG_WAITALL); \
    ss[sl] = '\0'; \
    utf2wch(ss, x, &tx)
*/

  if (resps) {
    clear_resp();
  }

  ss[0] = ss[1] = ss[2] = ss[3] = 0;
  recv(cs, ss, 2, MSG_WAITALL);
  respsl = ss[0] << 8 | ss[1];
  if (respsl == 0) {
    return;
  }
  resps = malloc(respsl * sizeof(resps[0]));
  int32_t i, j, k;
  uint32_t cum[3] = {0};
  for(i = 0; i < respsl; ++i) {
    R(resps[i].t);
    for(j = 0; j < resps[i].tl; ++j) {
      S(resps[i].t[j]);
    }

    R(resps[i].r);
    for(j = 0; j < resps[i].rl; ++j) {
      S(resps[i].r[j]);
    }

    R(resps[i].m);
    for(j = 0; j < resps[i].ml; ++j) {
      R(resps[i].m[j].t);
      resps[i].m[j].tls = 0;
      for(k = 0; k < resps[i].m[j].tl; ++k) {
        S(resps[i].m[j].t[k]);
        resps[i].m[j].tls += resps[i].m[j].t[k].l;
      }
      resps[i].m[j].tls += (resps[i].m[j].tl - 1) * 2;

      R(resps[i].m[j].p);
      resps[i].m[j].pls = 0;
      for(k = 0; k < resps[i].m[j].pl; ++k) {
        S(resps[i].m[j].p[k]);
        resps[i].m[j].pls += resps[i].m[j].p[k].l;
      }
      resps[i].m[j].pls += (resps[i].m[j].pl - 1) * 2;
    }
    cum[0] = max(cum[0], resps[i].tl);
    cum[1] = max(cum[1], resps[i].rl);
    cum[2] = max(cum[2], resps[i].ml);
  }
  cus[0] = calloc(sizeof(cus[0][0]), cum[0]);
  cus[1] = calloc(sizeof(cus[1][0]), cum[1]);
  cus[2] = calloc(sizeof(cus[2][0]), cum[2]);
}

void create_new_panels() {
  int32_t i, j, k;
  werase(sw);
  box(sw, 0, 0);
  if (resps == NULL) {
    return;
  }
  uint32_t tmw, ch, h;
  entrs = malloc(respsl * sizeof(entrs[0]));
  ch = 0;
  tmw = 0;
  for(k = 0; k < respsl; ++k) {
    h = max(resps[k].tl, max(resps[k].rl, resps[k].ml));
    if (resps[k].tl) {
      tmw = 0;
      for(i = 0; i < resps[k].tl; ++i) {
        tmw = max(tmw, resps[k].t[i].l * 2 + 2);
      }
      entrs[k].t.w = derwin(sw, h + 2, tmw, 1 + ch, 1);
      for(i = 0; i < resps[k].tl; ++i) {
        mvwaddwstr(entrs[k].t.w, 1 + i, 1, resps[k].t[i].s);
      }
      box(entrs[k].t.w, 0, 0);
      entrs[k].t.p = new_panel(entrs[k].t.w);
    }

    uint32_t rmw = 0;
    for(i = 0; i < resps[k].rl; ++i) {
      rmw = max(rmw, resps[k].r[i].l * 2 + 2);
    }
    entrs[k].r.w = derwin(sw, h + 2, rmw, 1 + ch, tmw + 1);
    for(i = 0; i < resps[k].rl; ++i) {
      mvwaddwstr(entrs[k].r.w, 1 + i, 1, resps[k].r[i].s);
    }
    box(entrs[k].r.w, 0, 0);
    entrs[k].r.p = new_panel(entrs[k].r.w);

    entrs[k].m.w = derwin(sw, h + 2, wx - 4 - tmw - rmw, 1 + ch, tmw + rmw + 1);
    uint32_t cd = 0;
    for(i = 0; i < resps[k].ml; ++i) {
      cd = 0;
      for(j = 0; j < resps[k].m[i].tl; ++j) {
        mvwaddwstr(entrs[k].m.w, 1 + i, 1 + cd, resps[k].m[i].t[j].s);
        cd += resps[k].m[i].t[j].l + 2;
        if (j < resps[k].m[i].tl - 1) {
          mvwaddwstr(entrs[k].m.w, 1 + i, cd - 1, L";");
        }
      }

      mvwaddwstr(entrs[k].m.w, 1 + i, cd, L"|");
      ++cd;

      for(j = 0; j < resps[k].m[i].pl; ++j) {
        mvwaddwstr(entrs[k].m.w, 1 + i, 1 + cd, resps[k].m[i].p[j].s);
        cd += resps[k].m[i].p[j].l + 2;
        if (j < resps[k].m[i].pl - 1) {
          mvwaddwstr(entrs[k].m.w, 1 + i, cd - 1, L";");
        }
      }
    }
    box(entrs[k].m.w, 0, 0);
    entrs[k].m.p = new_panel(entrs[k].m.w);
    ch += h + 2;
  }
}

void setup_windows() {
  erase();
  if (tw) {
    werase(tw);
  }
  if (sw) {
    werase(sw);
  }
  if (rw) {
    werase(rw);
  }
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

  sw = newwin(wy - textl - 3 - 6, wx - 2, textl + 3, 1);
  box(sw, 0, 0);
  sp = new_panel(sw);

  rw = newwin(6, wx - 2, textl + 3 + wy - textl - 3 - 6, 1);
  box(rw, 0, 0);
  rp = new_panel(rw);

  if (respsl) {
    create_new_panels();
  }

  update_panels();
  doupdate();
}

uint8_t fin_sel() {
  int32_t i, j;
  uint8_t f = 0;
  uint32_t cl = 0;

  if (resps[cws - 1].tl) {
    for(i = 0; i < resps[cws - 1].tl; ++i) { if (cus[0][i]) { f = 1; break; } }
    if (f == 0) { return 0; }
    f = 0;
  }
  for(i = 0; i < resps[cws - 1].rl; ++i) { if (cus[1][i]) { f = 1; break; } }
  if (f == 0) { return 0; }
  f = 0;
  for(i = 0; i < resps[cws - 1].ml; ++i) { if (cus[2][i]) { f = 1; break; } }
  if (f == 0) { return 0; }
  f = 0;
  selems[selemsl].s.s = calloc(1, sizeof(selems[selemsl].s.s[0]) * 1024);
  if (resps[cws - 1].tl) {
    for(i = 0; i < resps[cws - 1].tl; ++i) {
      if (cus[0][i]) {
        if (f != 0) {
          swprintf(selems[selemsl].s.s + cl, 2, L"/");
          ++cl;
        }
        swprintf(selems[selemsl].s.s + cl, resps[cws - 1].t[i].l + 1, L"%ls", resps[cws - 1].t[i].s);
        cl += resps[cws - 1].t[i].l;
        f = 1;
      }
    }
    swprintf(selems[selemsl].s.s + cl, 2, L"[");
    ++cl;
  }
  f = 0;
  for(i = 0; i < resps[cws - 1].rl; ++i) {
    if (cus[1][i]) {
      if (f != 0) {
        swprintf(selems[selemsl].s.s + cl, 2, L"/");
        ++cl;
      }
      swprintf(selems[selemsl].s.s + cl, resps[cws - 1].r[i].l + 1, L"%ls", resps[cws - 1].r[i].s);
      cl += resps[cws - 1].r[i].l;
      f = 1;
    }
  }
  if (resps[cws - 1].tl) {
    swprintf(selems[selemsl].s.s + cl, 4, L"]: ");
    cl += 3;
  } else {
    swprintf(selems[selemsl].s.s + cl, 4, L": ");
    cl += 2;
  }
  f = 0;
  for(i = 0; i < resps[cws - 1].ml; ++i) {
    if (cus[2][i]) {
      for(j = 0; j < resps[cws - 1].m[i].tl; ++j) {
        if (f != 0) {
          swprintf(selems[selemsl].s.s + cl, 3, L"; ");
          cl += 2;
        }
        swprintf(selems[selemsl].s.s + cl, resps[cws - 1].m[i].t[j].l + 1, L"%ls", resps[cws - 1].m[i].t[j].s);
        cl += resps[cws - 1].m[i].t[j].l;
        f = 1;
      }
    }
  }
  selems[selemsl].s.l = cl;
  selems[selemsl].s.s = realloc(selems[selemsl].s.s, (selems[selemsl].s.l + 1) * sizeof(selems[selemsl].s.s[0]));
  ++selemsl;

  werase(rw);
  box(rw, 0, 0);
  for(i = 0; i < selemsl; ++i) {
    mvwaddwstr(rw, 1 + i, 1, selems[i].s.s);
  }

  return 1;
}

void send_sel(uint32_t c) {
  endwin();
  char ss[1024];
  uint32_t cl = 3;
  ss[2] = c;

  {
    int32_t i;
    uint32_t dr = 0;
    for(i = 0; i < cstrl + dr; ++i) {
      if (cstr[i] != L' ') {
        cl += wchutf8(ss + cl, cstr + i, 1);
      }
    }
  }
  ss[0] = (cl - 3) >> 8;
  ss[1] = (cl - 3) & 0xFF;

  int32_t i;
  uint32_t cd = 2;
  for(i = 0; i < selemsl; ++i) {
    cd += wchutf8(ss + cl + cd, selems[i].s.s, selems[i].s.l);
    if (i < selemsl - 1) {
      ss[cl + cd] = '\n';
      ++cd;
    }
  }
  ss[cl    ] = (cd - 2) >> 8;
  ss[cl + 1] = (cd - 2) & 0xFF;
  ss[cl + cd] = '\0';
  //fprintf(stdout, "Sending down to america %s\n", ss + 3);
  //fprintf(stdout, "And %s\n", ss + cl + 2);
  
  ss[cl + cd] = selemsl;
  ++cd;
  for(i = 0; i < selemsl; ++i) {
    ss[cl + cd    ] = selems[i].sp;
    ss[cl + cd + 1] = selems[i].sl;
    cd += 2;
  }

  uint32_t al = 1 + strlen(apath);
  ss[cl + cd] = al;
  strcpy(ss + cl + cd + 1, apath);

  send(cs, ss, cl + cd + al, 0);
  EXIT = 1;
}

void handle_input(char ch) {
    switch(ch) {
      case 'h':
        if (cws == 0) {
          if (sel) {
            --sell;
          } else {
            --cpp;
          }
        } else {
          --cwss;
        }
        break;
      case 'l':
        if (cws == 0) {
          if (sel) {
            ++sell;
          } else {
            ++cpp;
          }
        } else {
          ++cwss;
        }
        break;
      case 'j':
        if (cws == 0) {
          if (sel) {
            sell += (wx - 4) / 2;
          } else {
            cpp += (wx - 4) / 2;
          }
        } else {
          ++cu;
        }
        break;
      case 'k':
        if (cws == 0) {
          if (sel) {
            sell -= (wx - 4) / 2;
          } else {
            cpp -= (wx - 4) / 2;
          }
        } else {
          --cu;
        }
        break;
      case 27:
        sel = 0;
        break;
      case 'v':
        sel ^= 1;
        sell = sel;
        break;
      case 'd':
        if (sel) {
          {
            int32_t i;
            for(i = 0; i <= sell; ++i) {
              if (cstr[cpp + i] == ' ') {
                ++sell;
              }
            }
          }
          memmove(cstr + cpp, cstr + cpp + sell, sizeof(cstr[0]) * (cstrl - cpp - sell));
          cstrl -= sell;
          setup_windows();
          sel = 0;
        }
        break;
      case ' ':
        if (cws == 0) {
          if (sel) {
            selems = realloc(selems, sizeof(selems[0]) * (selemsl + 1));
            wchar_t a[512];
            {
              int32_t i;
              uint32_t st = 0;
              uint32_t dr = 0;
              for(i = 0; i <= cpp + st; ++i) {
                if (cstr[i] == ' ') {
                  ++st;
                }
              }
              wcsncpy(a, cstr + cpp + st, sell + dr);
              for(i = 0; i < sell + dr; ++i) {
                if (cstr[cpp + st + i] == ' ') {
                  wcsncpy(a + i - dr, cstr + cpp + st + i + 1, sell + dr - i);
                  ++dr;
                }
              }
              a[sell] = L'\0';
              selems[selemsl].sp = cpp;
              selems[selemsl].sl = sell;
            }
            req(a);   
            create_new_panels();
            sel = 0;
          }
        } else {
          cus[cwss][cu] ^= 1;
        }
        break;
      case '\n':
        if (cws > 0) {
          if (fin_sel()) {
            clear_resp();
            cws = 0;
            cwss = 0;
            free(cus[0]);
            free(cus[1]);
            free(cus[2]);
            cu = 0;
          }
        }
        break;
      case 'y':
        EXIT = 1;
        COPY = 1;
        break;
      case 'O':
        send_sel(1);
        break;
      case 'I':
        send_sel(2);
        break;
      case 'J':
        ++cws;
        break;
      case 'K':
        if (cws > 0) {
          --cws;
        }
        break;
    }
}

void setup_server_connection() {
  int32_t rc;
  socklen_t len;
  struct sockaddr_un ssockaddr = {0}; 
  struct sockaddr_un csockaddr = {0};

  cs = socket(AF_UNIX, SOCK_STREAM, 0); 

  csockaddr.sun_family = AF_UNIX;
  strcpy(csockaddr.sun_path, "/tmp/ankee.client");
  len = sizeof(csockaddr);

  unlink("/tmp/ankee.client");
  rc = bind(cs, (struct sockaddr *) &csockaddr, len);
  if (rc == -1) {
    if (system("herbe \"Ankeec: Could not bind the socket!\" & disown")) {
      fprintf(stderr, "Could run herbe! [%m]\n");
      exit(1);
    }
    fprintf(stderr, "Could not bind the socket! [%m]\n");
    exit(1);
  }

  ssockaddr.sun_family = AF_UNIX;
  strcpy(ssockaddr.sun_path, "/tmp/ankeed.sock");
  rc = connect(cs, (struct sockaddr *) &ssockaddr, len);
  if (rc == -1) {
    if (system("herbe \"Ankeec: Could not connect the server!\" & disown")) {
      fprintf(stderr, "Could run herbe! [%m]\n");
      exit(1);
    }
    fprintf(stderr, "Could not connect to the server [%m]\n");
    exit(1);
  }
  fputs("Connected to the server!\n", stdout);
}

void shutdown_server_connection() {
  uint8_t a[2] = {0};
  send(cs, a, 2, 0);
  shutdown(cs, SHUT_RDWR);
  unlink("/tmp/r2k.client");
}

void rscr() {
  endwin();
  refresh();
  clear();
  setup_windows();
}

int main(int argc, char **argv) {
  if (argc < 3) {
    help();
  }
  
  setup_server_connection();
  /*req(L"きのこ");
  shutdown_server_connection();
  return 0;*/
  // char a[] = "きのこ人間駒マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠マメきこ海鼠";
  // cstr = malloc(sizeof(cstr[0]) * strlen(a));
  // utf2wch(a, cstr, &cstrl);

  cstr = malloc(sizeof(cstr[0]) * strlen(argv[1]));
  utf2wwch(argv[1], cstr, &cstrl);

  apath = strdup(argv[2]);

  setlocale(LC_ALL, "");

  initscr();
  curs_set(0);
  cbreak();
  nodelay(curscr, 1);
  noecho();
  clear();

  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = rscr;
  sigaction(SIGWINCH, &sa, NULL);

  getmaxyx(curscr, wy, wx);
  setup_windows();
  update();

  int ch;
  while ((ch = wgetch(sw)) != 'q') {
    handle_input(ch);
    if (EXIT) {
      break;
    }
    cpp = min(max(cpp, 0), cstrl - tsp - 1);
    update();
    doupdate();
  }
  endwin();
  if (COPY) {
    char a[1024] = "";
    sprintf(a, "echo \"%s\" | xclip -selection clipboard", argv[1]);
    if (system(a)) {
      fprintf(stderr, "Could not copy the text to the clipboard!\n");
    }
  }

  clear_resp();
  shutdown_server_connection();
  return 0;
}
