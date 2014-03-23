ACLOCAL_AMFLAGS = -Im4

PROTOBUF_LIBS = -L/opt/local/lib -lprotobuf

PROTOBUF_ITEMS = bucketinfo.proto \
		 domain2site.proto \
		 parsed_page.proto \
		 wpi.proto \
		 location.proto

PROTOBUF_DIR = proto/searcher

PROTOBUF_SRCS = bucketinfo.pb.cc  domain2site.pb.cc  location.pb.cc  parsed_page.pb.cc  wpi.pb.cc
PROTOBUF_HDRS = bucketinfo.pb.h   domain2site.pb.h   location.pb.h   parsed_page.pb.h   wpi.pb.h

%.pb.cc: $(PROTOBUF_DIR)/%.proto
	$(PROTOC) --proto_path=$(PROTOBUF_DIR) --cpp_out=. $(PROTOBUF_DIR)/$*.proto

BUILT_SOURCES = $(PROTOBUF_SRCS)

AM_CPPFLAGS = -I.. -I/opt/local/include $(BOOST_CPPFLAGS) $(LOGGER_CPPFLAGS)
AM_CXXFLAGS = $(BOOST_CXXFLAGS) $(LOGGER_CXXFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(LOGGER_LDFLAGS)

lib_LTLIBRARIES = libsearcher.la

libsearcher_la_SOURCES = query_parser.cc \
			 globals.cc \
			$(PROTOBUF_SRCS)
libsearcher_la_DEPENDENCIES = archives.h \
			config.h \
			itim_base.h \
			itim_constants.h \
			grouping_codecs.h \
			lexicon_item.h \
			posting.h \
			itim_string.h \
			web_page_info_repos.h \
			bucketinfo_repos.h \
			index.h \
			mapped_region.h \
			posting_iterator.h \
			itim_types.h \
			domain2site_repos.h \
			index_data_iterator.h \
			parsed_page.h \
			posting_list.h \
			config_branch.h \
			itim_exception.h \
			lexicon.h \
			parsed_page_repos.h \
			qatar_codecs.h \
			web_page_info.h \
			query_parser.h \
			$(PROTOBUF_HDRS)
libsearcher_la_LIBADD = -lpthread \
			$(BOOST_THREAD_LIB) \
			$(BOOST_PROGRAM_OPTIONS_LIB) \
			$(BOOST_LOCALE_LIB) \
			$(BOOST_FILESYSTEM_LIB) \
			$(ICU_LIBS) \
			$(PROTOBUF_LIBS) \
			$(LOGGER_LIBS)

bin_PROGRAMS = draft_pass_module

draft_pass_module_SOURCES = draft_pass_module.cc
draft_pass_module_LDADD = libsearcher.la $(LOGGER_LIBS)

clean-local:
	rm -f $(PROTOBUF_HDRS) $(PROTOBUF_SRCS)
