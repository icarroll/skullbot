#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

extern "C" {
#include <cairo.h>
#include <cairo-pdf.h>
}

using namespace std;

void die(string message) {
    cout << message << endl;
    exit(1);
}

const float POINTS_PER_INCH = 72.0;

const float PAPER_WIDTH = 5.5;
const float PAPER_HEIGHT = 8.5;

const float MARGIN = 1.0;

const float ROWS_PER_PAGE = 2;

const float COLUMN_HEIGHT = (PAPER_HEIGHT - MARGIN) / ROWS_PER_PAGE - MARGIN;

cairo_t * cr;
cairo_surface_t * csurf;

float unit = 1.0/5.0;

float step = unit/4;
float halfstep = step/2;
float step2 = step*2;
float ribstep = 2*step/3;
float vowelstep = ribstep;
float wordstep = step;
float colstep = 3*unit/2;

int cur_col = 0;
int cur_row = 0;

float elstep = step * sqrt(2)/2;

float hookrad = 2*halfstep/3;
float dotrad = halfstep/5;
float voicedlen = 2*step/3;

float leftx;
float spinex;
float rightx;
float markx;

float starty;
float riby;
float vowely;
float col_bot;

void new_column() {
    spinex = MARGIN + step + cur_col*colstep;
    cur_col += 1;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = (cur_row == 0 ? MARGIN : MARGIN/2) + cur_row * PAPER_HEIGHT/ROWS_PER_PAGE;
    riby = starty;

    col_bot = starty + COLUMN_HEIGHT;
}

bool first_page = true;

void new_page() {
    if (! first_page) cairo_surface_show_page(csurf);
    first_page = false;

    cur_row = 0;
    cur_col = 0;
}

void render_midline() {
    cairo_save(cr);
    cairo_set_line_width(cr, 0.005);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_source_rgb(cr, 0.5,0.5,0.5);
    cairo_move_to(cr, MARGIN,PAPER_HEIGHT/2);
    cairo_line_to(cr, PAPER_WIDTH-MARGIN,PAPER_HEIGHT/2);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void new_row() {
    render_midline();

    cur_row += 1;
    cur_col = 0;

    if (cur_row >= ROWS_PER_PAGE) new_page();
    new_column();
}

void render_voice_mark(float offset=0.0) {
    cairo_move_to(cr, markx, riby - voicedlen/2 + offset);
    cairo_line_to(cr, markx, riby + voicedlen/2 + offset);
}

void render_consonant(char c) {
    switch (c) {
    case 'b':
        render_voice_mark();
    case 'p':
        cairo_move_to(cr, spinex, riby);
        cairo_line_to(cr, rightx, riby);
        break;
    case 'v':
        render_voice_mark(halfstep/2);
    case 'f':
        cairo_move_to(cr, spinex, riby);
        cairo_line_to(cr, rightx, riby);
        cairo_rel_line_to(cr, -halfstep/2, halfstep);
        riby += halfstep;
        break;
    case 'D':
        render_voice_mark(halfstep/2);
    case 'T':
        cairo_move_to(cr, spinex, riby);
        cairo_line_to(cr, rightx, riby);
        cairo_rel_curve_to(cr, -2*step/3,0, -2*step/3,halfstep, 0,halfstep);
        riby += halfstep;
        break;
    case 'd':
        render_voice_mark();
    case 't':
        cairo_move_to(cr, leftx, riby);
        cairo_line_to(cr, rightx, riby);
        break;
    case 'z':
        render_voice_mark(halfstep/2);
    case 's':
        cairo_move_to(cr, leftx, riby);
        cairo_line_to(cr, rightx, riby);
        cairo_rel_line_to(cr, -halfstep/2, halfstep);
        riby += halfstep;
        break;
    case 'Z':
        render_voice_mark(halfstep/2);
    case 'S':
        cairo_move_to(cr, leftx, riby);
        cairo_line_to(cr, rightx, riby);
        cairo_rel_curve_to(cr, -2*step/3,0, -2*step/3,halfstep, 0,halfstep);
        riby += halfstep;
        break;
    case 'g':
        render_voice_mark();
    case 'k':
        cairo_move_to(cr, leftx, riby);
        cairo_line_to(cr, spinex, riby);
        break;
    case 'h':
        cairo_move_to(cr, leftx, riby);
        cairo_line_to(cr, spinex, riby);
        cairo_rel_curve_to(cr, -2*step/3,0, -2*step/3,halfstep, 0,halfstep);
        riby += halfstep;
        break;
    case 'w':
        cairo_move_to(cr, rightx, riby);
        riby += step;
        cairo_line_to(cr, spinex, riby);
        break;
    case 'l':
        cairo_move_to(cr, spinex+elstep, riby);
        riby += 2*elstep;
        cairo_line_to(cr, spinex-elstep, riby);
        break;
    case 'r':
        cairo_move_to(cr, spinex+elstep, riby);
        riby += 2*elstep;
        cairo_line_to(cr, spinex-elstep, riby);
        //cairo_rel_curve_to(cr, elstep/2,-halfstep, elstep,0, 0,halfstep);
        cairo_rel_curve_to(cr, halfstep,-elstep/2, 0,-elstep, -halfstep,0);
        break;
    case 'y':
        cairo_move_to(cr, spinex, riby);
        riby += step;
        cairo_line_to(cr, leftx, riby);
        break;
    case 'm':
        riby += 3*step/4;
        cairo_move_to(cr, spinex, riby);
        cairo_rel_curve_to(cr, 0,-step, step,-step, step,0);
        break;
    case 'n':
        riby += 3*step/4;
        cairo_move_to(cr, spinex-halfstep, riby);
        cairo_rel_curve_to(cr, 0,-step, step,-step, step,0);
        break;
    case 'N':
        riby += 3*step/4;
        cairo_move_to(cr, leftx, riby);
        cairo_rel_curve_to(cr, 0,-step, step,-step, step,0);
        break;
    default:
        // unimplemented letter
        cout << "unimplemented: " << c << endl;
        riby += halfstep;
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex,riby, halfstep, 0,2*M_PI);
        riby += halfstep;
        break;
    }
}

