#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
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
const float FONT_SIZE = 12.0;

const float PAPER_WIDTH = 12.3172;
const float PAPER_HEIGHT = 8.75;

const float MARGIN = 1.0;

const float ROWS_PER_PAGE = 2;

const float COLUMN_HEIGHT = (PAPER_HEIGHT - MARGIN) / ROWS_PER_PAGE - MARGIN;

cairo_t * cr;
cairo_surface_t * csurf;

float SCALE = 1.0;

float LINE_WIDTH = SCALE*0.01;

float unit = SCALE/5.0;

float step = unit/4;
float halfstep = step/2;
float step2 = step*2;
float ribstep = 2*step/3;
float vowelstep = ribstep;
float wordstep = step;
float colstep = 3*unit/2;

float elstep = step * sqrt(2)/2;

float hookrad = 2*halfstep/3;
float dotrad = 2*halfstep/5;
float voicedlen = 2*step/3;

float vowel_size = halfstep + 2*dotrad;

void set_scale(float newscale) {
    SCALE = newscale;

    LINE_WIDTH = SCALE*0.01;
    cairo_set_line_width(cr, LINE_WIDTH);

    unit = SCALE/5.0;

    step = unit/4;
    halfstep = step/2;
    step2 = step*2;
    ribstep = 2*step/3;
    vowelstep = ribstep;
    wordstep = step;
    colstep = 3*unit/2;

    elstep = step * sqrt(2)/2;

    hookrad = 2*halfstep/3;
    dotrad = 2*halfstep/5;
    voicedlen = 2*step/3;

    vowel_size = halfstep + 2*dotrad;
}

int cur_col = 0;
int cur_row = 0;

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

int page_number = 0;

bool even(int n) {
    return n % 2 == 0;
}

vector<string> split_words(string text);
vector<string> phoneticize_words(vector<string> ws);
void render_phonetic_words(vector<string> & ws);
void render_at_inches(string text, float x, float y) {
    vector<string> ws = split_words(text);
    vector<string> ps = phoneticize_words(ws);

    // column is undefined in this context
    cur_col = numeric_limits<int>::min();
    col_bot = numeric_limits<float>::max();

    spinex = x;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = y;
    riby = starty;

    render_phonetic_words(ps);
}

void mark_page_number() {
    string text = to_string(page_number);
    float x = even(page_number) ? MARGIN/2 : PAPER_WIDTH - MARGIN/2;
    float y = MARGIN/2;
    render_at_inches(text, x, y);
}

void new_page() {
    page_number += 1;

    if (page_number > 1) {
        cairo_surface_show_page(csurf);
    }

    mark_page_number();

    cur_row = 0;
    cur_col = 0;
}

