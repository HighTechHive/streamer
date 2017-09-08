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
 * SECTION:element-mediamux
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 mediademux name=demux demux. ! alsasink sync=false demux. ! xvimagesink sync=false
 * ]|
 * </refsect2>
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/** -- Includes -- */

/** demux plugin header */
#include "gstmediademux.h" /**< For all elements of the plugin */

/** gstreamer header file */
#include <gst/gst.h> /**< For all gstreamer functions */

/** stdlib header file */
#include <stdlib.h> /**<  */

/** stdio header file */
#include <stdio.h> /**< For printf() */

/** string header file */
#include <string.h> /**< For strncmp() */

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

GST_DEBUG_CATEGORY_STATIC (gst_mediademux_debug);
#define GST_CAT_DEFAULT gst_mediademux_debug

//==============================================================================
/**
 * Udp src plugin default port
 */

#define DEFAULT_UDP_PORT 5000

//==============================================================================

/**
 * Definition of pad added function
 * constansts
 */

#define VIDEO_PREFIX ((const char *)"video")
#define AUDIO_PREFIX ((const char *)"audio")
#define VIDEO_PREFIX_LEN 5
#define AUDIO_PREFIX_LEN 5

/** flags */
#define EXIT_PREFIX_CMP_FAILURE  -1 /**< Prefix not founded failure flag */
#define PREFIX_CMP_SUCCESS        0 /**< Prefix founded success flag */

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
 * @brief The capabilities of the inputs and outputs.
 *
 * This part creates a template for all the src pads
 * that the plugin is going to have
 *
 */

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
                                                                   GST_PAD_SRC,
                                                                   GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS_ANY
);

//==============================================================================

#define gst_mediademux_parent_class parent_class
G_DEFINE_TYPE (Gstmediademux, gst_mediademux, GST_TYPE_BIN);

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
 * @param mediaDemux The plugin instance
 * @return void
 */
static void createElements(Gstmediademux *mediaDemux);

/**
 * @brief verify if all elements were cretated
 *
 * @param mediaDemux The plugin instance
 * @return void
 */
static void verifyAllElementsCreated(Gstmediademux *mediaDemux);

/**
 * @brief Set some elements properties like udpsrc port
 *
 * @param mediaDemux The plugin instance
 * @return void
 */
static void setElementsPropsValues(Gstmediademux *mediaDemux);

/**
 * @brief Add elements to the main bin
 *
 * @param mediaDemux The plugin instance
 * @return void
 */
static void addElementsToBin(Gstmediademux *mediaDemux);

/**
 * @brief Link the internal plugins
 *
 * In order to follow a logic order is
 * necessary create a correct pads connection
 *
 * @param mediaDemux The plugin instance
 * @return void
 */
static void linkBinElements(Gstmediademux *mediaDemux);

/**
 * @brief Creates the audio and video ghost pads from src template
 *
 * @param mediaDemux
 * @return void
 */
static void createPluginGhostPads(Gstmediademux *mediaDemux);

/**
 * @brief set plugin ghost pads with the las video and audio plugin
 *
 * Link the plugin ghost pads with the audio and video last elements
 * Eg.
 * =============================
 * plugin_video_convert -> src = ======== VideoSrcGhostPad --- Plugin video flow output --->
 * =============================
 *
 * ==============================
 * plugin_audio_resample -> src = ======= AudioSrcGhostPad --- Plugin audio flow output --->
 * ==============================
 *
 * @param mediaDemux The plugin instance
 * @return void
 */
static void setPluginSrcPads(Gstmediademux *mediaDemux);



//==============================================================================

/**
 * @brief Demuxer add pads callback function
 *
 * @param demux The demux element with sometimes pad
 * @param new_pad New pad, can be audio or video type
 * @param mediaDemux The plugin instance
 */
static void cb_matroskaDemuxPadAdded (GstElement *demux, GstPad *new_pad, Gstmediademux *mediaDemux);
//==============================================================================

/**
 * @brief GObject vmethod implementations
 * initialize the mediaDemux class
 *
 */