struct fills_t {
    bool l_top;
    bool c_top;
    bool r_top;
    bool l_bot;
    bool c_bot;
    bool r_bot;
};

fills_t consonant_fills(char c) {
    switch (c) {
    case 'b':
    case 'p':
        return {false, true, true, false, true, true};
        break;
    case 'v':
    case 'f':
        return {false, true, true, false, false, true};
        break;
    case 'D':
    case 'T':
        return {false, true, true, false, false, true};
        break;
    case 'd':
    case 't':
        return {true, true, true, true, true, true};
        break;
    case 'z':
    case 's':
        return {true, true, true, false, false, true};
        break;
    case 'Z':
    case 'S':
        return {true, true, true, false, false, true};
        break;
    case 'g':
    case 'k':
        return {true, true, false, true, true, false};
        break;
    case 'h':
        return {true, true, false, true, true, false};
        break;
    case 'w':
        return {false, false, true, false, true, false};
        break;
    case 'l':
        return {false, false, true, true, false, false};
        break;
    case 'r':
        return {false, false, true, true, false, false};
        break;
    case 'y':
        return {false, true, false, true, false, false};
        break;
    case 'm':
        return {false, false, true, false, true, true};
        break;
    case 'n':
        return {false, true, false, true, true, true};
        break;
    case 'N':
        return {true, false, false, true, true, false};
        break;
    case '`':
        return {true, true, false, true, true, false};
        break;
    default:
        // unimplemented letter
        cout << "unimplemented: " << c << endl;
        return {false, true, false, false, true, false};
        break;
    }
}

struct vowel_space_t {
    float before;
    float after;
};

vowel_space_t vowel_space(char c) {
    switch (c) {
    case 'b':
    case 'p':
    case 'd':
    case 't':
    case 'g':
    case 'k':
        return {0,0};
        break;
    case 'v':
    case 'f':
    case 'z':
    case 's':
        return {0,0};
        break;
    case 'D':
    case 'T':
    case 'Z':
    case 'S':
    case 'h':
        return {0,0};
        break;
    case 'w':
        return {0,0};
        break;
    case 'l':
    case 'r':
        return {0,elstep};
        break;
    case 'y':
        return {0,step};
        break;
    case 'm':
    case 'n':
    case 'N':
        return {step/4,0};
        break;
    case '`':
        return {0,halfstep};
        break;
    default:
        cout << "unimplemented: " << c << endl;
        return {0,0};
        break;
    }
}

