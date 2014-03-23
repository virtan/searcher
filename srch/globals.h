#ifndef GLOBALS_H
#define GLOBALS_H

#include <iostream>
#include <locale>
#include <boost/locale.hpp>
#include <boost/program_options.hpp>
#include <eger/logger.h>
#include <srch/itim_base.h>
#include <srch/config_branch.h>

namespace itim {
    
    namespace po = boost::program_options;

    class globals :
        public po::options_description,
        public po::positional_options_description
    {
        public:
            typedef std::vector<string> queries_t;
            typedef string log_profile;
            // { full_debug, soft_debug, profile_only, no_debug, errors_only };

        public:
        globals();
        
        void init(int argc, char **argv, const log_profile &default_lp = "wiphd");

        ~globals() {
            cleanup();
        }

        config_branch &config() { return *cfg; }
        queries_t &queries() { return queries_; }
        static std::locale &get_locale() { return *srch_locale; }

        private:
        void cleanup() {
            delete cfg;
            cfg = 0;
            delete srch_locale;
            srch_locale = 0;
        }

        private:
        eger::instance eger_logger;
        config_branch *cfg;
        queries_t queries_;
        po::variables_map vmap;
        static std::locale *srch_locale;
    };

}

#endif
