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

float nan() {
    //TODO this is never checked for, retain?
    return numeric_limits<float>::signaling_NaN();
}

const float POINTS_PER_INCH = 72.0;

struct skullbat_context_t;
struct document_t {
    cairo_surface_t * csurf;

    float paper_width;
    float paper_height;

    float margin;

    skullbat_context_t * pn;   // for rendering page number
    int page_number = 0;

    vector<skullbat_context_t *> contexts;

    document_t(string filename, float newwidth=5.5, float newheight=8.5,
               float newmargin=1.0);

    void subscribe(skullbat_context_t * sb);

    void new_page();
    void mark_page_number();

    void unsubscribe(skullbat_context_t * sb);
    void save_and_close();

};

struct vowel_space_t {
    float before;
    float after;
};

struct skullbat_context_t {
    document_t & document;
    cairo_t * cr;

    float scale;

    float unit;
    float word_width;

    float step;
    float halfstep;
    float ribstep;
    float vowelstep;
    float wordstep;

    float elstep;

    float hookrad;
    float dotrad;
    float voicedlen;

    float vowel_size;

    float leftx;
    float spinex;
    float rightx;
    float markx;

    float starty;
    float riby;

    bool emphasis = false;

    skullbat_context_t(document_t & newdoc, float newscale=1.0)
            : document(newdoc) {
        document.subscribe(this);

        cr = cairo_create(document.csurf);
        cairo_scale(cr, POINTS_PER_INCH, POINTS_PER_INCH);

        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
        cairo_set_source_rgb(cr, 0,0,0);

        set_scale(newscale);
    }

    close() {
        document.unsubscribe(this);
        cairo_destroy(cr);
    }

    virtual void set_scale(float newscale);
    void render_at_inches(string text, float x, float y);
    virtual void handle_new_page();
    void render_voice_mark(float offset);
    float size_consonant(char c);
    void render_consonant(char c);
    vowel_space_t vowel_space(char c);
    float size_vowel_hook();
    void render_vowel_hook();
    void render_monopthong(char c, float x, float y);
    void render_i_dipthong(char c, float x, float y);
    void render_u_dipthong(char c, float x, float y);
    float size_logogram_word(string w);
    void render_logogram_word(string w);
    float size_vowel_word(string w);
    void render_vowel_word(string w);
    float size_digit(char c);
    void render_digit(char c);
    float size_numeral(string w);
    void render_numeral(string w);
    void render_vowel(string v, float x, float y);
    float size_punct(string w);
    void render_punct(string w);
    float size_phonetic_word(string w);
    void render_phonetic_word(string w);
    void render_phonetic_words(vector<string> & ws);
};

using sb_t = skullbat_context_t;

struct skullbat_justification_context_t : skullbat_context_t {
    const float SEP_LINE_WIDTH = 0.01/2;
    const float COLUMN_SPACING = 1.25;

    float rows_per_page;
    float inner_row_height;
    float column_height;

    float colstep;
    int ncols;

    //TODO origin 0 vs origin 1? currently inconsistent
    int cur_col = 0;
    int cur_row = 0;

    skullbat_justification_context_t(document_t & newdoc, float newscale=1.0,
                                     int newrows=2)
            : skullbat_context_t(newdoc, newscale) {
        set_row_count(newrows);
    }

    virtual void set_scale(float newscale);
    void set_row_count(int newrows);
    void new_column(int newcol);
    virtual void handle_new_page();
    void render_row_separator(int row);
    void new_row();
    void render_column_divider();
    void render_column(vector<string> & ps);
    void render_columns(string text);
};

using sbj_t = skullbat_justification_context_t;

document_t::document_t(string filename, float newwidth, float newheight,
                       float newmargin) {
    paper_width = newwidth;
    paper_height = newheight;
    margin = newmargin;

    csurf = cairo_pdf_surface_create(
        filename.c_str(),
        paper_width * POINTS_PER_INCH,
        paper_height * POINTS_PER_INCH);

    pn = new skullbat_context_t(* this);
}

