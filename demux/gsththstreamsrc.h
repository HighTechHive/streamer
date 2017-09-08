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

#ifndef __GST_HTHSTREAMSRC_H__
#define __GST_HTHSTREAMSRC_H__

#include <gst/gst.h>
    
    G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_HTHSTREAMSRC (gst_hthstreamsrc_get_type())
#define GST_HTHSTREAMSRC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_HTHSTREAMSRC,Gsththstreamsrc))
#define GST_HTHSTREAMSRC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_HTHSTREAMSRC,GsththstreamsrcClass))
#define GST_IS_HTHSTREAMSRC(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_HTHSTREAMSRC))
#define GST_IS_HTHSTREAMSRC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_HTHSTREAMSRC))

/**
 * @struct Gsththstreamsrc
 *
 * @brief This struct defines the internal elements of the plugin
 * Ej. Pads, internal plugins(bin plugin case)
 *
 */
    
    typedef struct _Gsththstreamsrc      Gsththstreamsrc;
    
    struct _Gsththstreamsrc{
        
        GstBin parent; /**< Parent struct. This element defines the plugin type */
        
        /** Video stream */
        GstElement *plugin_video_convert;   /**  takes an incoming stream of timestamped video frames
    									* It will produce a perfect stream that matches the source pad's framerate */
        GstElement *plugin_theora_dec;   /** This element encodes raw video into a Theora stream */
        
        /** Audio stream */
        GstElement *plugin_vorbis_dec;    /** This element decodes a raw float audio stream */
        GstElement *plugin_audio_resample;
        GstElement *plugin_audio_convert; /** This element converts raw audio buffers between various possible formats */
        
        /** Text stream */
        GstElement *plugin_identity;
        
        /** Queues */
        GstElement *plugin_video_queue; /** Tis element will create a new thread on the source pad to
                                    * decouple the processing on sink and source pad*/
        GstElement *plugin_audio_queue;
        GstElement *plugin_text_queue;
        
        /** src pad's */
        GstPad *videoSrcPad;  /**< video stream output pad */
        GstPad *audioSrcPad;  /**< audio stream output pad */
        GstPad *textSrcPad;  /**< data stream input pad*/
        
        /** Demuxer */
        GstElement *plugin_matroska_demux; /** This element demuxes different input streams into a Matroska file */
        
        /** Plugin transport */
        GstElement *plugin_udp_src; /**< Plugin that receive UDP packets from the network */
        
        /** Destination port */
        gint port;
    };

/**
 * @struct GsththstreamsrcClass
 *
 * @brief Generic struct that defines the plugin class.
 * This is useful for use the elements of the parent class
 *
 */
    
    typedef struct _GsththstreamsrcClass GsththstreamsrcClass;
    
    struct _GsththstreamsrcClass {
        GstBinClass parent_class; /**< Parent plugin class. Useful for access to elements that only the parent have*/
    };
    
    GType gst_hthstreamsrc_get_type(void);
    G_END_DECLS

#endif /* __GST_HTHSTREAMSRC_H__ */