//TODO change to render row separator in case rows != 2
void render_midline() {
    cairo_save(cr);
    cairo_set_line_width(cr, LINE_WIDTH/2);
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
        cout << "unimplemented consonant: " << c << endl;
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
        cout << "unimplemented consonant fill: " << c << endl;
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
        return {0,0};
        break;
    case 'h':
        return {0,halfstep/2};
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
        cout << "unimplemented vowel space: " << c << endl;
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
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
        cairo_stroke(cr);
        break;
    case 'I':   // sit
        x += halfstep * sqrt(3)/2 /2;
        y += halfstep/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, -halfstep/2);
        cairo_stroke(cr);
        break;
    case 'e':   // sate
        x += halfstep /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep, 0);
        cairo_stroke(cr);
        break;
    case 'E':   // set
        x += halfstep * sqrt(3)/2 /2;
        y -= halfstep/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, +halfstep/2);
        cairo_stroke(cr);
        break;
    case 'A':   // sat
        x += halfstep/2 /2;
        y -= halfstep * sqrt(3)/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep/2, halfstep * sqrt(3)/2);
        cairo_stroke(cr);
        break;
    case 'a':   // sot
        y -= halfstep /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, 0, halfstep);
        cairo_stroke(cr);
        break;
    case 'O':   // caught, if you don't merge with cot
        x -= halfstep/2 /2;
        y -= halfstep * sqrt(3)/2 /2;
        /*
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep/2, halfstep * sqrt(3)/2);
        cairo_stroke(cr);
        */
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep/2, halfstep * sqrt(3)/2);
        cairo_stroke(cr);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_stroke(cr);
        cairo_restore(cr);

        break;
    case '@':   // sup (schwa-ish)
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_fill(cr);
        break;
    case 'o':   // so
        x -= halfstep /2;
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep, 0);
        cairo_stroke(cr);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_stroke(cr);
        cairo_restore(cr);

        break;
    case 'U':   // soot
        x -= halfstep * sqrt(3)/2 /2;
        y += halfstep/2 /2;
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep * sqrt(3)/2, -halfstep/2);
        cairo_stroke(cr);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_stroke(cr);
        cairo_restore(cr);

        break;
    case 'u':   // suit
        x -= halfstep/2 /2;
        y += halfstep * sqrt(3)/2 /2;
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep/2, -halfstep * sqrt(3)/2);
        cairo_stroke(cr);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_stroke(cr);
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
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, -halfstep, 0);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
        cairo_stroke(cr);
        break;
    case 'a':   // !a ai site
        x += halfstep * sqrt(3)/2 /2;
        y -= halfstep /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, 0, halfstep);
        cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, -halfstep/2);
        cairo_stroke(cr);
        break;
    case 'o':   // !o oi soy
        //TODO open circle because it begins rounded
        x -= halfstep /2;
        y += halfstep * sqrt(3)/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep, 0);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
        cairo_stroke(cr);
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
        cairo_fill(cr);
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, 0, halfstep);
        cairo_rel_line_to(cr, halfstep * sqrt(3)/2, -halfstep/2);
        cairo_stroke(cr);
        break;
    case 'o':   // ^o ou low
        x -= halfstep /2 + halfstep/2 /2;
        y += halfstep * sqrt(3)/2 /2;
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep, 0);
        cairo_rel_line_to(cr, halfstep/2, -halfstep * sqrt(3)/2);
        cairo_stroke(cr);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_stroke(cr);
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
    case '@':
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
    case 'X':
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
    else if (w == "XXX") {
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

void render_vowel(string v, float vowelx);
void render_vowel_word(string w) {
    if (w == "A") {
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex,riby, halfstep, 2*M_PI,M_PI);
        riby += halfstep;
    }
    else if (w == "E") {
        riby += step;
        cairo_move_to(cr, spinex, riby);
        cairo_rel_curve_to(cr, step,0, step,-step, step,-step);
        vowely = riby - halfstep;
        render_vowel("i", markx);
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
    case '#':
        // ordinal ideogram
        cairo_move_to(cr, leftx, riby);
        cairo_line_to(cr, spinex, riby);
        cairo_rel_curve_to(cr, -2*step/3,0, -2*step/3,halfstep, 0,halfstep);
        riby += halfstep;
        cairo_line_to(cr, rightx, riby);

        /* TODO "rd" ligature perhaps?
        riby += wordstep/2;

        cairo_move_to(cr, rightx, riby);
        cairo_line_to(cr, spinex-halfstep, riby);
        cairo_rel_curve_to(cr, -2*step/3,0, -2*step/3,halfstep, 0,halfstep);
        riby += halfstep;
        cairo_line_to(cr, spinex, riby);
        cairo_rel_curve_to(cr, -2*step/3,0, -2*step/3,halfstep, 0,halfstep);
        riby += halfstep;

        riby += wordstep/2;

        cairo_move_to(cr, rightx, riby);
        riby += halfstep;
        cairo_line_to(cr, spinex-halfstep, riby);
        cairo_rel_curve_to(cr, -3*halfstep,halfstep, -3*halfstep,halfstep, 0,halfstep);
        riby += halfstep;
        */
        break;
    default:
        cout << "unknown digit: " << c << endl;
        break;
    }
}

