#ifndef ITIM_CONSTANTS_H
#define ITIM_CONSTANTS_H

#include <stddef.h>

namespace itim {

const size_t default_skip_len = 100;
const size_t variable_nbits_factor_docid_delta = 5;
const size_t golomb_factor_draft_rank = 60;
const size_t golomb_factor_skip_size = 90;
const size_t golomb_factor_position_delta = 190;
const size_t golomb_factor_alien_position_delta = 9;
const size_t golomb_factor_skip_offset_delta = 2000;

const size_t search_site_draft_coeff = 1000;
const size_t home_page_draft_coeff = 100;
const size_t draft_score_maximum = (1 << 30) - 11;

const size_t draft_timeout_mcs = 60 * 1000 * 1000; // originally was 60 milliseconds
const size_t num_doc_per_site = 50;
const size_t draft_size_high_level_limit = 2500;
const size_t num_doc_per_site_high_level = 15;

const double search_tree_eps = 0.000001;

const size_t java_max_int = (((size_t) 1) << 30) - 1;

const size_t default_metatimeout = 5000000;
const size_t metatimeout_timeout_delta = 100;
const size_t metatimeout_timeout_draft_delta = 500;
const size_t default_draft_npruning = 15000;

const double weak_idf_limit = 0.09;
const double strong_idf_limit = 0.15;

}

#endif
