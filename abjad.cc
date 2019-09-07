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

/*
void draw_guidelines(cairo_t * cr) {
    cairo_set_line_width(cr, 0.2/POINTS_PER_INCH);
    const double DASH_LEN = 0.2/POINTS_PER_INCH;
    double dashes[] = {DASH_LEN, DASH_LEN};
    cairo_set_dash(cr, dashes, 2, 0);
    cairo_set_source_rgb(cr, 1,1,0);
    for (int ix=0 ; ix<=CARDS_ACROSS ; ix+=1) {
        cairo_move_to(cr, (LEFT_MARGIN + CARD_WIDTH*ix), 0.0);
        cairo_line_to(cr, (LEFT_MARGIN + CARD_WIDTH*ix), PAPER_HEIGHT);
    }
    for (int iy=0 ; iy<=CARDS_DOWN ; iy+=1) {
        cairo_move_to(cr, 0.0, (TOP_MARGIN + CARD_HEIGHT*iy));
        cairo_line_to(cr, PAPER_WIDTH, (TOP_MARGIN + CARD_HEIGHT*iy));
    }
    cairo_stroke(cr);
}

struct seg_t {
    bool newline = false;
    string seg_text;
    bool italic = false;
    
    seg_t(string seg_text);
    seg_t(bool newline, string seg_text);
    seg_t(string seg_text, bool italic);
};

seg_t::seg_t(string seg_text) : seg_text(seg_text) {}
seg_t::seg_t(bool newline, string seg_text) : newline(newline), seg_text(seg_text) {}
seg_t::seg_t(string seg_text, bool italic) : seg_text(seg_text), italic(italic) {}

void write(cairo_t * cr, vector<seg_t> text) {
    cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_font_extents_t fe;
    cairo_font_extents(cr, & fe);
    float line_height = 1.5 * fe.height / POINTS_PER_INCH;

    float x_bearing = 0.0;
    float y_bearing = 0.0;
    float line_width = 0.0;
    vector<float> line_widths = {};
    float block_width = 0.0;
    float block_height = 0.0;
    bool first = true;
    for (auto seg : text) {
        if (seg.italic) cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
        else cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, FONT_SIZE/POINTS_PER_INCH);

        cairo_text_extents_t te;
        cairo_text_extents(cr, seg.seg_text.c_str(), & te);
        if (first) {
            first = false;
            x_bearing = te.x_bearing;
            y_bearing = te.y_bearing;
        }
        if (seg.newline) {
            line_widths.push_back(line_width);
            line_width = 0.0;
        }
        line_width += te.x_advance;
        if (line_width > block_width) block_width = line_width;
    }
    line_widths.push_back(line_width);
    block_height = line_height * line_widths.size();

    cairo_rel_move_to(cr, -x_bearing, -y_bearing);
    cairo_rel_move_to(cr, CARD_WIDTH/2.0 - block_width/2.0,
                          CARD_HEIGHT/2.0 - block_height/2.0);

    int line_ix = 0;
    for (auto seg : text) {
        if (seg.italic) cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
        else cairo_select_font_face(cr, "Segoe UI", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, FONT_SIZE/POINTS_PER_INCH);

        if (seg.newline) {
            cairo_rel_move_to(cr, -line_widths[line_ix], line_height);
            line_ix += 1;
        }

        cairo_text_path(cr, seg.seg_text.c_str());
    }
    cairo_set_source_rgb(cr, 0,0,0);
    cairo_fill(cr);
}

void write_card(cairo_t * cr, int x, int y, vector<seg_t> text) {
    cairo_save(cr);

    float left = (x*CARD_WIDTH + LEFT_MARGIN);
    float top = (y*CARD_HEIGHT + TOP_MARGIN);

    cairo_move_to(cr, (x*CARD_WIDTH + LEFT_MARGIN),
                      (y*CARD_HEIGHT + TOP_MARGIN));

    write(cr, text);

    cairo_restore(cr);
}

vector<seg_t> make_block(string text) {
    vector<seg_t> segs = {};

    bool newline = false;
    while (text.size()) {
        int ix = text.find('\n');
        if (ix == string::npos) {
            segs.push_back(seg_t(newline, text));
            break;
        }
        seg_t seg(newline, text.substr(0, ix));
        segs.push_back(seg);
        text = text.substr(ix+1, text.size());
        newline = true;
    }

    return segs;
}

vector<seg_t> wrap(string text, int line_length=26) {
    istringstream words(text);
    ostringstream wrapped;
    string word;
 
    if (words >> word) {
        wrapped << word;
        size_t space_left = line_length - word.length();
        while (true) {
            if (words.peek() == '\n') {
                if (! (words >> word)) break;
                wrapped << '\n' << word;
                space_left = line_length - word.length();
                continue;
            }
            if (! (words >> word)) break;
            if (space_left < word.length() + 1) {
                wrapped << '\n' << word;
                space_left = line_length - word.length();
            } else {
                wrapped << ' ' << word;
                space_left -= word.length() + 1;
            }
        }
    }
    return make_block(wrapped.str());
}

vector<seg_t> texts[] = {
    // 1..10
    vector<seg_t> {seg_t("Pillage, "), seg_t("then", true), seg_t(" burn.")},
    vector<seg_t> {seg_t("A Sergeant in motion"), seg_t(true, "outranks a Lieutenant"), seg_t(true, "who doesn't know"), seg_t(true, "what's going on.")},
    vector<seg_t> {seg_t("An ordnance technician"), seg_t(true, "at a dead run"), seg_t(true, "outranks everybody.")},
    make_block("Close air support\ncovereth a multitude\nof sins."),
    make_block("Close air support and\nfriendly fire should be\neasier to tell apart."),
    make_block("If violence wasn't\nyour last resort,\nyou failed to resort\nto enough of it."),
    make_block("If the food is good enough\nthe grunts will stop\ncomplaining about the\nincoming fire."),
    make_block("Mockery and derision\nhave their place.\nUsually, it's on the\nfar side of the airlock."),
    make_block("Never turn your back\non an enemy."),
    make_block("Sometimes the only\nway out is through...\nThrough the hull."),

    // 11..20
    make_block("Everything is air-droppable\nat least once."),
    make_block("A soft answer turneth\naway wrath. Once wrath\nis looking the other way,\nshoot it in the head."),
    make_block("Do unto others."),
    make_block("\"Mad Science\" means\nnever stopping to ask\n\"What's the worst thing\nthat could happen?\""),
    make_block("Only you can prevent\nfriendly fire."),
    make_block("Your name is in the\nmouth of others:\nBe sure it has teeth."),
    make_block("The longer everything\ngoes according to plan,\nthe bigger the impending\ndisaster."),
    wrap("If the officers are leading from in front, watch for an attack from the rear.", 27),
    wrap("The world is richer when you turn enemies into friends, but that's not the same as you being richer."),
    wrap("If you're not willing to shell your own position, you're not willing to win."),

    // 21..30
    wrap("Give a man a fish, feed him for a day. Take his fish away and tell him he's lucky just to be alive, and he'll figure out how to catch another one for you to take tomorrow.", 28),
    wrap("If you can see the whites of their eyes, somebody's done something wrong."),
    wrap("The company mess and friendly fire should be easier to tell apart."),
    wrap("Any sufficiently advanced technology is indistinguishable from a big gun.", 24),
    wrap("If a manufacturer's warranty covers the damage you do, you didn't do enough damage.", 23),
    wrap("\"Fire and Forget\" is fine, provided you never actually forget."),
    wrap("Don't be afraid to be the first to resort to violence.", 24),
    wrap("If the price of collateral damage is high enough,\nyou might be able to get paid to bring ammunition home with you."),
    wrap("The enemy of my enemy is my enemy's enemy,\nno more, no less.", 23),
    wrap("A little trust goes a long way. The less you use, the further you'll go."),

    // 31..40
    wrap("Only cheaters prosper."),
    wrap("Anything is amphibious if you can get it back out of the water.", 24),
    wrap("If you're leaving tracks, you're being followed."),
    wrap("If you're leaving scorch- marks, you need a bigger gun."),
    wrap("That which does not kill me has made a tactical error."),
    wrap("When the going gets tough, the tough call for close air support."),
    wrap("There is no \"overkill\". There is only \"open fire\" and \"reload\"."),
    wrap("What's easy for you can still be hard on your clients.", 22),
    wrap("There is a difference between \"spare\" parts and \"extra\" parts.", 24),
    wrap("Not all good news is enemy action.", 25),

    // 41..50
    wrap("\"Do you have a backup?\" means \"I can't fix this.\""),
    wrap("\"They'll never expect this\" means \"I want to try something stupid.\"", 27),
    wrap("If it's stupid and it works, it's still stupid and you're lucky.", 28),
    wrap("If it will blow a hole in the ground, it will double as an entrenching tool."),
    wrap("The size of the combat bonus is inversely proportional to the likelihood of surviving to collect it.", 25),
    wrap("Don't try to save money by conserving ammunition.", 25),
    wrap("Don't expect the enemy to cooperate in the creation of your dream engagement."),
    wrap("If it ain't broke, it hasn't been issued to the infantry.", 28),
    wrap("Every client is one missed payment from becoming a target."),
    wrap("Every target is one bribe away from becoming a client."),

    // 51..60
    wrap("Let them see you sharpen the sword before you fall on it."),
    wrap("The army you've got is never the army you want."),
    wrap("The intel you've got is never the intel you want."),
    wrap("It's only too many troops if you can't pay them."),
    wrap("It's only too many weapons if they're pointing in the wrong direction."),
    wrap("Infantry exists to paint targets for people with real guns."),
    wrap("Artillery exists to launch large chunks of budget at an enemy it cannot actually see."),
    wrap("The pen is mightiest when\nit writes orders for swords.", 28),
    wrap("Two wrongs is probably\nnot going to be enough."),
    wrap("Any weapon's rate of fire is inversely proportional to the number of available targets."),

    // 61..70
    wrap("Don't bring big grenades into small rooms."),
    wrap("Anything labeled\n\"This End Toward Enemy\"\nis dangerous at both ends."),
    wrap("The brass knows how to do it by knowing who can do it.", 24),
    wrap("An ounce of sniper is worth a pound of suppressing fire."),
    wrap("After the toss, be the one with the pin, not the one with the grenade."),
    wrap("Necessity is the mother\nof deception."),
    wrap("If you can't carry cash, carry a weapon."),
    wrap("Negotiating from a position of strength does not mean you shouldn't also negotiate from a position near the exits."),
    wrap("Sometimes rank is a function of firepower."),
    wrap("Failure is not an option. It is mandatory.\nThe option is whether or not to let failure be the last thing you do."),

    // reprints
    wrap("If the food is good enough, the grunts will stop complaining about the incoming fire.", 25),
};
*/