void render_numeral(string w) {
    for (char c : w) {
        if (isalpha(c)) {
            // assume it's an ordinal
            render_digit('#');
            break;
        }

        render_digit(c);
        riby += wordstep/2;
    }

    cairo_stroke(cr);
}

void render_vowel(string v, float vowelx) {
    cairo_path_t * saved_path = cairo_copy_path(cr);
    cairo_new_path(cr);

    if (v[0] == '!') render_i_dipthong(v[1], vowelx);
    else if (v[0] == '^') render_u_dipthong(v[1], vowelx);
    else render_monopthong(v[0], vowelx);

    cairo_new_path(cr);
    cairo_append_path(cr, saved_path);
    cairo_path_destroy(saved_path);
}

void render_vowel_at(string v, float x, float y) {
    vowely = y;
    render_vowel(v, x);
}

vector<string> split_vowels(string text) {
    vector<string> vs;

    int ix = 0;
    while (ix < text.length()) {
        if (text[ix] == '!' || text[ix] == '^') {
            vs.push_back(text.substr(ix, 2));
            ix += 2;
        }
        else {
            vs.push_back(text.substr(ix, 1));
            ix += 1;
        }
    }

    return vs;
}

void render_punct(string w) {
    cairo_path_t * saved_path = cairo_copy_path(cr);
    cairo_new_path(cr);

    if (w == "-") {
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex, riby, dotrad, 0, 2*M_PI);
        cairo_fill(cr);
    }
    else if (w == "--" || w == ";" || w == ":") {
        cairo_move_to(cr, spinex-step-halfstep, riby);
        cairo_line_to(cr, spinex-halfstep-halfstep, riby+halfstep);
        cairo_line_to(cr, spinex-step-halfstep, riby+step);
        cairo_move_to(cr, spinex-halfstep-halfstep, riby+halfstep);
        cairo_line_to(cr, spinex-halfstep, riby+halfstep);
        cairo_stroke(cr);
        riby += step;
    }
    else if (w == "----") {
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex, riby, dotrad, 0, 2*M_PI);
        riby += wordstep;
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex, riby, dotrad, 0, 2*M_PI);
        riby += wordstep;
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex, riby, dotrad, 0, 2*M_PI);
        /*
        riby += wordstep;
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex, riby, dotrad, 0, 2*M_PI);
        */
        cairo_fill(cr);
    }
    else if (w == ",") {
        cairo_move_to(cr, spinex-step-halfstep, riby-halfstep);
        cairo_line_to(cr, spinex-halfstep-halfstep, riby);
        cairo_line_to(cr, spinex-step-halfstep, riby+halfstep);
        cairo_stroke(cr);
    }
    else if (w == ".") {
        cairo_move_to(cr, spinex-step-halfstep, riby);
        cairo_line_to(cr, spinex, riby);
        cairo_stroke(cr);
    }
    else if (w == "!") {
        cairo_move_to(cr, spinex-step-halfstep/2, riby-step);
        cairo_rel_curve_to(cr, -step,halfstep, halfstep,halfstep, -halfstep,step);
        cairo_line_to(cr, spinex, riby);
        cairo_stroke(cr);
    }
    else if (w == "?") {
        cairo_move_to(cr, spinex-step-halfstep, riby-halfstep);
        cairo_rel_curve_to(cr, 0,-halfstep, step,-halfstep, 0,halfstep);
        cairo_line_to(cr, spinex, riby);
        cairo_stroke(cr);
    }
    else if (w == "\"") {
        cairo_move_to(cr, spinex-step-halfstep, riby);
        cairo_line_to(cr, spinex-step, riby);
        riby += ribstep;
        cairo_move_to(cr, spinex-step-halfstep, riby);
        cairo_line_to(cr, spinex-step, riby);
        cairo_stroke(cr);
    }
    else if (w == "(") {
        cairo_move_to(cr, spinex, riby);
        cairo_rel_curve_to(cr, -step,0, -step-halfstep,halfstep, -step-halfstep,step);
        cairo_stroke(cr);
    }
    else if (w == ")") {
        cairo_move_to(cr, spinex, riby);
        cairo_rel_curve_to(cr, -step,0, -step-halfstep,-halfstep, -step-halfstep,-step);
        cairo_stroke(cr);
    }
    else cout << "unknown punct: " << w << endl;

    cairo_new_path(cr);
    cairo_append_path(cr, saved_path);
    cairo_path_destroy(saved_path);
}