void render_vowel_hook() {
    cairo_new_sub_path(cr);
    cairo_arc(cr, spinex-halfstep,riby, halfstep, 2*M_PI,M_PI);
    riby += halfstep;
}

void render_monopthong(char c, float vowelx) {
    float x = vowelx;
    float y = vowely;

    switch (c) {
    case 'i':   // seat
        x += halfstep/2 /2;
        y += halfstep * sqrt(3)/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
        break;
    case 'I':   // sit
        x += halfstep * sqrt(3)/2 /2;
        y += halfstep/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, -halfstep/2);
        break;
    case 'e':   // sate
        x += halfstep /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep, 0);
        break;
    case 'E':   // set
        x += halfstep * sqrt(3)/2 /2;
        y -= halfstep/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, +halfstep/2);
        break;
    case 'A':   // sat
        x += halfstep/2 /2;
        y -= halfstep * sqrt(3)/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep/2, halfstep * sqrt(3)/2);
        break;
    case 'a':   // sot
        y -= halfstep /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, 0, halfstep);
        break;
    case 'O':   // sup
        x -= halfstep/2 /2;
        y -= halfstep * sqrt(3)/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep/2, halfstep * sqrt(3)/2);
        break;
    case 'o':   // so
        x -= halfstep /2;
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep, 0);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad*2, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_restore(cr);

        break;
    case 'U':   // soot
        x -= halfstep * sqrt(3)/2 /2;
        y += halfstep/2 /2;
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep * sqrt(3)/2, -halfstep/2);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad*2, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_restore(cr);

        break;
    case 'u':   // suit
        x -= halfstep/2 /2;
        y += halfstep * sqrt(3)/2 /2;
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep/2, -halfstep * sqrt(3)/2);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad*2, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_restore(cr);

        break;
    default:
        cout << "unknown vowel: " << c << endl;
        break;
    }
}

void render_i_dipthong(char c, float vowelx) {
    float x = vowelx;
    float y = vowely;

    switch (c) {
    case 'e':   // !e ei say
        x += halfstep /2 + halfstep/2 /2;
        y += halfstep * sqrt(3)/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep, 0);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
        break;
    case 'a':   // !a ai site
        x += halfstep * sqrt(3)/2 /2;
        y -= halfstep /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, 0, halfstep);
        cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, -halfstep/2);
        break;
    case 'o':   // !o oi soy
        x -= halfstep /2;
        y += halfstep * sqrt(3)/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep, 0);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
        break;
    default:
        cout << "unknown i dipthong: " << c << endl;
        return;
        break;
    }
}

void render_u_dipthong(char c, float vowelx) {
    float x = vowelx;
    float y = vowely;

    switch (c) {
    case 'a':   // ^a au south
        x -= halfstep * sqrt(3)/2 /2;
        y -= halfstep /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, 0, halfstep);
        cairo_rel_line_to(cr, halfstep * sqrt(3)/2, -halfstep/2);
        break;
    case 'o':   // ^o ou low
        x -= halfstep /2 + halfstep/2 /2;
        y += halfstep * sqrt(3)/2 /2;
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep, 0);
        cairo_rel_line_to(cr, halfstep/2, -halfstep * sqrt(3)/2);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad*2, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_restore(cr);

        break;
    default:
        cout << "unknown u dipthong: " << c << endl;
        break;
    }
}

bool vowel(char c) {
    switch (c) {
    case 'a':
    case 'A':
    case 'e':
    case 'E':
    case 'i':
    case 'I':
    case 'o':
    case 'O':
    case 'u':
    case 'U':
    case '!':
    case '^':
        return true;
        break;
    default:
        return false;
        break;
    }
}

