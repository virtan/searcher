AM_CPPFLAGS = -I.. -I/opt/local/include $(LOGGER_CPPFLAGS)
AM_CXXFLAGS = $(LOGGER_CXXFLAGS)

check_PROGRAMS = query_parser_test \
		 lexicon_load_test \
		 index_load_test \
		 lexicon_dumper \
		 draft_pass_load_test \
		 web_page_info_load_test \
		 posting_list_dumper \
		 posting_list_skip_list_test \
		 posting_list_speed \
		 long_postings_dumper

query_parser_test_SOURCES = query_parser_test.cc
query_parser_test_LDADD = ../srch/libsearcher.la $(LOGGER_LIBS)

lexicon_load_test_SOURCES = lexicon_load_test.cc
lexicon_load_test_LDADD = ../srch/libsearcher.la $(LOGGER_LIBS)

index_load_test_SOURCES = index_load_test.cc
index_load_test_LDADD = ../srch/libsearcher.la $(LOGGER_LIBS)

lexicon_dumper_SOURCES = lexicon_dumper.cc
lexicon_dumper_LDADD = ../srch/libsearcher.la $(LOGGER_LIBS)

draft_pass_load_test_SOURCES = draft_pass_load_test.cc
draft_pass_load_test_LDADD = ../srch/libsearcher.la $(LOGGER_LIBS)

web_page_info_load_test_SOURCES = web_page_info_load_test.cc
web_page_info_load_test_LDADD = ../srch/libsearcher.la $(LOGGER_LIBS)

posting_list_dumper_SOURCES = posting_list_dumper.cc
posting_list_dumper_LDADD = ../srch/libsearcher.la $(LOGGER_LIBS)

posting_list_speed_SOURCES = posting_list_speed.cc
posting_list_speed_LDADD = ../srch/libsearcher.la $(LOGGER_LIBS)

posting_list_skip_list_test_SOURCES = posting_list_skip_list_test.cc
posting_list_skip_list_test_LDADD = ../srch/libsearcher.la $(LOGGER_LIBS)

long_postings_dumper_SOURCES = long_postings_dumper.cc
long_postings_dumper_LDADD = ../srch/libsearcher.la $(LOGGER_LIBS)