bool isprosody(char c) {
    switch (c) {
    case '/':
    case '-':
    case ',':
    case '.':
    case '!':
    case '?':
    case '(':
    case ')':
    case '"':
    case ';':
    case ':':
        return true;
        break;
    default:
        return false;
        break;
    }
}

void render_column_divider() {
    float col_top = (cur_row == 0 ? MARGIN : MARGIN/2) + cur_row * PAPER_HEIGHT/ROWS_PER_PAGE;
    float col_bot = col_top + COLUMN_HEIGHT;

    float frac = COLUMN_HEIGHT / 3;

    cairo_save(cr);
    cairo_set_line_width(cr, LINE_WIDTH/2);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_source_rgb(cr, 0.5,0.5,0.5);
    cairo_move_to(cr, spinex, col_top+frac);
    cairo_line_to(cr, spinex, col_bot-frac);
    cairo_stroke(cr);
    cairo_restore(cr);
}

bool emphasis = false;

const string STARS = "*****";

void render_phonetic_word(string w) {
    riby = starty;

    if (w == STARS) {
        render_column_divider();
        return;
    }

    if (logogram(w[0])) {
        render_logogram_word(w);
        return;
    }

    // this must come before vowel() because '!' is a "vowel"
    if (isprosody(w[0])) {
        render_punct(w);
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
             ) {
                // gap
                riby += ribstep;
            }
            else {
                // no gap
                //XXX experiment with small gap due to eg "disquiet"
                riby += ribstep/3;
            }
        }
        else {
            // no next rib
            lastrib_ix = ix;
        }

        // render vowel if any
        float vowely_center;
        if (nextrib_ix < w.length() && vowel(w[ix+1])) {
            // there's a vowel then a following rib
            string vtext = w.substr(ix + 1, nextrib_ix - ix - 1);
            vector<string> vs = split_vowels(vtext);

            if (w[ix] == '`') vowely_center = pregap_riby - halfstep/2;
            else if (w[nextrib_ix] == '`') vowely_center = riby + halfstep/2;
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
                float vowel_needs = vowel_size * vs.size();
                if (vowel_room < vowel_needs) {
                    float skootch = vowel_needs - vowel_room;
                    riby += skootch;
                    nexty += skootch;
                }

                vowely_center = (prevy + nexty) / 2;
            }

            float vowelx = markx;
            vowely = vowely_center - vowel_size * (vs.size()-1) / 2.0;
            for (string v : vs) {
                render_vowel(v, vowelx);
                vowely += vowel_size;
            }
        }

        ix = nextrib_ix;
    }

    if (w[lastrib_ix] == '`') riby -= halfstep;
    else riby += ribstep/2;

    cairo_move_to(cr, spinex, starty);
    cairo_line_to(cr, spinex, riby);

    if (w[lastrib_ix] == '`') riby += halfstep;

    if (emphasis) {
        cairo_move_to(cr, spinex-step-halfstep, starty);
        cairo_line_to(cr, spinex-step-halfstep, riby);
    }

    cairo_stroke(cr);
}

void render_phonetic_words(vector<string> & ws) {
    for (auto it=ws.begin() ; it!=ws.end() ; ++it) {
        auto w = * it;
        if (w == "/") {
            emphasis = ! emphasis;
            continue;
        }

        render_phonetic_word(w);
        starty = riby + wordstep;
        //TODO move this into render_column
        if (starty > col_bot) {
            it += 1;
            while ((it != ws.end()) && isprosody((* it)[0])) {
                if (* it == STARS) break;
                if (* it == "(") break;
                if (* it == "/") break;
                render_phonetic_word(* it);
                starty = riby + wordstep;
                it += 1;
            }
            ws.assign(it, ws.end());
            return;
        }
    }
    ws.clear();
}