bool voiced(char c) {
    switch (c) {
    case 'b':
    case 'v':
    case 'D':
    case 'd':
    case 'z':
    case 'Z':
    case 'g':
        return true;
        break;
    default:
        return false;
        break;
    }
}

bool logogram(char c) {
    switch (c) {
    case '&':
    case '@':
        return true;
        break;
    default:
        return false;
        break;
    }
}

void render_logogram_word(string w) {
    if (w == "&") {
        cairo_new_sub_path(cr);
        cairo_arc_negative(cr, spinex-halfstep,riby, halfstep, M_PI,2*M_PI);
        cairo_rel_line_to(cr, 0, step);
        cairo_rel_curve_to(cr, 0,step, -step,0, 0,0);
        cairo_rel_curve_to(cr, halfstep,0, halfstep,step, 0,step);
        cairo_rel_line_to(cr, -step, 0);
        riby += step*2;
    }
    else if (w == "@@@") {
        // red mark for unknown word
        cairo_save(cr);
        cairo_set_source_rgb(cr, 1,0,0);
        cairo_rectangle(cr, spinex-halfstep,riby, step,step);
        cairo_fill(cr);
        cairo_restore(cr);
        riby += step;
    }
    else cout << "unknown logogram word: " << w << endl;

    cairo_stroke(cr);
}

void render_vowel_word(string w) {
    if (w == "A") {
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex,riby, halfstep, 2*M_PI,M_PI);
        riby += halfstep;
    }
    else if (w == "I") {
        riby += step;
        cairo_move_to(cr, spinex, riby);
        cairo_rel_curve_to(cr, step,0, step,-step, step,-step);
    }
    else if (w == "O") {
        riby += halfstep;
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex,riby, halfstep, 0,2*M_PI);
        riby += halfstep;
    }
    else cout << "unknown vowel word: " << w << endl;

    cairo_stroke(cr);
}

void render_digit(char c) {
    switch (c) {
    case '0':
        cairo_move_to(cr, spinex-halfstep, riby);
        cairo_line_to(cr, spinex+halfstep, riby);
        cairo_line_to(cr, spinex+halfstep/2, riby+step/3);
        cairo_curve_to(cr, spinex+halfstep,riby+step/3, spinex+halfstep,riby+step, spinex,riby+step);
        cairo_curve_to(cr, spinex-step/3,riby+step, spinex-halfstep,riby+2*step/3, spinex-step/3,riby+halfstep);
        riby += step;
        break;
    case '1':
        cairo_move_to(cr, spinex, riby);
        cairo_line_to(cr, spinex, riby+step);
        riby += step;
        break;
    case '2':
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex+halfstep/2,riby, halfstep/2, 0,M_PI);
        cairo_line_to(cr, spinex, riby+step);
        riby += step;
        break;
    case '3':
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex+3*halfstep/2,riby, halfstep/2, 0,M_PI);
        cairo_arc(cr, spinex+halfstep/2,riby, halfstep/2, 0,M_PI);
        cairo_line_to(cr, spinex, riby+step);
        riby += step;
        break;
    case '4':
        cairo_move_to(cr, spinex+3*halfstep/2, riby);
        cairo_line_to(cr, spinex+halfstep, riby+halfstep);
        cairo_rel_curve_to(cr, 0,-3*halfstep/2, -halfstep,-3*halfstep/2, -halfstep,0);
        cairo_line_to(cr, spinex, riby+step);
        riby += step;
        break;
    case '5':
        cairo_move_to(cr, spinex+halfstep, riby);
        cairo_line_to(cr, spinex, riby+halfstep);
        cairo_rel_curve_to(cr, step,0, -halfstep/2,halfstep, -halfstep/2,halfstep);
        riby += step;
        break;
    case '6':
        cairo_move_to(cr, spinex+halfstep, riby+halfstep);
        cairo_rel_curve_to(cr, -step*2,0, 0,-step, 0,0);
        cairo_rel_curve_to(cr, 0,halfstep, -step,halfstep, -step,halfstep);
        cairo_line_to(cr, spinex+halfstep, riby+step);
        riby += step;
        break;
    case '7':
        cairo_move_to(cr, spinex-halfstep, riby);
        cairo_curve_to(cr, spinex,riby+halfstep/2, spinex,riby+halfstep/2, spinex+halfstep,riby+halfstep/4);
        cairo_curve_to(cr, spinex+step,riby, spinex+step/2,riby-step/2+halfstep/8, spinex+halfstep,riby+halfstep/4);
        cairo_line_to(cr, spinex, riby+step);
        cairo_line_to(cr, spinex-halfstep, riby+halfstep);
        riby += step;
        break;
    case '8':
        cairo_move_to(cr, spinex-halfstep, riby);
        cairo_curve_to(cr, spinex-halfstep,riby+halfstep, spinex+halfstep/2,riby+halfstep, spinex+halfstep/2,riby);
        cairo_line_to(cr, spinex-halfstep, riby+step);
        cairo_line_to(cr, spinex+halfstep, riby+step);
        riby += step;
        break;
    case '9':
        cairo_move_to(cr, spinex, riby+halfstep);
        cairo_curve_to(cr, spinex-step,riby+halfstep/2, spinex,riby-halfstep, spinex,riby+halfstep);
        cairo_line_to(cr, spinex, riby+step);
        riby += step;
        break;
    default:
        cout << "unknown digit: " << c << endl;
        break;
    }
}

