# Note: plugindir is set in configure

##############################################################################
# TODO: change libgsththstreamsrc.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgsththstreamsrc.la 

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

## Plugin 1

# sources used to compile this plug-in
libgsththstreamsrc_la_SOURCES = gsththstreamsrc.c gsththstreamsrc.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgsththstreamsrc_la_CFLAGS = $(GST_CFLAGS)
libgsththstreamsrc_la_LIBADD = $(GST_LIBS)
libgsththstreamsrc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgsththstreamsrc_la_LIBTOOLFLAGS = --tag=disable-static

