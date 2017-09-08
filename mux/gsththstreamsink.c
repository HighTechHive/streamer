/*********************************************************************************
 * GStreamer                                                                     *
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>             *
 * Copyright (C) 2017 basultobd <<basultobd@gmail.com>>
 *                                                                               *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),    *
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,      *
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:          *
 *
 * The above copyright notice and this permission notice shall be included in    *
 * all copies or substantial portions of the Software.
 *                                                                               *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER           *
 * DEALINGS IN THE SOFTWARE.
 *                                                                               *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in                *
 * which case the following provisions apply instead of the ones
 * mentioned above:                                                              *
 *
 * This library is free software; you can redistribute it and/or                 *
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either                  *
 * version 2 of the License, or (at your option) any later version.
 *                                                                               *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.                              *
 *
 * You should have received a copy of the GNU Library General Public             *
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,                  *
 * Boston, MA 02111-1307, USA.
**********************************************************************************/
/**
 * SECTION:element-hthstreamsink
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 v4l2src ! mux.
 *                alsasrc ! mux.
 *                serialtextsrc ! mux.
 *                hthstreamsink host=x.x.x.x port=xxxx name=mux
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** -- Includes -- */

/** demux plugin header */
#include "gsththstreamsink.h" /**< For all elements of the plugin */

/** gstreamer header file */
#include <gst/gst.h> /**< For all gstreamer functions */

/** stdlib header file */
#include <stdlib.h> /**< For exit()  */

/** stdio header file */
#include <stdio.h> /**< For printf() */

/**
 * @brief Colors for printed messages
 *
 */
#define RESET   "\033[0m"
#define RED     "\033[1m\033[31m"   /** Error state */
#define GREEN   "\033[1m\033[32m"   /** Success state, playing state or close state*/
#define YELLOW  "\033[1m\033[33m"   /** Pause state  */
#define BLUE    "\033[1m\033[34m"   /** Ready state */
#define WHITE   "\033[1m\033[37m"   /** Normal text of functions*/

GST_DEBUG_CATEGORY_STATIC (gst_hthstreamsink_debug);
#define GST_CAT_DEFAULT gst_hthstreamsink_debug

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

#define DEFAULT_HOST                    ((const char *)"127.0.0.1") /**< udpsrc default host */
#define DEFAULT_PORT                    5000 /**< udpsrc default port */

enum{
    PROP_0,
    PROP_HOST,
    PROP_PORT
};

//==============================================================================

/**
 * @brief The capabilities of the inputs and outputs.
 *
 * This part creates a template for all the sink pads
 * that the plugin is going to have
 *
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
                                                                    GST_PAD_SINK,
                                                                    GST_PAD_ALWAYS,
                                                                    GST_STATIC_CAPS_ANY
);

//==============================================================================

#define gst_hthstreamsink_parent_class parent_class
G_DEFINE_TYPE (Gsththstreamsink, gst_hthstreamsink, GST_TYPE_BIN);

//==============================================================================

/**
 * @brief Notify the changes in the state of the bin
 *
 * @param element The plugin instance
 * @param trans Actual transition (state) of the plugin
 * @return GstStateChangeReturn Indicates the state transition of the bin
 */
static GstStateChangeReturn gst_bin_change_state (GstElement * element, GstStateChange trans);

/**
 * @brief Create all the internal elements
 *
 * @param hthstreamsink The plugin instance
 * @return void
 */
static void createElements(Gsththstreamsink *hthstreamsink);

/**
 * @brief verify if all elements were cretated
 *
 * @param hthstreamsink The plugin instance
 * @return void
 */
static void verifyAllElementsCreated(Gsththstreamsink *hthstreamsink);

/**
 * @brief Set some elements properties like udpsink host and port
 *
 * @param hthstreamsink The plugin instance
 * @return void
 */
static void setElementsPropsValues(Gsththstreamsink *hthstreamsink);

/**
 * @brief Add elements to the main bin
 *
 * @param hthstreamsink The plugin instance
 * @return void
 */
static void addElementsToBin(Gsththstreamsink *hthstreamsink);

/**
 * @brief Link the internal plugins
 *
 * In order to follow a logic order is
 * necessary create a correct pads connection
 *
 * @param hthstreamsink The plugin instance
 * @return void
 */
static void linkBinElements(Gsththstreamsink *hthstreamsink);