void render_numeral(string w) {
    for (char c : w) {
        render_digit(c);
        riby += wordstep/2;
    }

    cairo_stroke(cr);
}

void render_phonetic_word(string w) {
    riby = starty;

    if (logogram(w[0])) {
        render_logogram_word(w);
        return;
    }

    if (vowel(w[0])) {
        render_vowel_word(w);
        return;
    }

    if (isdigit(w[0])) {
        render_numeral(w);
        return;
    }

    if (w[0] != '`') riby += ribstep/2;

    int ix = 0;
    int lastrib_ix;
    while (ix < w.length()) {
        int nextrib_ix = ix + 1;
        while (nextrib_ix<w.length() && vowel(w[nextrib_ix])) nextrib_ix += 1;

        float prerib_riby = riby;
        if (w[ix] == '`') render_vowel_hook();
        else render_consonant(w[ix]);
        float rib_height = riby - prerib_riby;

        float pregap_riby = riby;

        // do current and next ribs need a gap?
        fills_t fills = consonant_fills(w[ix]);
        if (nextrib_ix<w.length()) {
            fills_t fills2 = consonant_fills(w[nextrib_ix]);
            if (fills.l_bot && fills2.l_top
             || fills.c_bot && fills2.c_top
             || fills.r_bot && fills2.r_top
             ) { // || ix+1<w.length() && vowel(w[ix+1])) {
                // gap
                riby += ribstep;
            }
            else {
                // no gap
                //XXX experiment with small gap due to eg "disquiet"
            }
        }
        else {
            // no next rib
            lastrib_ix = ix;
        }

        // render vowel if any
        if (nextrib_ix < w.length() && vowel(w[ix+1])) {
            // there's a vowel then a following rib
            if (w[ix] == '`') vowely = pregap_riby - halfstep/2;
            else if (w[nextrib_ix] == '`') vowely = riby + halfstep/2;
            else {
                // next rib is a consonant
                float prevy, nexty;

                // this only works because voiced ribs never have vowel space
                if (voiced(w[ix])) {
                    prevy = pregap_riby - rib_height/2 + voicedlen/2;
                }
                else prevy = pregap_riby - vowel_space(w[ix]).after;

                if (voiced(w[nextrib_ix])) {
                    //TODO assume 0 because we don't know height of next rib
                    nexty = riby + 0/2 - voicedlen/2;
                }
                else nexty = riby + vowel_space(w[nextrib_ix]).before;

                float vowel_room = nexty - prevy;
                float vowel_needs = halfstep + dotrad*2;
                if (vowel_room < vowel_needs) {
                    float skootch = vowel_needs - vowel_room;
                    riby += skootch;
                    nexty += skootch;
                }

                vowely = (prevy + nexty) / 2;
            }

            float vowelx = markx;
            if (w[ix+1] == '!') render_i_dipthong(w[ix+2], vowelx);
            else if (w[ix+1] == '^') render_u_dipthong(w[ix+2], vowelx);
            else render_monopthong(w[ix+1], vowelx);
        }

        ix = nextrib_ix;
    }

    if (w[lastrib_ix] == '`') riby -= halfstep;
    else riby += ribstep/2;

    cairo_move_to(cr, spinex, starty);
    cairo_line_to(cr, spinex, riby);
    cairo_stroke(cr);

    if (w[lastrib_ix] == '`') riby += halfstep;
}