static void gst_mediademux_class_init (GstmediademuxClass * klass){
    
    printf(WHITE "Bin-plugin  -- GeoWebMedia Class Init ---  \n" RESET);

    GstElementClass *gstelement_class;
    gstelement_class = (GstElementClass *) klass;
    
    gst_element_class_set_details_simple(gstelement_class,
    	"mediademux",
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
 * @param mediaDemux The plugin instance
 * @return void
 */
static void gst_mediademux_init (Gstmediademux *mediaDemux) {
    
    printf(WHITE "Bin-plugin  -- GeoWebMedia Init ---  \n" RESET);
    
    gboolean isVideoSrcPadActivated;
    gboolean isAudioSrcPadActivated;
    
    /** Elements  */
    createElements(mediaDemux);
    verifyAllElementsCreated(mediaDemux);
    setElementsPropsValues(mediaDemux);
    
    /** Bin */
    addElementsToBin(mediaDemux);
    linkBinElements(mediaDemux);
    
    /** Pads */
    createPluginGhostPads(mediaDemux);
    setPluginSrcPads(mediaDemux);
    
    /** plugin src pads */
    gst_element_add_pad (GST_ELEMENT (mediaDemux), mediaDemux->videoSrcPad);
    gst_element_add_pad (GST_ELEMENT (mediaDemux), mediaDemux->audioSrcPad);
    
    /** Active the pads  */
    isVideoSrcPadActivated = gst_pad_set_active (mediaDemux->videoSrcPad, TRUE);
    if (!isVideoSrcPadActivated)
        printf(RED "videoSrcPad: gst_pad_set_active = FALSE" RESET);
    
    isAudioSrcPadActivated = gst_pad_set_active (mediaDemux->audioSrcPad, TRUE);
    if (!isAudioSrcPadActivated)
        printf(RED "aduioSrcPad: gst_pad_set_active = FALSE" RESET);

    /**
     * Set this if the element always outputs data in the
     * exact same format as it receives as input
     * */
    GST_PAD_SET_PROXY_CAPS (mediaDemux->videoSrcPad);
    GST_PAD_SET_PROXY_CAPS (mediaDemux->audioSrcPad);

}

//==============================================================================

static void createElements(Gstmediademux *mediaDemux) {
    
    /**
     * Create all internal elements
     */
    
    /** udp src*/
    mediaDemux->plugin_udp_src = gst_element_factory_make("udpsrc", "udp-receiver");
    
    /** demuxer */
    mediaDemux->plugin_matroska_demux = gst_element_factory_make("matroskademux", "demuxer");
    
    /** add matroska demuxer element pad added callback */
    g_signal_connect(mediaDemux->plugin_matroska_demux, "pad-added", G_CALLBACK(cb_matroskaDemuxPadAdded), mediaDemux);
    
    /** video elements */
    mediaDemux->plugin_theora_dec = gst_element_factory_make("theoradec", "video-dec");
    mediaDemux->plugin_video_convert = gst_element_factory_make("videoconvert", "audio-converter");
    
    /** audio elements */
    mediaDemux->plugin_vorbis_dec = gst_element_factory_make("vorbisdec", "audio-decoder");
    mediaDemux->plugin_audio_convert = gst_element_factory_make("audioconvert", "audio-convert");
    mediaDemux->plugin_audio_resample = gst_element_factory_make("audioresample","audio-resample");
    
    /** queues */
    mediaDemux->plugin_video_queue = gst_element_factory_make("queue2", "video-queue");
    mediaDemux->plugin_audio_queue = gst_element_factory_make("queue2", "audio-queue");
    
}


//==============================================================================

static void verifyAllElementsCreated(Gstmediademux *mediaDemux){
    
    /**
     * Verify that all the internal plugins are created properly
     */
    
    gboolean allElementsCreated; /**< Boolean that stores the function return values*/
    
    allElementsCreated = mediaDemux->plugin_theora_dec
        && mediaDemux->plugin_video_convert
        && mediaDemux->plugin_vorbis_dec
        && mediaDemux->plugin_audio_convert
        && mediaDemux->plugin_audio_resample
        && mediaDemux->plugin_audio_queue
        && mediaDemux->plugin_video_queue
        && mediaDemux->plugin_matroska_demux
        && mediaDemux->plugin_udp_src;
    
    if(!allElementsCreated){
        printf (RED "One element could not be created\n" RESET);
        exit(EXIT_ELEMENT_CREATION_FAILURE);
    }

}


//==============================================================================

static void setElementsPropsValues(Gstmediademux *mediaDemux){
    
    /**
     * Configure the streaming capabilities mediademux
     */

    g_object_set (mediaDemux->plugin_udp_src, "port", DEFAULT_UDP_PORT, NULL);
}

//==============================================================================

static void addElementsToBin(Gstmediademux *mediaDemux){
    
    /**
    * Add elements to the bin mediademux
    */
    
    gst_bin_add_many(GST_BIN(mediaDemux) ,
                     GST_ELEMENT(mediaDemux->plugin_theora_dec),
                     GST_ELEMENT(mediaDemux->plugin_video_convert),
                     GST_ELEMENT(mediaDemux->plugin_vorbis_dec),
                     GST_ELEMENT(mediaDemux->plugin_audio_convert),
                     GST_ELEMENT(mediaDemux->plugin_audio_resample),
                     GST_ELEMENT(mediaDemux->plugin_video_queue),
                     GST_ELEMENT(mediaDemux->plugin_audio_queue),
                     GST_ELEMENT(mediaDemux->plugin_matroska_demux),
                     GST_ELEMENT(mediaDemux->plugin_udp_src),
                     NULL);
}

//==============================================================================

static void linkBinElements(Gstmediademux *mediaDemux){
    

    gboolean link_ok; /**< Boolean that stores the function return values*/
    
    /** link udp src and matroska demux*/
    link_ok = gst_element_link(mediaDemux->plugin_udp_src, mediaDemux->plugin_matroska_demux);
    if (!link_ok){
        printf(RED "UDP src fail linking pads with matroska demuxer" RESET);
        exit(EXIT_ELEMENT_LINKING_FAILURE);
    }
    
    /** Link the neccesary elements for do a correct analysis of video flow */
    link_ok = gst_element_link_many(mediaDemux->plugin_theora_dec,
                                    mediaDemux->plugin_video_convert,
                                    NULL);
    
    if (!link_ok){
        printf(RED "Fail linking video elements" RESET);
        exit(EXIT_ELEMENT_LINKING_FAILURE);
    }
    
    /** Link the neccesary elements for do a correct analysis of audio flow */
    link_ok = gst_element_link_many(mediaDemux->plugin_vorbis_dec,
                                    mediaDemux->plugin_audio_convert,
                                    mediaDemux->plugin_audio_resample,
                                    NULL);
    
    if (!link_ok){
        printf(RED "Fail linking audio elements" RESET);
        exit(EXIT_ELEMENT_LINKING_FAILURE);
    }
    
    
}

//==============================================================================

static void createPluginGhostPads(Gstmediademux *mediaDemux){
    
    /**
     * Create src ghost pads
     */
    
    mediaDemux->audioSrcPad = gst_ghost_pad_new_no_target_from_template ("audio_src",
                                                                         gst_static_pad_template_get (&src_factory));
    if (!mediaDemux->audioSrcPad) {
        printf(RED "mediademux audio src ghost pad no created from template \n" RESET);
        exit(EXIT_GHOSTPAD_CREATION_FAILURE);
    }
    
    mediaDemux->videoSrcPad = gst_ghost_pad_new_no_target_from_template ("video_src",
                                                                         gst_static_pad_template_get (&src_factory));
    if (!mediaDemux->videoSrcPad) {
        printf(RED "mediademux video src ghost pad no created from template \n" RESET);
        exit(EXIT_GHOSTPAD_CREATION_FAILURE);
    }

}

//==============================================================================

static void setPluginSrcPads(Gstmediademux *mediaDemux){
    
    /**
     * Link the ghost pads with the last element of the audio and video flow respectively
     */

    gboolean setGhostPad_ok; /**< Boolean that stores the function return values*/
    
    /** Get static pads of the created elements*/
    GstPad *videoSrcPad1 = gst_element_get_static_pad (mediaDemux->plugin_video_convert, "src");
    if (videoSrcPad1 == NULL) {
        printf(RED "Fail on get video src pad of element \n" RESET);
        exit(EXIT_GET_PAD_FAILURE);
    }
    
    GstPad *audioSrcPad1 = gst_element_get_static_pad (mediaDemux->plugin_audio_resample, "src");
    if (audioSrcPad1 == NULL) {
        printf(RED "Fail on get audio src pad of element \n" RESET);
        exit(EXIT_GET_PAD_FAILURE);
    }
    
    /** Set ghost pads with the elements src pads*/
    setGhostPad_ok = gst_ghost_pad_set_target ((GstGhostPad*)mediaDemux->videoSrcPad, videoSrcPad1);
    if(!setGhostPad_ok){
        printf(RED "Video ghost pad could not be linked with element src pad \n" RESET);
        exit(EXIT_SET_GHOSTH_PAD_FAILURE);
    }
    
    setGhostPad_ok = gst_ghost_pad_set_target ((GstGhostPad*)mediaDemux->audioSrcPad, audioSrcPad1);
    if(!setGhostPad_ok){
        printf(RED "Audio ghost pad could not be linked with element src pad \n" RESET);
        exit(EXIT_SET_GHOSTH_PAD_FAILURE);
    }
    
}

//==============================================================================

static void cb_matroskaDemuxPadAdded (GstElement *demuxer, GstPad* pad, Gstmediademux *mediaDemux) {
    
    char *padName; /**< type of pad that ig going to be created*/
    GstPad *sinkpad; /**< stores the sink pads of the audio and video queue*/
    gboolean audioPadsLink_ok; /**< Boolean that stores the function return values*/
    gboolean videoPadsLink_ok; /**< Boolean that stores the function return values*/
    GstPadLinkReturn audioPadLink_ok; /**< Stores the function return values with a specific format*/
    GstPadLinkReturn videoPadLink_ok; /**< Stores the function return values with a specific format*/
    
    padName = gst_pad_get_name(pad);
    
    printf ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (pad), GST_ELEMENT_NAME (demuxer));
    
    if(strncmp(padName, VIDEO_PREFIX, VIDEO_PREFIX_LEN) == PREFIX_CMP_SUCCESS){
        
        sinkpad = gst_element_get_static_pad(mediaDemux->plugin_video_queue, "sink");
        if (sinkpad == NULL) {
            printf(RED "Fail on get video queue sink pad \n" RESET);
            exit(EXIT_GET_PAD_FAILURE);
        }
        
        videoPadLink_ok = gst_pad_link(pad, sinkpad);
        if (videoPadLink_ok != GST_PAD_LINK_OK){
            printf(RED "New video pad linking fails" RESET);
            exit(EXIT_PADS_LINKING_FAILURE);
        }
    
        videoPadsLink_ok = gst_element_link_pads(mediaDemux->plugin_video_queue, "src", mediaDemux->plugin_theora_dec, "sink");
        if (!videoPadsLink_ok){
            printf(RED "Video queue pad linking with decoder pad fails" RESET);
            exit(EXIT_PADS_LINKING_FAILURE);
        }
        
        gst_object_unref(sinkpad);
        
        printf(GREEN "Linked pad %s of demuxer\n" RESET, padName);
    
    }else if(strncmp(padName, AUDIO_PREFIX, AUDIO_PREFIX_LEN) == PREFIX_CMP_SUCCESS){
        
        sinkpad = gst_element_get_static_pad(mediaDemux->plugin_audio_queue, "sink");
        if (sinkpad == NULL) {
            printf(RED "Fail on get audio queue sink pad \n" RESET);
            exit(EXIT_GET_PAD_FAILURE);
        }
    
        audioPadLink_ok = gst_pad_link(pad, sinkpad);
        if (audioPadLink_ok != GST_PAD_LINK_OK){
            printf(RED "New audio pad linking fails" RESET);
            exit(EXIT_PADS_LINKING_FAILURE);
        }
    
        audioPadsLink_ok = gst_element_link_pads(mediaDemux->plugin_audio_queue, "src", mediaDemux->plugin_vorbis_dec, "sink");
        if (!audioPadsLink_ok){
            printf(RED "Audio queue pad linking with decoder pad fails" RESET);
            exit(EXIT_PADS_LINKING_FAILURE);
        }
    
        gst_object_unref(sinkpad);
        
        printf (GREEN "Linked pad %s of demuxer\n" RESET, padName);
    }
    
    g_free (padName);
}

//==============================================================================

static GstStateChangeReturn gst_bin_change_state (GstElement *element, GstStateChange trans) {
    
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
static gboolean mediademux_init (GstPlugin * mediaDemux){
    
    printf(WHITE "Bin-plugin  -- GeoWebMedia --- Init plugin function \n" RESET);
    
    /** Debug category for fltering log messages
    * exchange the string 'Template mediademux' with your description
    */
    
    GST_DEBUG_CATEGORY_INIT (gst_mediademux_debug, "mediademux",0, "Template mediademux");
    
    return gst_element_register (mediaDemux, "mediademux", GST_RANK_NONE, GST_TYPE_MEDIADEMUX);
}

//==============================================================================

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstmediademux"
#endif

/* gstreamer looks for this structure to register mediademuxs
 *
 * exchange the string 'Template mediademux' with your mediademux description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    mediademux,
    "Template mediademux",
    mediademux_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