/**
 * @brief Creates the audio and video ghost pads from src template
 *
 * @param hthstreamsink
 * @return void
 */
static void createPluginGhostPads(Gsththstreamsink *hthstreamsink);

/**
 * @brief set plugin ghost pads with the las video and audio plugin
 *
 * Link the plugin ghost pads with the audio and video first elements
 * Eg.
 * =============================
 * --- Plugin video flow output ---> plugin_time_overlay -> sink
 * =============================
 *
 * ==============================
 * --- Plugin audio flow output ---> plugin_audio_convert -> sink
 * ==============================
 *
 * @param hthstreamsink The plugin instance
 * @return void
 */
static void setPluginSinkPads(Gsththstreamsink *hthstreamsink);

/**
 * @brief set plugin's properties with new values
 *
 * @param object
 * @param prop_id property id
 * @param value new property value
 * @param pspec
 */
static void gst_hthstreamsink_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);

/**
 * @brief Obtain the values of the plugin's properties
 *
 * @param object
 * @param prop_id property id
 * @param value
 * @param pspec
 */
static void gst_hthstreamsink_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);

//==============================================================================

/**
 * @brief GObject vmethod implementations
 * initialize the hthstreamsink class
 *
 */
static void gst_hthstreamsink_class_init (GsththstreamsinkClass * klass){
    
    printf(WHITE "Bin-plugin  -- Serialtextsrc Class Init ---  \n" RESET);
    
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;
    
    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;
    
    gobject_class->set_property = gst_hthstreamsink_set_property;
    gobject_class->get_property = gst_hthstreamsink_get_property;
    
    /** Install properties*/
    g_object_class_install_property (gobject_class, PROP_HOST,
                                     g_param_spec_string ("host", "Host",
                                                          "Address to send packets for" , NULL,
                                                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    g_object_class_install_property (gobject_class, PROP_PORT,
                                     g_param_spec_int ("port", "Port", "The port that receives the packets",
                                                       0, G_MAXUINT16,
                                                       0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    
    gst_element_class_set_details_simple(gstelement_class,
                                         "hthstreamsink",
                                         "FIXME:Generic",
                                         "FIXME:Generic Template Element",
                                         "basultobd <<user@hostname.org>>");
    
    gstelement_class->change_state = GST_DEBUG_FUNCPTR (gst_bin_change_state);
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
 * @param hthstreamsink The plugin instance
 * @return void
 */
static void gst_hthstreamsink_init (Gsththstreamsink *hthstreamsink) {
    
    printf(WHITE "Bin-plugin  -- Serialtextsrc Init ---  \n" RESET);
    
    /** Set plugin properties */
    hthstreamsink->host = g_strdup(DEFAULT_HOST);
    printf(GREEN "Default host %s \n" RESET, hthstreamsink->host);
    hthstreamsink->port = DEFAULT_PORT;
    printf(GREEN "Default port %d \n" RESET, hthstreamsink->port);
    
    gboolean isVideoSinkPadActivated;
    gboolean isAudioSinkPadActivated;
    gboolean isTextSinkPadActivated;
    
    /** Elements  */
    createElements(hthstreamsink);
    verifyAllElementsCreated(hthstreamsink);
    setElementsPropsValues(hthstreamsink);
    
    /** Bin */
    addElementsToBin(hthstreamsink);
    linkBinElements(hthstreamsink);
    
    /** Pads */
    createPluginGhostPads(hthstreamsink);
    setPluginSinkPads(hthstreamsink);
    
    /** plugin sink pads */
    gst_element_add_pad (GST_ELEMENT (hthstreamsink), hthstreamsink->videoSinkPad);
    gst_element_add_pad (GST_ELEMENT (hthstreamsink), hthstreamsink->audioSinkPad);
    gst_element_add_pad (GST_ELEMENT (hthstreamsink), hthstreamsink->textSinkPad);
    
    /** Active the pads  */
    isVideoSinkPadActivated = gst_pad_set_active (hthstreamsink->videoSinkPad, TRUE);
    if (!isVideoSinkPadActivated)
        printf(RED "videoSinkPad: gst_pad_set_active = FALSE" RESET);
    
    isAudioSinkPadActivated = gst_pad_set_active (hthstreamsink->audioSinkPad, TRUE);
    if (!isAudioSinkPadActivated)
        printf(RED "aduioSinkPad: gst_pad_set_active = FALSE" RESET);
    
    isTextSinkPadActivated = gst_pad_set_active (hthstreamsink->textSinkPad, TRUE);
    if (!isTextSinkPadActivated)
        printf(RED "textSinkPad: gst_pad_set_active = FALSE" RESET);
    
    /**
     * Set this if the element always outputs data in the
     * exact same format as it receives as input
     * */
    GST_PAD_SET_PROXY_CAPS (hthstreamsink->videoSinkPad);
    GST_PAD_SET_PROXY_CAPS (hthstreamsink->audioSinkPad);
    GST_PAD_SET_PROXY_CAPS (hthstreamsink->textSinkPad);
    
}

//==============================================================================

static void gst_hthstreamsink_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec){
    
    Gsththstreamsink *hthstreamsink = GST_HTHSTREAMSINK (object);
    
    switch (prop_id) {
        case PROP_HOST:
            
            hthstreamsink->host = g_value_get_string (value);
            g_object_set (hthstreamsink->plugin_udp_sink, "host",  hthstreamsink->host, NULL);
            printf(GREEN "New host: %s \n" RESET , hthstreamsink->host);
            break;
        
        case PROP_PORT:
            
            hthstreamsink->port = g_value_get_int(value);
            g_object_set (hthstreamsink->plugin_udp_sink, "port", hthstreamsink->port, NULL);
            printf(GREEN "New port: %d \n" RESET , hthstreamsink->port);
            break;
        
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

//==============================================================================

static void gst_hthstreamsink_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec){
    
    Gsththstreamsink *hthstreamsink = GST_HTHSTREAMSINK (object);
    
    switch (prop_id) {
        case PROP_HOST:
            g_value_set_string (value, hthstreamsink->host);
            break;
        case PROP_PORT:
            g_value_set_int (value, hthstreamsink->port);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

//==============================================================================

static void createElements(Gsththstreamsink *hthstreamsink) {
    
    /**
     * Create all the internal elements
    */
    
    /** video */
    hthstreamsink->plugin_time_overlay = gst_element_factory_make ("timeoverlay", "time-overlay");
    hthstreamsink->plugin_caps_filter = gst_element_factory_make("capsfilter", "filter-cap");
    hthstreamsink->plugin_video_rate = gst_element_factory_make("videorate", "audio-rate");
    hthstreamsink->plugin_theora_enc = gst_element_factory_make("theoraenc", "video-enc");
    
    /** audio */
    hthstreamsink->plugin_audio_convert = gst_element_factory_make("audioconvert", "audio-convert");
    hthstreamsink->plugin_vorbis_enc = gst_element_factory_make("vorbisenc", "audio-encoder");
    
    /** text */
    hthstreamsink->plugin_identity = gst_element_factory_make("identity", "text-filter");
    
    /** queues */
    hthstreamsink->plugin_video_queue = gst_element_factory_make("queue2", "video-queue");
    hthstreamsink->plugin_audio_queue = gst_element_factory_make("queue2", "audio-queue");
    hthstreamsink->plugin_text_queue = gst_element_factory_make("queue2", "text-queue");
    
    /** mux */
    hthstreamsink->plugin_matroska_mux = gst_element_factory_make("matroskamux", "muxer");
    
    /** udp sender*/
    hthstreamsink->plugin_udp_sink = gst_element_factory_make("udpsink", "udp-sender");
}

//==============================================================================

static void verifyAllElementsCreated(Gsththstreamsink *hthstreamsink){
    
    /**
     * Verify that all the internal plugins are created properly
     */
    
    gboolean notAllElementCreated; /**< Boolean that stores the function return values*/
    
    notAllElementCreated = !hthstreamsink->plugin_time_overlay
        || !hthstreamsink->plugin_caps_filter
        || !hthstreamsink->plugin_video_rate
        || !hthstreamsink->plugin_video_rate
        || !hthstreamsink->plugin_theora_enc
        || !hthstreamsink->plugin_audio_convert
        || !hthstreamsink->plugin_vorbis_enc
        || !hthstreamsink->plugin_identity
        || !hthstreamsink->plugin_audio_queue
        || !hthstreamsink->plugin_video_queue
        || !hthstreamsink->plugin_text_queue
        || !hthstreamsink->plugin_matroska_mux
        || !hthstreamsink->plugin_udp_sink;
    
    if (notAllElementCreated) {
        printf (RED "One element could not be created.\n" RESET);
        exit(EXIT_ELEMENT_CREATION_FAILURE);
    }
}

//==============================================================================

static void setElementsPropsValues(Gsththstreamsink *hthstreamsink){
    
    GstCaps *caps;
    
    /**
     * Configure the streaming capabilities filter
     */
    caps = gst_caps_new_simple("video/x-raw",
                               "width", G_TYPE_INT, 640,
                               "height", G_TYPE_INT, 480,
                               NULL);
    
    g_object_set(G_OBJECT (hthstreamsink->plugin_caps_filter), "caps", caps, NULL);
    
    g_object_set (hthstreamsink->plugin_udp_sink, "host", hthstreamsink->host, NULL);
    g_object_set (hthstreamsink->plugin_udp_sink, "port", hthstreamsink->port, NULL);
    
}

//==============================================================================

static void addElementsToBin(Gsththstreamsink *hthstreamsink){
    
    /**
    * Add elements to the bin filter
    */
    
    gst_bin_add_many(GST_BIN(hthstreamsink) ,
                     GST_ELEMENT(hthstreamsink->plugin_time_overlay),
                     GST_ELEMENT(hthstreamsink->plugin_caps_filter),
                     GST_ELEMENT(hthstreamsink->plugin_video_rate),
                     GST_ELEMENT(hthstreamsink->plugin_theora_enc),
                     GST_ELEMENT(hthstreamsink->plugin_audio_convert),
                     GST_ELEMENT(hthstreamsink->plugin_vorbis_enc),
                     GST_ELEMENT(hthstreamsink->plugin_identity),
                     GST_ELEMENT(hthstreamsink->plugin_audio_queue),
                     GST_ELEMENT(hthstreamsink->plugin_video_queue),
                     GST_ELEMENT(hthstreamsink->plugin_text_queue),
                     GST_ELEMENT(hthstreamsink->plugin_matroska_mux),
                     GST_ELEMENT(hthstreamsink->plugin_udp_sink),
                     NULL);
}

//==============================================================================

static void linkBinElements(Gsththstreamsink *hthstreamsink){
    
    /**
    *
    * Link the audio elements and
    * video elements respectively
    *
    */
    
    GstPad *sink_videoPad;
    GstPad *src_videoPad;
    
    GstPad *sink_audioPad;
    GstPad *src_audioPad;
    
    GstPad *sink_textPad;
    GstPad *src_textPad;
    
    gboolean link_ok; /**< Boolean that stores the function return values*/
    GstPadLinkReturn audioPadLink_ok; /**< Stores the function return values with a specific format*/
    GstPadLinkReturn videoPadLink_ok; /**< Stores the function return values with a specific format*/
    GstPadLinkReturn textPadLink_ok; /**< Stores the function return values with a specific format*/
    
    /** Video elements linking */
    link_ok = gst_element_link_many(hthstreamsink->plugin_time_overlay,
                                    hthstreamsink->plugin_caps_filter,
                                    hthstreamsink->plugin_video_rate,
                                    hthstreamsink->plugin_theora_enc,
                                    hthstreamsink->plugin_video_queue,
                                    NULL);
    if (!link_ok){
        printf(RED "Video stream elements linking fail" RESET);
        exit(EXIT_ELEMENT_LINKING_FAILURE);
    }

//    gst_element_link_pads(hthstreamsink->plugin_time_overlay, "src", hthstreamsink->plugin_caps_filter, "sink");
//    gst_element_link_pads(hthstreamsink->plugin_caps_filter, "src", hthstreamsink->plugin_video_rate, "sink");
//    gst_element_link_pads(hthstreamsink->plugin_video_rate, "src", hthstreamsink->plugin_theora_enc, "sink");
//    gst_element_link_pads(hthstreamsink->plugin_theora_enc, "src", hthstreamsink->plugin_video_queue, "sink");
    
    /** link the video queue with muxer*/
    src_videoPad = gst_element_get_static_pad(hthstreamsink->plugin_video_queue, "src");
    sink_videoPad = gst_element_get_request_pad(hthstreamsink->plugin_matroska_mux, "video_%u");
    videoPadLink_ok = gst_pad_link(src_videoPad, sink_videoPad);
    
    if (videoPadLink_ok != GST_PAD_LINK_OK){
        printf(RED "New video sink request pad linking fails" RESET);
        exit(EXIT_PADS_LINKING_FAILURE);
    }
    
    /** Audio elements linking */
    link_ok = gst_element_link_many(hthstreamsink->plugin_audio_convert,
                                    hthstreamsink->plugin_vorbis_enc,
                                    hthstreamsink->plugin_audio_queue,
                                    NULL);
    if (!link_ok){
        printf(RED "Audio stream elements linking fail" RESET);
        exit(EXIT_ELEMENT_LINKING_FAILURE);
    }

//    gst_element_link_pads(hthstreamsink->plugin_audio_convert, "src", hthstreamsink->plugin_vorbis_enc, "sink");
//    gst_element_link_pads(hthstreamsink->plugin_vorbis_enc, "src", hthstreamsink->plugin_audio_queue, "sink");
    
    /** link the audio queue with muxer*/
    src_audioPad = gst_element_get_static_pad(hthstreamsink->plugin_audio_queue, "src");
    sink_audioPad = gst_element_get_request_pad(hthstreamsink->plugin_matroska_mux, "audio_%u");
    audioPadLink_ok = gst_pad_link(src_audioPad, sink_audioPad);
    
    if (audioPadLink_ok != GST_PAD_LINK_OK){
        printf(RED "New audio sink request pad linking fails" RESET);
        exit(EXIT_PADS_LINKING_FAILURE);
    }
    
    /** Text elements linking */
    link_ok = gst_element_link_many(hthstreamsink->plugin_identity,
                                    hthstreamsink->plugin_text_queue,
                                    NULL);
    if (!link_ok){
        printf(RED "Text stream elements linking fail" RESET);
        exit(EXIT_ELEMENT_LINKING_FAILURE);
    }

//    gst_element_link_pads(hthstreamsink->plugin_audio_convert, "src", hthstreamsink->plugin_vorbis_enc, "sink");
//    gst_element_link_pads(hthstreamsink->plugin_vorbis_enc, "src", hthstreamsink->plugin_audio_queue, "sink");
    
    /** link the audio queue with muxer*/
    src_textPad = gst_element_get_static_pad(hthstreamsink->plugin_text_queue, "src");
    sink_textPad = gst_element_get_request_pad(hthstreamsink->plugin_matroska_mux, "subtitle_%u");
    textPadLink_ok = gst_pad_link(src_textPad, sink_textPad);
    
    if (textPadLink_ok != GST_PAD_LINK_OK){
        printf(RED "New text sink request pad linking fails" RESET);
        exit(EXIT_PADS_LINKING_FAILURE);
    }
    
    /** link matroska mux and udp sink*/
    link_ok = gst_element_link(hthstreamsink->plugin_matroska_mux, hthstreamsink->plugin_udp_sink);
    if (!link_ok){
        printf(RED "UDP sink fail linking pads with matroska muxer" RESET);
        exit(EXIT_ELEMENT_LINKING_FAILURE);
    }
    
    
}

//==============================================================================

static void createPluginGhostPads(Gsththstreamsink *hthstreamsink){
    
    /**
     * Create sink ghost pads
     */
    
    hthstreamsink->videoSinkPad = gst_ghost_pad_new_no_target_from_template ("video_sink",
                                                                            gst_static_pad_template_get (&sink_factory));
    if (!hthstreamsink->videoSinkPad) {
        printf(RED "filter videoSinkPad ghost pad no created from template \n" RESET);
        exit(EXIT_GHOSTPAD_CREATION_FAILURE);
    }
    
    hthstreamsink->audioSinkPad = gst_ghost_pad_new_no_target_from_template ("audio_sink",
                                                                            gst_static_pad_template_get (&sink_factory));
    if (!hthstreamsink->audioSinkPad) {
        printf(RED "filter audioSinkPad ghost pad no created from template \n" RESET);
        exit(EXIT_GHOSTPAD_CREATION_FAILURE);
    }
    
    hthstreamsink->textSinkPad = gst_ghost_pad_new_no_target_from_template ("text_sink",
                                                                           gst_static_pad_template_get (&sink_factory));
    if (!hthstreamsink->textSinkPad) {
        printf(RED "filter textSinkPad ghost pad no created from template \n" RESET);
        exit(EXIT_GHOSTPAD_CREATION_FAILURE);
    }
    
}

//==============================================================================

static void setPluginSinkPads(Gsththstreamsink *hthstreamsink){
    
    /**
     * Link the ghost pads with the last element of the audio and video flow respectively
     */
    
    gboolean setGhostPad_ok; /**< Boolean that stores the function return values*/
    
    GstPad *videoSinkPad1 = gst_element_get_static_pad (hthstreamsink->plugin_time_overlay, "video_sink");
    if (videoSinkPad1 == NULL) {
        printf(RED "Fail on get video sink pad of element \n" RESET);
        exit(EXIT_GET_PAD_FAILURE);
    }
    
    GstPad *audioSinkPad1 = gst_element_get_static_pad (hthstreamsink->plugin_audio_convert, "sink");
    if(audioSinkPad1 == NULL ){// || audioSrcPad1 == NULL){
        printf(RED "Fail on get audio sink pad of element \n" RESET);
        exit(EXIT_GET_PAD_FAILURE);
    }
    
    GstPad *textSinkPad1 = gst_element_get_static_pad (hthstreamsink->plugin_identity, "sink");
    if(textSinkPad1 == NULL ){// || audioSrcPad1 == NULL){
        printf(RED "Fail on get text sink pad of element \n" RESET);
        exit(EXIT_GET_PAD_FAILURE);
    }
    
    setGhostPad_ok = gst_ghost_pad_set_target ((GstGhostPad*)hthstreamsink->videoSinkPad, videoSinkPad1);
    if(!setGhostPad_ok){
        printf(RED "video ghost pad could not be linked with element sink pad \n" RESET);
        exit(EXIT_SET_GHOSTH_PAD_FAILURE);
    }
    
    setGhostPad_ok = gst_ghost_pad_set_target ((GstGhostPad*)hthstreamsink->audioSinkPad, audioSinkPad1);
    if(!setGhostPad_ok){
        printf(RED "audio ghost pad could not be linked with element sink pad \n" RESET);
        exit(EXIT_SET_GHOSTH_PAD_FAILURE);
    }
    
    setGhostPad_ok = gst_ghost_pad_set_target ((GstGhostPad*)hthstreamsink->textSinkPad, textSinkPad1);
    if(!setGhostPad_ok){
        printf(RED "text ghost pad could not be linked with element sink pad \n" RESET);
        exit(EXIT_SET_GHOSTH_PAD_FAILURE);
    }
    
}

//==============================================================================

static GstStateChangeReturn gst_bin_change_state (GstElement *element, GstStateChange trans)
{
    //GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
    
    switch (trans)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
            printf(BLUE "GST_STATE_CHANGE_NULL_TO_READY\n" RESET);
            break;
        
        case GST_STATE_CHANGE_READY_TO_PAUSED:
            printf(YELLOW "GST_STATE_CHANGE_READY_TO_PAUSED\n" RESET);
            break;
        
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
            printf(GREEN "GST_STATE_CHANGE_PAUSED_TO_PLAYING\n" RESET);
            break;
        
        case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
            printf(GREEN "GST_STATE_CHANGE_PLAYING_TO_PAUSED\n" RESET);
        
        case GST_STATE_CHANGE_PAUSED_TO_READY:
            printf(YELLOW "GST_STATE_CHANGE_PAUSED_TO_READY\n" RESET);
            break;
        
        case GST_STATE_CHANGE_READY_TO_NULL:
            printf(BLUE "GST_STATE_CHANGE_READY_TO_NULL\n" RESET);
            break;
        
        default:
            break;
    }
    
    return GST_ELEMENT_CLASS (parent_class)->change_state (element, trans);
}


//==============================================================================

/**
 *
 * @brief entry point to initialize the plug-in
 *
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean hthstreamsink_init (GstPlugin * hthstreamsink){
    
    printf(WHITE "Bin-plugin  -- Serialtextsrc --- Init plugin function \n" RESET);
    
    /** Debug category for fltering log messages
    * exchange the string 'Template hthstreamsink' with your description
    */
    
    GST_DEBUG_CATEGORY_INIT (gst_hthstreamsink_debug, "hthstreamsink",0, "Template hthstreamsink");
    
    return gst_element_register (hthstreamsink, "hthstreamsink", GST_RANK_NONE, GST_TYPE_HTHSTREAMSINK);
}

//==============================================================================

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirsththstreamsink"
#endif

/* gstreamer looks for this structure to register hthstreamsinks
 *
 * exchange the string 'Template hthstreamsink' with your hthstreamsink description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    hthstreamsink,
"Template hthstreamsink",
hthstreamsink_init,
VERSION,
"LGPL",
"GStreamer",
"http://gstreamer.net/"
)