cairo_t * cr;

float unit = 1.0/5.0;

float step = unit/4;
float halfstep = step/2;
float step2 = step*2;
float ribstep = 2*step/3;
float wordstep = step;
float colstep = 3*unit/2;

int cur_col;

float elstep = step * sqrt(2)/2;

float hookrad = 2*halfstep/3;
float dotrad = halfstep/5;

float leftx;
float spinex;
float rightx;
float markx;

float riby;
float starty;
float vowely;

void render_voice_mark(float offset=0.0) {
    cairo_move_to(cr, markx, riby - 3*step/8 + offset);
    cairo_line_to(cr, markx, riby + 3*step/8 + offset);
}

void render_letter(char c) {
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
        cairo_rel_curve_to(cr, elstep/2,-halfstep, elstep,0, 0,halfstep);
        riby += halfstep;
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

void render_vowel_hook() {
    cairo_new_sub_path(cr);
    cairo_arc(cr, spinex-halfstep,riby, halfstep, 2*M_PI,M_PI);
}

void render_vowel(char c) {
    switch (c) {
    case 'i':   // seat
        break;
    case 'I':   // sit
        break;
    case 'e':   // sate
        break;
    case 'E':   // set
        break;
    case 'A':   // sat
        break;
    case 'a':   // sot
        break;
    case 'O':   // sup
        break;
    case 'o':   // so
        break;
    case 'U':   // soot
        break;
    case 'u':   // suit
        break;
    default:
        cout << "unknown vowel: " << c << endl;
        break;
    }
}

void render_i_dipthong(char c) {
    switch (c) {
    case 'e':   // !e ei say
        break;
    case 'a':   // !a ai site
        break;
    case 'o':   // !o oi soy
        break;
    default:
        cout << "unknown i dipthong: " << c << endl;
        break;
    }
}

void render_u_dipthong(char c) {
    switch (c) {
    case 'a':   // ^a au south
        break;
    case 'o':   // ^o ou low
        break;
    default:
        cout << "unknown u dipthong: " << c << endl;
        break;
    }
}

void render_word(string w) {
    riby = starty;

    if (w[0] == '`') {
        render_vowel_hook();
        w = w.substr(1);
        riby += halfstep;
    }

    riby += ribstep;

    bool final_vowel = false;
    if (w.back() == '`') {
        final_vowel = true;
        w.pop_back();
    }

    for (char c : w) {
        render_letter(c);
        riby += ribstep;
    }

    if (final_vowel) {
        render_vowel_hook();
    }

    cairo_move_to(cr, spinex, starty);
    cairo_line_to(cr, spinex, riby);

    if (final_vowel) riby += halfstep;

    cairo_stroke(cr);
}

void render_vowel_word(string w) {
    riby += step;

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

void render_words(vector<string> ws) {
    for (auto w : ws) {
        if (w == "A" || w == "I" || w == "O") render_vowel_word(w);
        else render_word(w);

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

    /*
    spinex = MARGIN + step;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = MARGIN;
    riby = starty;

    render_word("`mzN");
    starty = riby + wordstep;
    render_word("grs");
    starty = riby + wordstep;
    render_word("h`");
    starty = riby + wordstep;
    render_word("swt");
    starty = riby + wordstep;
    render_word("T");
    starty = riby + wordstep;
    render_word("snd");
    starty = riby + wordstep;
    render_word("Tt");
    starty = riby + wordstep;
    render_word("svd");
    starty = riby + wordstep;
    render_vowel_word("A");
    starty = riby + wordstep;
    render_word("rtS");
    starty = riby + wordstep;
    render_word("lk");
    starty = riby + wordstep;
    render_word("m");

    starty = riby + wordstep;
    render_vowel_word("I");
    starty = riby + wordstep;
    render_word("wns");
    starty = riby + wordstep;
    render_word("wz");
    starty = riby + wordstep;
    render_word("lst");

    starty = riby + wordstep;
    render_vowel_word("O");
    starty = riby + wordstep;
    render_word("`ts");
    starty = riby + wordstep;
    render_word("glrs");

    // column

    spinex = MARGIN + step + colstep;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = MARGIN;
    riby = starty;

    render_word("smnN");

    starty = riby + wordstep;
    render_word("`bdZd");

    starty = riby + wordstep;
    render_word("kdl`");

    starty = riby + wordstep;
    render_word("krtr");

    starty = riby + wordstep;
    render_word("wTr");
    starty = riby + wordstep;
    render_word("t");
    starty = riby + wordstep;
    render_word("sfr");
    starty = riby + wordstep;
    render_word("T");
    starty = riby + wordstep;
    render_word("slNs");
    starty = riby + wordstep;
    render_word("`nt");
    starty = riby + wordstep;
    render_word("`roz");
    starty = riby + wordstep;
    render_word("`f");
    starty = riby + wordstep;
    render_word("`^atrdZs");
    starty = riby + wordstep;
    render_word("frtSn");
    */

    cur_col = 0;

    // column

    spinex = MARGIN + step + cur_col++*colstep;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = MARGIN;
    riby = starty;

    render_words(split_words("prd `nt prdZds b` dZn `stn"));

    // column

    spinex = MARGIN + step + cur_col++*colstep;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = MARGIN;
    riby = starty;

    render_words(split_words("`t `s A trT ynfrsl` `knltSt Tt A sNkl mn `n psSn `f A gd frtSn mst b `n wnt `f A wf"));

    // column

    spinex = MARGIN + step + cur_col++*colstep;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = MARGIN;
    riby = starty;

    render_words(split_words("hwfr ltl nn T flNs `r fys `f stS A mn m` b `n hs frst `ntrN A nbrhd Ts trT"));

    // column

    spinex = MARGIN + step + cur_col++*colstep;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = MARGIN;
    riby = starty;

    render_words(split_words("`s s wl fkst `n T mnds `f T srntN fmls Tt h `s knstrt T rtfl prprt` `f sm wn"));

    // column

    spinex = MARGIN + step + cur_col++*colstep;
    leftx = spinex - step;
    rightx = spinex + step;
    markx = rightx + halfstep;

    starty = MARGIN;
    riby = starty;

    render_words(split_words("`r `Tr `f Tr dtrs"));

    /*
    vowely = riby;
    cairo_new_sub_path(cr);
    cairo_arc(cr, markx, vowely, dotrad, 0, 2*M_PI);
    cairo_move_to(cr, markx, vowely);
    cairo_rel_line_to(cr, -halfstep * sqrt(3)/2, -halfstep/2);
    */

    cairo_surface_show_page(csurf);

    cairo_destroy(cr);
    cairo_surface_finish(csurf);
    cairo_surface_destroy(csurf);
    return 0;
}
