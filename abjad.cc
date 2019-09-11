#include <cmath>
#include <iostream>
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

const float PAPER_WIDTH = 8.5;
const float PAPER_HEIGHT = 11;

const float MARGIN = 1.0;

const float ROWS_PER_PAGE = 2;

const float COLUMN_HEIGHT = (11 - MARGIN) / ROWS_PER_PAGE - MARGIN;

cairo_t * cr;

float unit = 1.0/5.0;

float step = unit/4;
float halfstep = step/2;
float step2 = step*2;
float ribstep = 2*step/3;
float vowelstep = ribstep;
float wordstep = step;
float colstep = 3*unit/2;

int cur_col;

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

void new_column() {
    spinex = MARGIN + step + cur_col++*colstep;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = MARGIN;
    riby = starty;
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
        render_voice_mark();
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
        render_voice_mark();
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
        return {false, true, false, true, true, true}; // ??? c_bot=?
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

void render_vowel_hook() {
    cairo_new_sub_path(cr);
    cairo_arc(cr, spinex-halfstep,riby, halfstep, 2*M_PI,M_PI);
    riby += halfstep;
}

void render_monopthong(char c, float vowelx) {
    switch (c) {
    case 'i':   // seat
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
        vowely += vowelstep;
        break;
    case 'I':   // sit
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, -halfstep/2);
        vowely += vowelstep;
        break;
    case 'e':   // sate
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, -halfstep, 0);
        vowely += vowelstep;
        break;
    case 'E':   // set
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, +halfstep/2);
        vowely += vowelstep;
        break;
    case 'A':   // sat
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, -halfstep/2, halfstep * sqrt(3)/2);
        vowely += vowelstep;
        break;
    case 'a':   // sot
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, 0, halfstep);
        vowely += vowelstep;
        break;
    case 'O':   // sup
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, halfstep/2, halfstep * sqrt(3)/2);
        vowely += vowelstep;
        break;
    case 'o':   // so
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, halfstep, 0);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad*2, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_restore(cr);

        vowely += vowelstep;
        break;
    case 'U':   // soot
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, halfstep * sqrt(3)/2, -halfstep/2);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad*2, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_restore(cr);

        vowely += vowelstep;
        break;
    case 'u':   // suit
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, halfstep/2, -halfstep * sqrt(3)/2);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad*2, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_restore(cr);

        vowely += vowelstep;
        break;
    default:
        cout << "unknown vowel: " << c << endl;
        break;
    }
}

void render_i_dipthong(char c, float vowelx) {
    switch (c) {
    case 'e':   // !e ei say
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, -halfstep, 0);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
        vowely += vowelstep;
        break;
    case 'a':   // !a ai site
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, 0, halfstep);
        cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, -halfstep/2);
        vowely += vowelstep;
        break;
    case 'o':   // !o oi soy
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, halfstep, 0);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
        vowely += vowelstep;
        break;
    default:
        cout << "unknown i dipthong: " << c << endl;
        break;
    }
}

void render_u_dipthong(char c, float vowelx) {
    switch (c) {
    case 'a':   // ^a au south
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, 0, halfstep);
        cairo_rel_line_to(cr, halfstep * sqrt(3)/2, -halfstep/2);
        vowely += vowelstep;
        break;
    case 'o':   // ^o ou low
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, halfstep, 0);
        cairo_rel_line_to(cr, halfstep/2, -halfstep * sqrt(3)/2);

        cairo_save(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad*2, 0, 2*M_PI);
        cairo_stroke(cr);
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_fill(cr);
        cairo_restore(cr);

        vowely += vowelstep;
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
    else cout << "unknown vowel word: " << w;

    cairo_stroke(cr);
}

void render_word(string w) {
    riby = starty;

    if (vowel(w[0])) {
        render_vowel_word(w);
        return;
    }

    if (w[0] != '`') riby += ribstep/2;

    int ix = 0;
    int nextrib_ix;
    while (ix < w.length()) {
        nextrib_ix = ix + 1;
        while (nextrib_ix<w.length() && vowel(w[nextrib_ix])) nextrib_ix += 1;

        if (w[ix] == '`') render_vowel_hook();
        else render_consonant(w[ix]);
        fills_t fills = consonant_fills(w[ix]);

        float prev_riby = riby;

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
            }
        }
        else {
            // no next rib
        }

        vowely = (prev_riby + riby) / 2;
        for (int vix=ix+1 ; vix<nextrib_ix ; vix+=1) {
            float vowelx = markx;
            if (w[vix] == '!') render_i_dipthong(w[++vix], vowelx);
            else if (w[vix] == '^') render_u_dipthong(w[++vix], vowelx);
            else render_monopthong(w[vix], vowelx);
        }

        ix = nextrib_ix;
    }

    if (w.back() == '`') riby -= halfstep;
    else riby += ribstep/2;

    cairo_move_to(cr, spinex, starty);
    cairo_line_to(cr, spinex, riby);
    cairo_stroke(cr);
}

void render_words(vector<string> ws) {
    for (auto w : ws) {
        render_word(w);
        starty = riby + wordstep;
    }
}

vector<string> split_words(string text) {
    istringstream iss(text);
    vector<string> ws;
    string w;
    while (iss >> w) {
        ws.push_back(w);
    }

    return ws;
}

int main(int nargs, char * args[])
{
    cairo_surface_t * csurf = cairo_pdf_surface_create(
        "abjad.pdf",
        PAPER_WIDTH * POINTS_PER_INCH,
        PAPER_HEIGHT * POINTS_PER_INCH);
    cr = cairo_create(csurf);
    cairo_scale(cr, POINTS_PER_INCH, POINTS_PER_INCH);

    cairo_set_line_width(cr, 0.01);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb(cr, 0,0,0);

    cur_col = 0;

    new_column();
    render_words(split_words("pr!ad `nt prEdZds b` dZen `astn"));

    new_column();
    render_words(split_words("`t `s A trT yunfrsl` `knltSt Tt A sNkl mn `n psSn `f A gd frtSn mst b `n wnt `f A w!af"));

    new_column();
    render_words(split_words("h^awfr ltl n^on T flNs `r vys `f stS A mn m` b `an hs frst `EntrN A nbrhd Ts trT"));

    new_column();
    render_words(split_words("`s s wl fkst `n T mnds `f T srntN fmls Tt h `s knstrt T r!atfUl prprt` `f sm wn"));

    new_column();
    render_words(split_words("`r `Tr `f Tr dtrs"));

    cairo_surface_show_page(csurf);

    cairo_destroy(cr);
    cairo_surface_finish(csurf);
    cairo_surface_destroy(csurf);
    return 0;
}