void document_t::new_page() {
    page_number += 1;

    if (page_number > 1) {
        cairo_surface_show_page(csurf);
    }

    for (skullbat_context_t * context : contexts) {
        context->handle_new_page();
    }

    mark_page_number();

    /*
    cairo_t * cr = cairo_create(csurf);
    cairo_scale(cr, POINTS_PER_INCH, POINTS_PER_INCH);
    cairo_move_to(cr, margin, 0);
    cairo_line_to(cr, margin, paper_height);
    cairo_move_to(cr, paper_width-margin, 0);
    cairo_line_to(cr, paper_width-margin, paper_height);
    cairo_set_source_rgb(cr, 1,0,0);
    cairo_set_line_width(cr, 0.01);
    cairo_stroke(cr);
    cairo_destroy(cr);
    */
}

bool even(int n) {
    return n % 2 == 0;
}

void document_t::mark_page_number() {
    string text = to_string(page_number);
    float x = even(page_number) ? margin/2 : paper_width - margin/2;
    float y = margin/2;
    pn->render_at_inches(text, x, y);
}

void document_t::subscribe(skullbat_context_t * sb) {
    contexts.push_back(sb);
}

void document_t::unsubscribe(skullbat_context_t * sb) {
    auto it = begin(contexts);
    while (it != end(contexts)) {
        if (* it == sb) contexts.erase(it);
        else ++it;
    }
}

void document_t::save_and_close() {
    cairo_surface_show_page(csurf);

    cairo_surface_finish(csurf);
    cairo_surface_destroy(csurf);
}

void sb_t::set_scale(float newscale) {
    scale = newscale;

    cairo_set_line_width(cr, scale*0.01/2);

    unit = scale/5.0;
    word_width = unit;

    step = unit/4;
    halfstep = step/2;
    ribstep = 2*step/3;
    vowelstep = ribstep;
    wordstep = step;

    elstep = step * sqrt(2)/2;

    hookrad = 2*halfstep/3;
    dotrad = 2*halfstep/5;
    voicedlen = 2*step/3;

    vowel_size = halfstep + 2*dotrad;
}

void sbj_t::set_scale(float newscale) {
    sb_t::set_scale(newscale);

    float row_width = document.paper_width - document.margin*2;

    ncols = row_width / (word_width * COLUMN_SPACING);
    colstep = row_width / ncols;
}

void sbj_t::set_row_count(int newrows) {
    rows_per_page = newrows;
    inner_row_height = (document.paper_height-document.margin) / rows_per_page;
    column_height = inner_row_height - document.margin;
}

void sbj_t::new_column(int newcol=-1) {
    if (newcol != -1) cur_col = newcol;

    spinex = document.margin + word_width/2 + cur_col*colstep;
    if (newcol == -1) cur_col += 1;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = document.margin + inner_row_height * cur_row;
    riby = starty;
}

void sb_t::handle_new_page() {
}

void sbj_t::handle_new_page() {
    cur_row = 0;
    cur_col = 0;
}

