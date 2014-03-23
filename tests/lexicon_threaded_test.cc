#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>

#include <srch/lexicon.h>
#include <srch/config_branch.h>
#include <srch/exception.h>

using namespace std;

typedef boost::chrono::duration<long, boost::nano> nanoseconds;


int loop_count;
int thread_num;

void get_terms(const string& path, vector<string>& terms);
void measure(const itim::lexicon<>& lex, const vector<string>& terms, vector<long>& result);

int main(int argc, char** argv) {
    vector<string> terms;
    itim::config_branch cfg("config.json");
    itim::lexicon<> lex(cfg.branch("data.lexicon"));
	if (argc != 4) {
		cout << "not enough arguments supplies" << endl;
		exit(1);
	}
    get_terms(argv[3], terms);
	loop_count = atoi(argv[1]);
	thread_num = atoi(argv[2]);
    boost::thread_group g;
    vector<long> results[thread_num];
    for (int i = 0; i < thread_num; i++) {
        g.add_thread(new boost::thread(measure, boost::ref(lex), boost::ref(terms), boost::ref(results[i])));
    }
    g.join_all();

    for (int i = 0; i < thread_num; i++) {
        for (auto it = results[i].begin(); it != results[i].end(); it++) {
            cout << it - results[i].begin() << " " << (*it) << endl;
        }
    }
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

void measure(const itim::lexicon<>& lex, const vector<string>& terms, vector<long>& result) {
    nanoseconds acc = nanoseconds::zero();
    vector<long> thread_results;
    itim::lexicon_item item;
    volatile uint32_t tmp = 0;

    for (int i = 0; i < loop_count; i++) {
        for (auto it = terms.begin(); it != terms.end(); it++) {
            auto start = boost::chrono::steady_clock::now();
            auto item_iter = lex.find(*it);
            nanoseconds nss = boost::chrono::steady_clock::now() - start;
            acc += nss;
            // prevent optimizations
            if (item_iter != lex.end()) {
                item = item_iter->second;
                tmp += item.df;
            }
        }
        result.push_back(acc.count());
        acc = nanoseconds::zero();
    }
}
