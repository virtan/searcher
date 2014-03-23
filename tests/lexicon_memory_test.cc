#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <malloc.h>

#include <srch/lexicon.h>
#include <srch/config_branch.h>

using namespace std;

int main(int argc, char** argv) {
    itim::config_branch cfg("config.json");
    struct mallinfo info = mallinfo();
    cout << info.uordblks / 1024*1024 << "Mb" <<  endl;
    itim::lexicon<> lex(cfg.branch("data.lexicon"));
    info = mallinfo();
    cout << info.uordblks / 1024*1024 << "Mb" << endl;
}
