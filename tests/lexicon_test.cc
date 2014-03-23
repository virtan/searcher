#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <boost/chrono.hpp>

#include <srch/lexicon.h>
#include <srch/config_branch.h>
#include <srch/exception.h>

using namespace std;

typedef boost::chrono::duration<long, boost::nano> nanoseconds;

void get_terms(const string& path, vector<string>& terms);
void measure(itim::lexicon<>& lex, vector<string>& terms);

const int loop_count = 100;

int main(int argc, char** argv) {
    vector<string> terms;
    itim::config_branch cfg("config.json");
    itim::lexicon<> lex(cfg.branch("data.lexicon"));

    get_terms(argv[1], terms);
    measure(lex, terms);
}

void get_terms(const string &path, vector<string>& terms) {
    ifstream term_file(path);
    if (!term_file.is_open()) {
        cout << "Error opening the term file" << endl;
        exit(1);
    }
    string term;
    while (!term_file.eof()) {
        getline(term_file, term);
        if (term.length() > 0) terms.push_back(term); 
    }
}

void measure(itim::lexicon<>& lex, vector<string>& terms) {
    nanoseconds acc = nanoseconds::zero();
    for (int i = 0; i < loop_count; i++) {
        for (auto it = terms.begin(); it != terms.end(); it++) {
            auto start = boost::chrono::steady_clock::now();
            lex.find(*it);
            nanoseconds nss = boost::chrono::steady_clock::now() - start;
            acc += nss;
        }
        // The results
        cout << i << " " << acc.count() << endl;
        acc = nanoseconds::zero();
    }
}