map<string, string> pronunciation;

string phoneticize_word(string raw_w) {
    if (raw_w == STARS) return raw_w;
    if (isdigit(raw_w[0])) return raw_w;
    if (isprosody(raw_w[0])) return raw_w;
    if (raw_w[0] == '\'') raw_w = raw_w.substr(1);

    string w;
    for (char c : raw_w) w.push_back(tolower(c));
    if (w.empty()) return "";

    if (pronunciation.count(w)) return pronunciation[w];

    cout << "unknown word: " << raw_w << endl;
    return "XXX"; // red mark for unknown word
}

vector<string> phoneticize_words(vector<string> ws) {
    vector<string> ps;
    for (auto w : ws) {
        string p = phoneticize_word(w);
        if (p.length() != 0) ps.push_back(p);
    }
    return ps;
}

set<string> abbrevs = {"Mrs", "Mr", "St", "EDW", "E", "M"};

vector<string> split_words(string text) {
    vector<string> ws;
    string w;

    for (char c : text) {
        // this whole * thing is an ugly hack
        if (c == '*') {
            w.push_back(c);

            continue;
        }

        if (isspace(c)) {
            if (w.back() == '*') continue;
            else if (! w.empty()) {
                ws.push_back(w);
                w.clear();
            }
        }
        else if (isdigit(c)) {
            if (w.empty() || isdigit(w.back())) w.push_back(c);
            else {
                ws.push_back(w);
                w.clear();
                w.push_back(c);
            }
        }
        else if (isalpha(c) || c == '\'') {
            if (w.empty() || isalpha(w.back()) || w.back() == '\'') {
                w.push_back(c);
            }
            else if (isdigit(w.back())) w.push_back(c);
            else {
                ws.push_back(w);
                w.clear();
                w.push_back(c);
            }
        }
        else if (c == '-') {
            if (w.empty() || w.back() == '-') w.push_back(c);
            else {
                ws.push_back(w);
                w.clear();
                w.push_back(c);
            }
        }
        else if (c == '.') {
            if (w.empty()) ws.push_back(string(1, c));
            else if (abbrevs.count(w) == 0) {
                ws.push_back(w);
                w.clear();
                ws.push_back(string(1, c));
            }
        }
        else switch (c) {
        case '"':
        case ',':
        case ';':
        case ':':
        case '!':
        case '?':
        case '(':
        case ')':
            if (! w.empty()) {
                ws.push_back(w);
                w.clear();
            }
            ws.push_back(string(1, c));
            break;
        case '_':
            if (! w.empty()) {
                ws.push_back(w);
                w.clear();
            }
            ws.push_back(string(1, '/'));
            break;
        default:
            cout << "can't handle character: " << c << endl;
            break;
        }
    }
    if (! w.empty()) ws.push_back(w);

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

void render_latin(string text, float x, float y) {
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text.c_str());

    cairo_set_source_rgb(cr, 0,0,0);
    cairo_fill(cr);
}

void render_skullbat(string text, float x, float y) {
    // column is undefined in this context
    cur_col = numeric_limits<int>::min();
    col_bot = numeric_limits<float>::max();

    spinex = x;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = y;
    riby = starty;

    render_phonetic_word(text);
}

