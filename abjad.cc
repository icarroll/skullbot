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

const float PAPER_WIDTH = 5.5;
const float PAPER_HEIGHT = 8.5;

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
        //x -= halfstep * sqrt(3)/2 /2;
        //y -= halfstep/2 /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, -halfstep, 0);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
        break;
    case 'a':   // !a ai site
        //x 
        //y -= halfstep /2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, 0, halfstep);
        cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, -halfstep/2);
        break;
    case 'o':   // !o oi soy
        cairo_new_sub_path(cr);
        cairo_arc(cr, vowelx, vowely, dotrad, 0, 2*M_PI);
        cairo_move_to(cr, vowelx, vowely);
        cairo_rel_line_to(cr, halfstep, 0);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
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

void render_column(string text) {
    new_column();
    render_words(split_words(text));
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

    render_column("pr!ad `nt prEdZds b` dZen `astn");

    render_column("`t `s A trT yunfrsl` `knltSt Tt A sNkl mn `n psSn `f A gd frtSn mst b `n wnt `f A w!af");

    render_column("h^awfr ltl n^on T flNs `r vys `f stS A mn m` b `an hs frst `EntrN A nbrhd Ts trT");

    render_column("`s s wl fkst `n T mnds `f T srntN fmls Tt h `s knstrt T r!atfUl prprt` `f sm wn");

    render_column("`r `Tr `f Tr dtrs");

    cairo_surface_show_page(csurf);

    cairo_destroy(cr);
    cairo_surface_finish(csurf);
    cairo_surface_destroy(csurf);
    return 0;
}