void render_phonetic_words(vector<string> & ws) {
    for (auto it=ws.begin() ; it!=ws.end() ; ++it) {
        auto w = * it;
        render_phonetic_word(w);
        starty = riby + wordstep;
        //TODO move this into render_column
        if (starty > col_bot) {
            ws.assign(++it, ws.end());
            return;
        }
    }
    ws.clear();
}

map<string, string> pronunciation;

string phoneticize_word(string raw_w) {
    if (isdigit(raw_w[0])) return raw_w;

    string w;
    for (char c : raw_w) if (isalpha(c) || c == '\'') w.push_back(tolower(c));

    if (w.length() == 0) return "";

    if (pronunciation.count(w)) return pronunciation[w];

    cout << "unknown word: " << raw_w << endl;
    return "@@@"; // red mark for unknown word
}

vector<string> phoneticize_words(vector<string> ws) {
    vector<string> ps;
    for (auto w : ws) {
        string p = phoneticize_word(w);
        if (p.length() != 0) ps.push_back(p);
    }
    return ps;
}

vector<string> split_words(string text) {
    istringstream iss(text);
    vector<string> ws;
    string w;
    while (iss >> w) {
        int ix;
        while ((ix=w.find("-")) != -1) {
            ws.push_back(w.substr(0,ix));
            w = w.substr(ix+1);
        }
        ws.push_back(w);
    }
    return ws;
}

void render_column(vector<string> & ps) {
    new_column();
    if (spinex > PAPER_WIDTH-MARGIN-step-halfstep) new_row();
    render_phonetic_words(ps);
}

void render_columns(string text) {
    vector<string> ws = split_words(text);
    vector<string> ps = phoneticize_words(ws);
    while (ps.size()) render_column(ps);
}

void draw_skull_bat(float width, float x, float y) {
    cairo_surface_t * skullbat =
        cairo_image_surface_create_from_png("skullbat.png");
    cairo_save(cr);
    int w = cairo_image_surface_get_width(skullbat);
    cairo_translate(cr, x, y);
    cairo_scale(cr, width /w, width /w);
    cairo_set_source_surface(cr, skullbat, 0,0);
    cairo_paint(cr);
    cairo_restore(cr);
    cairo_surface_destroy(skullbat);

}

void load_phonetic() {
    ifstream phonetics("pronunciation.txt");
    string word, phonetic;
    while (phonetics >> word >> phonetic) {
        pronunciation[word] = phonetic;
    }
}

vector<string> load_text(string filename) {
    ifstream text(filename);
    vector<string> lines;
    string line;
    while (getline(text, line)) lines.push_back(line);
    return lines;
}

int main(int nargs, char * args[])
{
    load_phonetic();

    csurf = cairo_pdf_surface_create(
        "abjad.pdf",
        PAPER_WIDTH * POINTS_PER_INCH,
        PAPER_HEIGHT * POINTS_PER_INCH);
    cr = cairo_create(csurf);
    cairo_scale(cr, POINTS_PER_INCH, POINTS_PER_INCH);

    //draw_skull_bat(0.5, MARGIN/3, MARGIN);

    cairo_set_line_width(cr, 0.01);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb(cr, 0,0,0);

    vector<string> paras = load_text("chapter04.txt");
    for (string para : paras) {
        if (para.substr(0,7) == "Chapter") new_page();
        render_columns(para);
    }

    cairo_surface_show_page(csurf);

    cairo_destroy(cr);
    cairo_surface_finish(csurf);
    cairo_surface_destroy(csurf);
    return 0;
}
