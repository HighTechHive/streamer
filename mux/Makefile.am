# Note: plugindir is set in configure

##############################################################################
# TODO: change libgsththstreamsink.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgsththstreamsink.la 

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

## Plugin 1

# sources used to compile this plug-in
libgsththstreamsink_la_SOURCES = gsththstreamsink.c gsththstreamsink.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgsththstreamsink_la_CFLAGS = $(GST_CFLAGS)
libgsththstreamsink_la_LIBADD = $(GST_LIBS)
libgsththstreamsink_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgsththstreamsink_la_LIBTOOLFLAGS = --tag=disable-static
