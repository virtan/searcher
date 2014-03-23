#include <srch/globals.h>

std::locale *itim::globals::srch_locale = 0;

itim::globals::globals() : po::options_description("Allowed options"), cfg(0) {}

void itim::globals::init(int argc, char **argv, const log_profile &dlp)
{
    try {
        string locale_str;
        string config_path;
        string log_profile;
        string async_log;
        po::options_description::add_options()
            ("help", "this help message")
            ("config", po::value<string>(&config_path)->default_value("config.json"),
             "specify path to config file")
            ("locale", po::value<string>(&locale_str)->default_value("vi_VN.UTF-8"),
             "used locale")
            ("asynclog", po::value<string>(&async_log)->default_value("on"),
             "asynchronous or synchronous logger")
            ("log", po::value<string>(&log_profile)->default_value(dlp),
             "set of turned on log streams: w(arning) i(nfo) p(rofile) d(ebug) (debug_)h(ard) (debug_)m(are)")
            ("query", po::value<std::vector<string> >(),
             "query string to searcher")
            ;
        po::positional_options_description::add("query", -1);
        po::store(po::command_line_parser(argc, argv).options(*this).positional(*this).run(), vmap);
        po::notify(vmap);

        if(vmap.count("help")) {
            std::cout << (po::options_description&) *this << std::endl;
            exit(1);
        }

        boost::locale::generator gen;
        srch_locale = new std::locale(gen(locale_str));

        eger_logger[(size_t) eger::level_critical] = "stderr";
        eger_logger[(size_t) eger::level_error] = "stderr";
        eger_logger[(size_t) eger::level_warning] = log_profile.find('w') != string::npos ? "stderr" : "";
        eger_logger[(size_t) eger::level_info] = log_profile.find('i') != string::npos ? "stderr" : "";
        eger_logger[(size_t) eger::level_profile] = log_profile.find('p') != string::npos ? "stderr" : "";
        eger_logger[(size_t) eger::level_debug] = log_profile.find('d') != string::npos ? "stderr" : "";
        eger_logger[(size_t) eger::level_debug_hard] = log_profile.find('h') != string::npos ? "stderr" : "";
        eger_logger[(size_t) eger::level_debug_mare] = log_profile.find('m') != string::npos ? "stderr" : "";
        async_log == "on" ?  eger_logger.start_writer() : eger_logger.start_sync_writer();

        cfg = new config_branch(config_path);

        /*
        size_t buckets = cfg->branches("index_buckets.bucket").size();
        size_t *ptr = (size_t*) malloc(buckets * 4 * 1024 * 1024 * 1024);
        if(!ptr) {
            log_error("Check memory allocation ( malloc(" << (buckets * 4) << "Gb) ) unsuccessful");
        } else {
            log_debug("Preallocating " << (buckets * 4) << "Gb for process");
            for(size_t i = 0; i < (buckets * 4 * 1024 * 1024 * 1024) / sizeof(size_t); ++i) ptr[i] = i;
            log_debug_hard("Preallocation done");
            free(ptr);
        }
        */

        if(vmap.count("query"))
            queries_ = vmap["query"].as<queries_t>();

    } catch(exception &e) {
        cleanup();
        throw exception(e.what()) << where(__PRETTY_FUNCTION__);
    } catch(std::exception &e) {
        cleanup();
        throw exception(e.what()) << where(__PRETTY_FUNCTION__);
    }
}