void render_key_page() {
    draw_skull_bat(1, PAPER_WIDTH/2-0.5, MARGIN - 3.0/4.0 /2);

    set_scale(1.5);

    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, FONT_SIZE/POINTS_PER_INCH);

    cairo_font_extents_t fe;
    cairo_text_extents_t te;
    cairo_font_extents(cr, & fe);

    int x = 0;
    int y = 0;

    render_latin("Skullbat Key", MARGIN, MARGIN+fe.height+ y *1.5*fe.height);

    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_font_extents(cr, & fe);
    cairo_text_extents(cr, "W", & te);

    y += 1;
    render_latin("Consonants", MARGIN, MARGIN+fe.height + y *1.5*fe.height);
    y += 1;
    render_latin("(voicing mark is optional if unambiguous)", MARGIN, MARGIN+fe.height + y *1.5*fe.height);

    y += 1;
    x = 0;
    for (string s : {"P","B","M","F","V","W"}) {
        render_latin(s, MARGIN+x*0.6, MARGIN+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"p","b","m","f","v","w"}) {
        render_skullbat(s, MARGIN+x*0.6+te.width*1.5, MARGIN+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    x = 0;
    for (string s : {"","","","TH","DH",""}) {
        float nudge = s.length() > 1 ? 0.1 : 0;
        render_latin(s, MARGIN+x*0.6-nudge, MARGIN+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"","","","T","D",""}) {
        if (s.length()) render_skullbat(s, MARGIN+x*0.6+te.width*1.5, MARGIN+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    x = 0;
    for (string s : {"T","D","N","S","Z","L"}) {
        render_latin(s, MARGIN+x*0.6, MARGIN+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"t","d","n","s","z","l"}) {
        render_skullbat(s, MARGIN+x*0.6+te.width*1.5, MARGIN+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    x = 0;
    for (string s : {"","","","SH","ZH","R"}) {
        float nudge = s.length() > 1 ? 0.1 : 0;
        render_latin(s, MARGIN+x*0.6-nudge, MARGIN+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"","","","S","Z","r"}) {
        if (s.length()) render_skullbat(s, MARGIN+x*0.6+te.width*1.5, MARGIN+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    x = 0;
    for (string s : {"K","G","NG","","","Y"}) {
        float nudge = s.length() > 1 ? 0.1 : 0;
        render_latin(s, MARGIN+x*0.6-nudge, MARGIN+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"k","g","N","","","y"}) {
        if (s.length()) render_skullbat(s, MARGIN+x*0.6+te.width*1.5, MARGIN+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    x = 0;
    for (string s : {"","","","H","",""}) {
        float nudge = s.length() > 1 ? 0.1 : 0;
        render_latin(s, MARGIN+x*0.6-nudge, MARGIN+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"","","","h","",""}) {
        if (s.length()) render_skullbat(s, MARGIN+x*0.6+te.width*1.5, MARGIN+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    render_latin("Vowels", MARGIN, MARGIN+fe.height + y *1.5*fe.height);

    y += 1;
    render_latin("Full words only", MARGIN, MARGIN+fe.height + y *1.5*fe.height);

    y += 1;
    x = 0;
    for (string s : {"A","E","I","O","U"}) {
        float nudge = s.length() > 1 ? 0.1 : 0;
        render_latin(s, MARGIN+x*0.6-nudge, MARGIN+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"A","E","I","O","yu`"}) {
        if (s.length()) render_skullbat(s, MARGIN+x*0.6+te.width*1.5, MARGIN+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    render_latin("Diacritics (optional if unambiguous)", MARGIN, MARGIN+fe.height+ y *1.5*fe.height);
    set_scale(3);

    y += 1;
    render_latin("monopthongs", MARGIN, MARGIN+fe.height+ y *1.5*fe.height);

    int yy = 0;

    y += 1;

    x = 0;
    yy = y;
    for (string s : {"i","ɪ","e","ɛ","æ"}) {
        render_latin("/"+s+"/", MARGIN+x*1.0, MARGIN+fe.height+ yy *1.5*fe.height);
        yy += 1;
    }
    yy = y;
    for (string s : {"i","I","e","E","A"}) {
        if (s.length()) render_vowel_at(s, MARGIN+x*1.0+te.width*2.5, MARGIN+fe.height*2.0/3.0 + yy *1.5*fe.height);
        yy += 1;
    }
    x = 1;
    yy = y;
    for (string s : {"","","ə","","a"}) {
        if (s.length()) render_latin("/"+s+"/", MARGIN+x*1.0, MARGIN+fe.height+ yy *1.5*fe.height);
        yy += 1;
    }
    yy = y;
    for (string s : {"","","@","","a"}) {
        if (s.length()) render_vowel_at(s, MARGIN+x*1.0+te.width*2.5, MARGIN+fe.height*2.0/3.0 + yy *1.5*fe.height);
        yy += 1;
    }
    x = 2;
    yy = y;
    for (string s : {"u","ʊ","o","","ɒ"}) {
        if (s.length()) render_latin("/"+s+"/", MARGIN+x*1.0, MARGIN+fe.height+ yy *1.5*fe.height);
        yy += 1;
    }
    yy = y;
    for (string s : {"u","U","o","","O"}) {
        if (s.length()) render_vowel_at(s, MARGIN+x*1.0+te.width*2.5, MARGIN+fe.height*2.0/3.0 + yy *1.5*fe.height);
        yy += 1;
    }

    y += 5;
    render_latin("dipthongs", MARGIN, MARGIN+fe.height + y *1.5*fe.height);

    y += 1;
    x = 0;
    for (string s : {"ei","ai","oi","au","ou"}) {
        render_latin("/"+s+"/", MARGIN+x*0.7, MARGIN+fe.height+ y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"!e","!a","!o","^a","^o"}) {
        render_vowel_at(s, MARGIN+x*0.7+te.width*2.5, MARGIN+fe.height*2.0/3.0 + y *1.5*fe.height);
        x += 1;
    }

    set_scale(1);
}

void render_back_cover() {
    draw_skull_bat(4, 5.5/2-2+0.125, (PAPER_HEIGHT-3)/2);
}

void render_spine() {
    set_scale(3);
    render_at_inches("Pride and Prejudice", 6.1586, MARGIN/2+0.125);
    render_at_inches("Jane Austen", 6.1586, 4.5+0.125);

    draw_skull_bat(0.5, 6.1586-0.25, PAPER_HEIGHT-MARGIN-0.125);
}

void render_front_cover() {
    float width = 5.5;
    float x = PAPER_WIDTH-width-0.125;
    float y = 0.125;

    cairo_surface_t * art =
        cairo_image_surface_create_from_png("Final Book Cover.png");
    cairo_save(cr);
    int w = cairo_image_surface_get_width(art);
    cairo_translate(cr, x, y);
    cairo_scale(cr, width /w, width /w);
    cairo_set_source_surface(cr, art, 0,0);
    cairo_paint(cr);
    cairo_restore(cr);
    cairo_surface_destroy(art);
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
    //if (nargs != 2) die("filename");

    load_phonetic();

    csurf = cairo_pdf_surface_create(
        "cover.pdf",
        PAPER_WIDTH * POINTS_PER_INCH,
        PAPER_HEIGHT * POINTS_PER_INCH);
    cr = cairo_create(csurf);
    cairo_scale(cr, POINTS_PER_INCH, POINTS_PER_INCH);

    cairo_set_line_width(cr, LINE_WIDTH);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb(cr, 0,0,0);

    /*
    vector<string> lines = load_text(args[1]);
    vector<string> paras;
    string para;
    for (string line : lines) {
        if (line.size()) para += line + " ";
        else {
            paras.push_back(para);
            para.clear();
        }
    }
    paras.push_back(para);
    new_page();
    set_scale(7);
    for (string para : paras) {
        if (para.substr(0,7) == "Chapter") {
            set_scale(1);
            new_page();

            // chapter heading
            set_scale(2);
            render_at_inches(para, MARGIN+step, MARGIN);
            set_scale(1);

            cur_col = 1; //TODO new_column should take a column argument
            new_column();
        }
        else render_columns(para);
    }

    new_page();

    render_key_page();
    */

    render_back_cover();
    render_spine();
    render_front_cover();
    cairo_surface_show_page(csurf);

    cairo_destroy(cr);
    cairo_surface_finish(csurf);
    cairo_surface_destroy(csurf);
    return 0;
}
