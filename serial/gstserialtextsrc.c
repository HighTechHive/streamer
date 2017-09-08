/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2017 basultobd <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-serialtextsrc
 *
 * FIXME:Describe serialtextsrc here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m serialtextsrc ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gstserialtextsrc.h"
#include "ADT_SerialPort.h" //own library
#include <gst/gst.h>
#include <stdio.h>
#include <gst/app/gstappsrc.h>
#include <stdlib.h>
#include <string.h>

#define RESET   "\033[0m"
#define RED     "\033[1m\033[31m"   /** Error state */
#define GREEN   "\033[1m\033[32m"   /** Success state, playing state or close state*/
#define YELLOW  "\033[1m\033[33m"   /** Pause state  */
#define BLUE    "\033[1m\033[34m"   /** Ready state */
#define WHITE   "\033[1m\033[37m"   /** Normal text of functions*/
#define MAX_BUFFER_SIZE 32

GST_DEBUG_CATEGORY_STATIC (gst_serialtextsrc_debug);
#define GST_CAT_DEFAULT gst_serialtextsrc_debug

//==============================================================================


/**
 * Exit error flags
 */

#define EXIT_ELEMENT_CREATION_FAILURE  -1 /**< Creation elements failure flag */
#define EXIT_ELEMENT_LINKING_FAILURE   -2 /**< Linking elements failure flag */
#define EXIT_PADS_LINKING_FAILURE      -3 /**< Pads elements failure flag */
#define EXIT_GHOSTPAD_CREATION_FAILURE -4 /**< Ghost pads elements failure flag */
#define EXIT_GET_PAD_FAILURE           -5 /**< Get static pad failure flag */
#define EXIT_SET_GHOSTH_PAD_FAILURE    -6 /**< Set ghosth pad failure flag */

//==============================================================================

/**
 * Parameters
 */

#define DEFAULT_DEVICE                   ((const char *)"/dev/pts/19") /**< ADT Serial port reader default device */
#define DEFAULT_SPEED                    9600 /**< ADT Serial port reader default speed */
#define DEFAULT_SETTINGS                 ((const char *)"8n1") /**< ADT Serial port reader default settings */

enum{
    PROP_0,
    PROP_DEVICE
};

//==============================================================================

/**
 * @brief The capabilities of the inputs and outputs.
 *
 * This part creates a template for all the sink pads
 * that the plugin is going to have
 *
 */
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
                                                                    GST_PAD_SRC,
                                                                    GST_PAD_ALWAYS,
                                                                    GST_STATIC_CAPS_ANY
);

//==============================================================================


#define gst_serialtextsrc_parent_class parent_class
G_DEFINE_TYPE (Gstserialtextsrc, gst_serialtextsrc, GST_TYPE_BIN);

//==============================================================================

/**
 * @brief Create all the internal elements
 *
 * @param mediaDemux The plugin instance
 * @return void
 */
static void createElements(Gstserialtextsrc *serialTextSrc);

/**
 * @brief verify if all elements were cretated
 *
 * @param mediaDemux The plugin instance
 * @return void
 */
static void verifyAllElementsCreated(Gstserialtextsrc *serialTextSrc);

/**
 * @brief Set some elements properties like udpsrc port
 *
 * @param mediaDemux The plugin instance
 * @return void
 */
static void setElementsPropsValues(Gstserialtextsrc *serialTextSrc);

/**
 * @brief Add elements to the main bin
 *
 * @param mediaDemux The plugin instance
 * @return void
 */
static void addElementsToBin(Gstserialtextsrc *serialTextSrc);

/**
 * @brief Creates the audio and video ghost pads from src template
 *
 * @param mediaDemux
 * @return void
 */
static void createPluginGhostPads(Gstserialtextsrc *serialTextSrc);

