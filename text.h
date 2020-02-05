#pragma once

#include <memory>
#include <string>
#include <vector>

using namespace std;

struct word_t {
    string value;
    bool emphasis;
    float break_penalty;
};

struct chapter_item_t {
};

struct paragraph_t : chapter_item_t {
    vector<shared_ptr<word_t>> words;
};

struct separator_t : chapter_item_t {
};

struct chapter_t {
    vector<unique_ptr<paragraph_t>> chapter_heading;
    vector<unique_ptr<chapter_item_t>> items;
};

struct text_t {
    vector<unique_ptr<paragraph_t>> front_matter;
    vector<unique_ptr<chapter_t>> chapters;
};

//-----

struct column_t {
    vector<shared_ptr<word_t>> words;
};

struct row_t {
    vector<unique_ptr<column_t>> columns;
};

struct page_t {
    vector<unique_ptr<row_t>> rows;
};

struct section_t {
    vector<unique_ptr<page_t>> pages;
};