void sbj_t::render_row_separator(int row) {
    cairo_save(cr);
    cairo_set_line_width(cr, SEP_LINE_WIDTH);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_source_rgb(cr, 0.5,0.5,0.5);
    float sepy = document.margin/2 + inner_row_height * row;
    cairo_move_to(cr, document.margin,sepy);
    cairo_line_to(cr, document.paper_width-document.margin,sepy);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void sbj_t::new_row() {
    cur_row += 1;
    cur_col = 0;

    if (cur_row >= rows_per_page) document.new_page();
    else render_row_separator(cur_row);

    new_column();
}

void sb_t::render_voice_mark(float offset=0.0) {
    cairo_move_to(cr, markx, riby - voicedlen/2 + offset);
    cairo_line_to(cr, markx, riby + voicedlen/2 + offset);
}

float sb_t::size_consonant(char c) {
    float size = 0;

    switch (c) {
    case 'b':
    case 'p':
        break;
    case 'v':
    case 'f':
        size += halfstep;
        break;
    case 'D':
    case 'T':
        size += halfstep;
        break;
    case 'd':
    case 't':
        break;
    case 'z':
    case 's':
        size += halfstep;
        break;
    case 'Z':
    case 'S':
        size += halfstep;
        break;
    case 'g':
    case 'k':
        break;
    case 'h':
        size += halfstep;
        break;
    case 'w':
        size += step;
        break;
    case 'l':
    case 'r':
        size += 2*elstep;
        break;
    case 'y':
        size += step;
        break;
    case 'm':
    case 'n':
    case 'N':
        size += 3*step/4;
        break;
    default:
        // unimplemented letter
        size += step;
        break;
    }

    return size;
}

void sb_t::render_consonant(char c) {
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

vowel_space_t sb_t::vowel_space(char c) {
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

float sb_t::size_vowel_hook() {
    return halfstep;
}

void sb_t::render_vowel_hook() {
    cairo_new_sub_path(cr);
    cairo_arc(cr, spinex-halfstep,riby, halfstep, 2*M_PI,M_PI);
    riby += halfstep;
}

void sb_t::render_monopthong(char c, float x, float y) {
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

void sb_t::render_i_dipthong(char c, float x, float y) {
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
        x -= halfstep /2;
        y += halfstep * sqrt(3)/2 /2;
        cairo_move_to(cr, x, y);
        cairo_rel_line_to(cr, halfstep, 0);
        cairo_rel_line_to(cr, -halfstep/2, -halfstep * sqrt(3)/2);
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
        cout << "unknown i dipthong: " << c << endl;
        return;
        break;
    }
}

void sb_t::render_u_dipthong(char c, float x, float y) {
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

float sb_t::size_logogram_word(string w) {
    float size = nan();

    if (w == "&") size = step*2;
    else if (w == "XXX") size = step; // red mark for unknown word

    return size;
}

void sb_t::render_logogram_word(string w) {
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

float sb_t::size_vowel_word(string w) {
    float size = nan();

    if (w == "A") size = halfstep;
    else if (w == "E") size = halfstep;
    else if (w == "I") size = step;
    else if (w == "O") size = step;
    else if (w == "U") size = step;

    return size;
}

void sb_t::render_vowel_word(string w) {
    if (w == "A") {
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex,riby, halfstep, 2*M_PI,M_PI);
        riby += halfstep;
    }
    else if (w == "E") {
        cairo_arc(cr, spinex,riby, halfstep, 2*M_PI,M_PI);
        riby += halfstep;
        float vowely = riby - halfstep;
        render_vowel("i", markx, vowely);
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
    else if (w == "U") {
        riby += halfstep;
        cairo_new_sub_path(cr);
        cairo_arc(cr, spinex,riby, halfstep, 0,2*M_PI);
        riby += halfstep;
        float vowely = riby - halfstep;
        render_vowel("u", markx, vowely);
    }
    else cout << "unknown vowel word: " << w << endl;

    cairo_stroke(cr);
}

float sb_t::size_digit(char c) {
    float size = nan();

    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        size = step;
        break;
    case '#':
        // ordinal ideogram
        size = halfstep;
        break;
    default:
        break;
    }

    return size;
}

void sb_t::render_digit(char c) {
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
        break;
    default:
        cout << "unknown digit: " << c << endl;
        break;
    }
}

float sb_t::size_numeral(string w) {
    float size = 0;

    for (char c : w) {
        if (isalpha(c)) {
            // assume it's an ordinal
            size += size_digit('#');
            break;
        }

        size += size_digit(c);
        size += wordstep/2; //TODO omit this at end of numeral?
    }

    return size;
}

void sb_t::render_numeral(string w) {
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

void sb_t::render_vowel(string v, float x, float y) {
    cairo_path_t * saved_path = cairo_copy_path(cr);
    cairo_new_path(cr);

    if (v[0] == '!') render_i_dipthong(v[1], x, y);
    else if (v[0] == '^') render_u_dipthong(v[1], x, y);
    else render_monopthong(v[0], x, y);

    cairo_new_path(cr);
    cairo_append_path(cr, saved_path);
    cairo_path_destroy(saved_path);
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

float sb_t::size_punct(string w) {
    float size = nan();

    if (w == "-") size = 0;
    else if (w == "--" || w == ";" || w == ":") size = step;
    else if (w == "----") size = wordstep*2;
    else if (w == ",") size = 0;
    else if (w == ".") size = halfstep;
    else if (w == "!") size = 0;
    else if (w == "?") size = 0;
    else if (w == "\"") size = ribstep;
    else if (w == "(") size = 0;
    else if (w == ")") size = 0;
    else if (w == "/") size = 0;   //TODO move this into word object

    return size;
}

void sb_t::render_punct(string w) {
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
        cairo_move_to(cr, spinex-step-halfstep, riby-halfstep);
        cairo_line_to(cr, spinex-step-halfstep, riby);
        cairo_line_to(cr, spinex, riby);
        cairo_stroke(cr);
        riby += halfstep;
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

void sbj_t::render_column_divider() {
    float col_top = (cur_row == 0 ? document.margin : document.margin/2) + cur_row * document.paper_height/rows_per_page;
    float col_bot = col_top + column_height;

    float frac = column_height / 3;

    cairo_save(cr);
    cairo_set_line_width(cr, SEP_LINE_WIDTH);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_source_rgb(cr, 0.5,0.5,0.5);
    cairo_move_to(cr, spinex, col_top+frac);
    cairo_line_to(cr, spinex, col_bot-frac);
    cairo_stroke(cr);
    cairo_restore(cr);
}

float sb_t::size_phonetic_word(string w) {
    float temp_riby = starty;

    if (logogram(w[0])) {
        return size_logogram_word(w);
    }

    // this must come before vowel() because '!' is a "vowel"
    if (isprosody(w[0])) {
        return size_punct(w);
    }

    if (vowel(w[0])) {
        return size_vowel_word(w);
    }

    if (isdigit(w[0])) {
        return size_numeral(w);
    }

    if (w[0] != '`') temp_riby += ribstep/2;

    int ix = 0;
    int lastrib_ix;
    while (ix < w.length()) {
        int nextrib_ix = ix + 1;
        while (nextrib_ix<w.length() && vowel(w[nextrib_ix])) nextrib_ix += 1;

        float prerib_riby = temp_riby;
        if (w[ix] == '`') temp_riby += size_vowel_hook();
        else temp_riby += size_consonant(w[ix]);
        float rib_height = temp_riby - prerib_riby;

        float pregap_riby = temp_riby;

        // do current and next ribs need a gap?
        fills_t fills = consonant_fills(w[ix]);
        if (nextrib_ix<w.length()) {
            fills_t fills2 = consonant_fills(w[nextrib_ix]);
            if (fills.l_bot && fills2.l_top
             || fills.c_bot && fills2.c_top
             || fills.r_bot && fills2.r_top
             ) {
                // gap
                temp_riby += ribstep;
            }
            else {
                // no gap
                //XXX experiment with small gap due to eg "disquiet"
                temp_riby += ribstep/3;
            }
        }
        else {
            // no next rib
            lastrib_ix = ix;
        }

        // size vowel if any
        float vowely_center;
        if (nextrib_ix < w.length() && vowel(w[ix+1])) {
            // there's a vowel then a following rib
            string vtext = w.substr(ix + 1, nextrib_ix - ix - 1);
            vector<string> vs = split_vowels(vtext);

            if (w[ix] == '`') vowely_center = pregap_riby - halfstep/2;
            else if (w[nextrib_ix] == '`') vowely_center = temp_riby + halfstep/2;
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
                    nexty = temp_riby + 0/2 - voicedlen/2;
                }
                else nexty = temp_riby + vowel_space(w[nextrib_ix]).before;

                float vowel_room = nexty - prevy;
                float vowel_needs = vowel_size * vs.size();
                if (vowel_room < vowel_needs) {
                    float skootch = vowel_needs - vowel_room;
                    temp_riby += skootch;
                    nexty += skootch;
                }

                vowely_center = (prevy + nexty) / 2;
            }
        }

        ix = nextrib_ix;
    }

    if (w[lastrib_ix] == '`') temp_riby -= halfstep;
    else temp_riby += ribstep/2;

    if (w[lastrib_ix] == '`') temp_riby += halfstep;

    return temp_riby - starty;
}

void sb_t::render_phonetic_word(string w) {
    riby = starty;

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

            float vowely = vowely_center - vowel_size * (vs.size()-1) / 2.0;
            for (string v : vs) {
                render_vowel(v, markx, vowely);
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

void sb_t::render_phonetic_words(vector<string> & ws) {
    for (auto w : ws) {
        if (w == "/") {
            emphasis = ! emphasis;
            continue;
        }

        render_phonetic_word(w);
        starty = riby + wordstep;
    }
}

map<string, string> pronunciation;

const string STARS = "*****";

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

void sb_t::render_at_inches(string text, float x, float y) {
    vector<string> ws = split_words(text);
    vector<string> ps = phoneticize_words(ws);

    spinex = x;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = y;
    riby = starty;

    render_phonetic_words(ps);
}

void sbj_t::render_column(vector<string> & ps) {
    new_column();
    if (cur_col > ncols) new_row();
    render_phonetic_words(ps);
}

void sbj_t::render_columns(string text) {
    vector<string> ws = split_words(text);
    vector<string> ps = phoneticize_words(ws);

    vector<string> col_ps;
    float col_size = 0;
    for (auto it=ps.begin() ; it!=ps.end() ; ++it) {
        auto p = * it;
        if (p == STARS) {
            render_column(col_ps);
            col_ps.clear();
            col_size = 0;

            render_column_divider();
            continue;
        }

        float word_size = size_phonetic_word(p);
        if (col_size + word_size <= column_height) {
            col_ps.push_back(p);
            col_size += word_size + wordstep;
        }
        else {
            render_column(col_ps);
            col_ps.clear();
            col_size = 0;

            col_ps.push_back(p);
            col_size += word_size + wordstep;
        }
    }

    if (! col_ps.empty()) render_column(col_ps);
}

struct keypage_context_t : skullbat_context_t {
    const float FONT_SIZE = 12.0;

    void draw_skull_bat(float width, float x, float y);
    void render_latin(string text, float x, float y);
    void render_skullbat(string text, float x, float y);
    void render_key_page();

    keypage_context_t(document_t & newdoc, float newscale=1.0)
            : skullbat_context_t(newdoc, newscale) {
    }
};

using kp_t = keypage_context_t;

void kp_t::draw_skull_bat(float width, float x, float y) {
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

void kp_t::render_latin(string text, float x, float y) {
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text.c_str());

    cairo_set_source_rgb(cr, 0,0,0);
    cairo_fill(cr);
}

void kp_t::render_skullbat(string text, float x, float y) {
    spinex = x;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = y;
    riby = starty;

    render_phonetic_word(text);
}

void kp_t::render_key_page() {
    draw_skull_bat(1, document.paper_width/2-0.5, document.margin - 3.0/4.0 /2);

    set_scale(1.5);

    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, FONT_SIZE/POINTS_PER_INCH);

    cairo_font_extents_t fe;
    cairo_text_extents_t te;
    cairo_font_extents(cr, & fe);

    int x = 0;
    int y = 0;

    render_latin("Skullbat Key", document.margin, document.margin+fe.height+ y *1.5*fe.height);

    cairo_select_font_face(cr, "DejaVu Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_font_extents(cr, & fe);
    cairo_text_extents(cr, "W", & te);

    y += 1;
    render_latin("Consonants", document.margin, document.margin+fe.height + y *1.5*fe.height);
    y += 1;
    render_latin("(voicing mark is optional if unambiguous)", document.margin, document.margin+fe.height + y *1.5*fe.height);

    const float SKT = 1.6667;

    y += 1;
    x = 0;
    for (string s : {"P","B","M","F","V","W"}) {
        render_latin(s, document.margin+x*0.6, document.margin+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"p","b","m","f","v","w"}) {
        render_skullbat(s, document.margin+x*0.6+te.width*SKT, document.margin+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    x = 0;
    for (string s : {"","","","TH","DH",""}) {
        float nudge = s.length() > 1 ? 0.1 : 0;
        render_latin(s, document.margin+x*0.6-nudge, document.margin+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"","","","T","D",""}) {
        if (s.length()) render_skullbat(s, document.margin+x*0.6+te.width*SKT, document.margin+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    x = 0;
    for (string s : {"T","D","N","S","Z","L"}) {
        render_latin(s, document.margin+x*0.6, document.margin+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"t","d","n","s","z","l"}) {
        render_skullbat(s, document.margin+x*0.6+te.width*SKT, document.margin+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    x = 0;
    for (string s : {"","","","SH","ZH","R"}) {
        float nudge = s.length() > 1 ? 0.1 : 0;
        render_latin(s, document.margin+x*0.6-nudge, document.margin+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"","","","S","Z","r"}) {
        if (s.length()) render_skullbat(s, document.margin+x*0.6+te.width*SKT, document.margin+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    x = 0;
    for (string s : {"K","G","NG","","","Y"}) {
        float nudge = s.length() > 1 ? 0.1 : 0;
        render_latin(s, document.margin+x*0.6-nudge, document.margin+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"k","g","N","","","y"}) {
        if (s.length()) render_skullbat(s, document.margin+x*0.6+te.width*SKT, document.margin+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    x = 0;
    for (string s : {"","","","H","",""}) {
        float nudge = s.length() > 1 ? 0.1 : 0;
        render_latin(s, document.margin+x*0.6-nudge, document.margin+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"","","","h","",""}) {
        if (s.length()) render_skullbat(s, document.margin+x*0.6+te.width*SKT, document.margin+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    render_latin("Vowels", document.margin, document.margin+fe.height + y *1.5*fe.height);

    y += 1;
    render_latin("Full words only", document.margin, document.margin+fe.height + y *1.5*fe.height);

    y += 1;
    x = 0;
    for (string s : {"A","E","I","O","OO"}) {
        float nudge = s.length() > 1 ? 0.1 : 0;
        render_latin(s, document.margin+x*0.6-nudge, document.margin+fe.height + y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"A","E","I","O","U"}) {
        if (s.length()) render_skullbat(s, document.margin+x*0.6+te.width*SKT, document.margin+fe.height/2 + y *1.5*fe.height);
        x += 1;
    }

    y += 1;
    render_latin("Diacritics (optional if unambiguous)", document.margin, document.margin+fe.height+ y *1.5*fe.height);
    set_scale(3);

    y += 1;
    render_latin("monopthongs", document.margin, document.margin+fe.height+ y *1.5*fe.height);

    int yy = 0;

    y += 1;

    x = 0;
    yy = y;
    for (string s : {"i","ɪ","e","ɛ","æ"}) {
        render_latin("/"+s+"/", document.margin+x*1.0, document.margin+fe.height+ yy *1.5*fe.height);
        yy += 1;
    }
    yy = y;
    for (string s : {"i","I","e","E","A"}) {
        if (s.length()) render_vowel(s, document.margin+x*1.0+te.width*2.5, document.margin+fe.height*2.0/3.0 + yy *1.5*fe.height);
        yy += 1;
    }
    x = 1;
    yy = y;
    for (string s : {"","","ə","","a"}) {
        if (s.length()) render_latin("/"+s+"/", document.margin+x*1.0, document.margin+fe.height+ yy *1.5*fe.height);
        yy += 1;
    }
    yy = y;
    for (string s : {"","","@","","a"}) {
        if (s.length()) render_vowel(s, document.margin+x*1.0+te.width*2.5, document.margin+fe.height*2.0/3.0 + yy *1.5*fe.height);
        yy += 1;
    }
    x = 2;
    yy = y;
    for (string s : {"u","ʊ","o","","ɒ"}) {
        if (s.length()) render_latin("/"+s+"/", document.margin+x*1.0, document.margin+fe.height+ yy *1.5*fe.height);
        yy += 1;
    }
    yy = y;
    for (string s : {"u","U","o","","O"}) {
        if (s.length()) render_vowel(s, document.margin+x*1.0+te.width*2.5, document.margin+fe.height*2.0/3.0 + yy *1.5*fe.height);
        yy += 1;
    }

    y += 5;
    render_latin("dipthongs", document.margin, document.margin+fe.height + y *1.5*fe.height);

    y += 1;
    x = 0;
    for (string s : {"ei","ai","oi","au","ou"}) {
        render_latin("/"+s+"/", document.margin+x*0.7, document.margin+fe.height+ y *1.5*fe.height);
        x += 1;
    }
    x = 0;
    for (string s : {"!e","!a","!o","^a","^o"}) {
        render_vowel(s, document.margin+x*0.7+te.width*2.5, document.margin+fe.height*2.0/3.0 + y *1.5*fe.height);
        x += 1;
    }

    set_scale(1);
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
    if (nargs != 2) die("filename");

    load_phonetic();

    vector<string> lines = load_text(args[1]);

    /*
    // Goblin Market driver
    new_page();
    set_row_count(2);
    for (string line : lines) {
        if (line.empty()) new_column();
        else {
            auto ws = split_words(line);
            auto ps = phoneticize_words(ws);
            render_column(ps);
        }
    }
    */

    // Pride and Prejudice driver
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

    document_t doc("abjad.pdf");
    skullbat_justification_context_t sbj(doc);

    doc.new_page();
    //TODO use a separate skullbat context here
    sbj.set_scale(7);
    sbj.set_row_count(1);
    for (string para : paras) {
        if (para.substr(0,7) == "Chapter") {
            sbj.set_scale(1);
            sbj.set_row_count(2);
            doc.new_page();

            // chapter heading
            sbj.set_scale(2);
            //TODO use a separate skullbat context here
            sbj.render_at_inches(para, doc.margin+sbj.step, doc.margin);
            sbj.set_scale(1);

            sbj.new_column(2);
        }
        else sbj.render_columns(para);
    }

    doc.new_page();

    keypage_context_t kp(doc);
    kp.render_key_page();

    doc.save_and_close();
    return 0;
}