/**
 * @brief set plugin ghost pads with the las video and audio plugin
 *
 * Link the plugin ghost pads with the text last elements
 * Eg.
 * =============================
 * plugin_app_src -> src = ======== TextSrcGhostPad --- Plugin text flow output --->
 * =============================
 *
 *
 * @param mediaDemux The plugin instance
 * @return void
 */
static void setPluginSrcPads(Gstserialtextsrc *serialTextSrc);

/**
 * @brief appsrc callback that ask for data
 * @param appsrc
 * @param unused_size
 * @param user_data
 */
static void cb_need_data (GstElement *appsrc, guint unused_size, gpointer serialTextsrc);

/**
 * @brief generate the info that is going to be contained in the buffer
 *
 * @param appsrc instance
 * @return
 */
static gboolean feed_buffer(gpointer serialTextsrc);

/**
 * @brief set plugin's properties with new values
 *
 * @param object
 * @param prop_id property id
 * @param value new property value
 * @param pspec
 */
static void gst_serialtextsrc_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);

/**
 * @brief Obtain the values of the plugin's properties
 *
 * @param object
 * @param prop_id property id
 * @param value
 * @param pspec
 */
static void gst_serialtextsrc_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);

//==============================================================================

/**
 * @brief GObject vmethod implementations
 * initialize the mediaMux class
 *
 */
