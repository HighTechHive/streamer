# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstserialtextsrc.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstserialtextsrc.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

## Plugin 1

# sources used to compile this plug-in
libgstserialtextsrc_la_SOURCES = gstserialtextsrc.c gstserialtextsrc.h ADT_SerialPort.c ADT_SerialPort.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstserialtextsrc_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS)
libgstserialtextsrc_la_LIBADD = $(GST_BASE_LIBS)
libgstserialtextsrc_la_LDFLAGS = $(GST_LIB_LDFLAGS) $(GST_ALL_LDFLAGS) $(GST_LT_LDFLAGS)
libgstserialtextsrc_la_LIBTOOLFLAGS = --tag=disable-static