static void gst_serialtextsrc_class_init (GstserialtextsrcClass * klass) {
    printf(WHITE "Bin-plugin  -- serialtext Class Init ---  \n" RESET);
    
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;
    
    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;
    
    gobject_class->set_property = gst_serialtextsrc_set_property;
    gobject_class->get_property = gst_serialtextsrc_get_property;
    
    gst_element_class_set_details_simple(gstelement_class,
                                         "serialtextsrc",
                                         "FIXME:Generic",
                                         "FIXME:Generic Template Element",
                                         "basultobd <<user@hostname.org>>");
    
    /** Install properties*/
    g_object_class_install_property (gobject_class, PROP_DEVICE,
                                     g_param_spec_string ("device", "Device Name",
                                                          "Device of /dev to open" , NULL,
                                                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    
    gst_element_class_add_pad_template (gstelement_class,
                                        gst_static_pad_template_get (&src_factory));
    
}

//==============================================================================

static void gst_serialtextsrc_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec){
    
    Gstserialtextsrc *serialtextsrc = GST_SERIALTEXTSRC(object);
    
    int isInit;
    
    switch (prop_id) {
        case PROP_DEVICE:
            
            strcpy(serialtextsrc->device,g_value_get_string (value));
            char* token = strtok(g_value_get_string (value), ",");
            int propertyNumber = 1;
            int number;
            while (token != NULL)
            {
                if(propertyNumber == 1){
                    serialtextsrc->serialPortStruct.deviceName = token;
                    printf(GREEN "New device: %s \n" RESET , serialtextsrc->serialPortStruct.deviceName);
                    propertyNumber++;
                }else if(propertyNumber == 2){
                    serialtextsrc->serialPortStruct.speed = atoi(token);
                    printf(GREEN "New speed: %d \n" RESET , serialtextsrc->serialPortStruct.speed);
                    propertyNumber++;
                }else{
                    serialtextsrc->serialPortStruct.settings = token;
                    printf(GREEN "New settings: %s \n" RESET , serialtextsrc->serialPortStruct.settings);
                    propertyNumber++;
                }
                
                token = strtok (NULL,",");
            }
            
            ADT_initSerialPort(&serialtextsrc->serialPortStruct);
            
            break;
            
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

//==============================================================================

static void gst_serialtextsrc_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec){
    
    Gstserialtextsrc *serialtextsrc = GST_SERIALTEXTSRC(object);
    
    switch (prop_id) {
        case PROP_DEVICE:
            g_value_set_string (value, serialtextsrc->device);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

//==============================================================================

/**
 * @brief initialize the new element
 *
 *
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 *
 * @param mediaMux The plugin instance
 * @return void
 */

static void gst_serialtextsrc_init (Gstserialtextsrc * serialTextSrc)
{
    printf(WHITE "Bin-plugin  -- serialtext Init ---  \n" RESET);
    
    gboolean isDataSrcPadActivated;
    
    /** Set plugin properties */
    serialTextSrc->serialPortStruct.deviceName = g_strdup(DEFAULT_DEVICE);
    printf(GREEN "Default device name %s \n" RESET, serialTextSrc->serialPortStruct.deviceName);
    serialTextSrc->serialPortStruct.speed = DEFAULT_SPEED;
    printf(GREEN "Default speed %d \n" RESET, serialTextSrc->serialPortStruct.speed);
    serialTextSrc->serialPortStruct.settings = g_strdup(DEFAULT_SETTINGS);
    printf(GREEN "Default speed %s \n" RESET, serialTextSrc->serialPortStruct.settings);
    
    /** Elements  */
    createElements(serialTextSrc);
    verifyAllElementsCreated(serialTextSrc);
    setElementsPropsValues(serialTextSrc);
    
    /** Bin */
    addElementsToBin(serialTextSrc);
    
    /** Pads */
    createPluginGhostPads(serialTextSrc);
    setPluginSrcPads(serialTextSrc);
    
    /** plugin src pads */
    gst_element_add_pad (GST_ELEMENT (serialTextSrc), serialTextSrc->dataSrcPad);
    
    /** Active the pads  */
    isDataSrcPadActivated = gst_pad_set_active (serialTextSrc->dataSrcPad, TRUE);
    if (!isDataSrcPadActivated)
        printf(RED "dataSrcPad: gst_pad_set_active = FALSE" RESET);
    
    /**
     * Set this if the element always outputs data in the
     * exact same format as it receives as input
     * */
    GST_PAD_SET_PROXY_CAPS (serialTextSrc->dataSrcPad);
    
}

//==============================================================================

static void createElements(Gstserialtextsrc *serialTextSrc) {
    
    /**
     * Create all internal elements
     */
    
    /** udp src*/
    serialTextSrc->plugin_app_src = gst_element_factory_make("appsrc", "text-src");
    
    g_signal_connect (serialTextSrc->plugin_app_src, "need-data", G_CALLBACK (cb_need_data), serialTextSrc);
    
}


//==============================================================================

static void verifyAllElementsCreated(Gstserialtextsrc *serialTextSrc){
    
    /**
     * Verify that all the internal plugins are created properly
     */
    
    if(!serialTextSrc->plugin_app_src){
        printf (RED "Appsrc could not be created\n" RESET);
        exit(EXIT_ELEMENT_CREATION_FAILURE);
    }
    
}


//==============================================================================

static void setElementsPropsValues(Gstserialtextsrc *serialTextSrc){
    
    /**
     * Configure the streaming capabilities mediademux
     */
    
    g_object_set (G_OBJECT (serialTextSrc->plugin_app_src),
                  "stream-type", 0, // GST_APP_STREAM_TYPE_STREAM
                  "format", GST_FORMAT_TIME,
                  NULL);
    
    g_object_set (G_OBJECT (serialTextSrc->plugin_app_src), "caps",
                  gst_caps_new_simple ("text/x-raw",
                                       "format", G_TYPE_STRING, "utf8",
                                       NULL),
                  NULL);
    
}

//==============================================================================

static void addElementsToBin(Gstserialtextsrc *serialTextSrc){
    
    /**
    * Add elements to the bin mediademux
    */
    
    gst_bin_add_many(GST_BIN(serialTextSrc) ,
                     GST_ELEMENT(serialTextSrc->plugin_app_src),
                     NULL);
}

//==============================================================================



static void createPluginGhostPads(Gstserialtextsrc *serialTextSrc){
    
    /**
     * Create src ghost pads
     */
    
    serialTextSrc->dataSrcPad = gst_ghost_pad_new_no_target_from_template ("text_src",
                                                                         gst_static_pad_template_get (&src_factory));
    if (!serialTextSrc->dataSrcPad) {
        printf(RED "serial data src ghost pad no created from template \n" RESET);
        exit(EXIT_GHOSTPAD_CREATION_FAILURE);
    }
    
    
}

//==============================================================================

static void setPluginSrcPads(Gstserialtextsrc *serialTextSrc){
    
    /**
     * Link the ghost pads with the last element of the audio and video flow respectively
     */
    
    gboolean setGhostPad_ok; /**< Boolean that stores the function return values*/
    
    /** Get static pads of the created elements*/
    GstPad *dataSrcPad1 = gst_element_get_static_pad (serialTextSrc->plugin_app_src, "src");
    if (dataSrcPad1 == NULL) {
        printf(RED "Fail on get appsrc src pad \n" RESET);
        exit(EXIT_GET_PAD_FAILURE);
    }
    
    /** Set ghost pads with the elements src pads*/
    setGhostPad_ok = gst_ghost_pad_set_target ((GstGhostPad*)serialTextSrc->dataSrcPad, dataSrcPad1);
    if(!setGhostPad_ok){
        printf(RED "Text ghost pad could not be linked with appsrc pad \n" RESET);
        exit(EXIT_SET_GHOSTH_PAD_FAILURE);
    }
    
}

//==============================================================================

static void cb_need_data (GstElement *appsrc, guint unused_size, gpointer serialTextsrc)
{
    //printf("cb_need_data\n");
    feed_buffer(serialTextsrc);
}

//==============================================================================

static gboolean feed_buffer(gpointer serialTextsrc) {
    
    static GstClockTime timestamp = 0;
    GstBuffer *buffer;
    GstFlowReturn ret;
    //char *str;
    int size = 32;
    
    //printf("%s\n", ((Gstserialtextsrc*)serialTextsrc)->serialPortStruct->buffer);
    strcpy(((Gstserialtextsrc*)serialTextsrc)->buff, (char*)((Gstserialtextsrc*)serialTextsrc)->serialPortStruct.buffer);
    buffer = gst_buffer_new_wrapped_full( 0, (gpointer)(((Gstserialtextsrc*)serialTextsrc)->buff), MAX_BUFFER_SIZE,
                                          0, size, NULL, NULL);

    //str = g_strdup_printf ("Hello World!");
    //buffer = gst_buffer_new_wrapped(str, strlen (str) + 1);

    GST_BUFFER_PTS (buffer) = timestamp;
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);

    timestamp += GST_BUFFER_DURATION (buffer);
    g_signal_emit_by_name ((GstAppSrc*)((Gstserialtextsrc*)serialTextsrc)->plugin_app_src, "push-buffer", buffer, &ret);
    
    if (ret != GST_FLOW_OK)
    {
        //g_debug("push buffer returned %d for %d bytes \n", ret, size);
        return FALSE;
    }

    //g_free(buffer);
    return TRUE;
}

//==============================================================================



/**
 *
 * @brief entry point to initialize the plug-in
 *
 * initialize the plug-in itself
 * register the element factories and other features
 */

static gboolean serialtextsrc_init (GstPlugin * serialTextSrc)
{
    printf(WHITE "Bin-plugin  -- Serialtextsrc  --- Init plugin function \n" RESET);
    
    /** Debug category for fltering log messages
    * exchange the string 'Template mediaMux' with your description
    */
    
    GST_DEBUG_CATEGORY_INIT (gst_serialtextsrc_debug, "serialtextsrc",
                             0, "Template serialtextsrc");

    return gst_element_register (serialTextSrc, "serialtextsrc", GST_RANK_NONE,
                                 GST_TYPE_SERIALTEXTSRC);
}

//==============================================================================

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstserialtextsrc"
#endif

/* gstreamer looks for this structure to register serialtextsrcs
 *
 * exchange the string 'Template serialtextsrc' with your serialtextsrc description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    serialtextsrc,
    "Template serialtextsrc",
    serialtextsrc_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
