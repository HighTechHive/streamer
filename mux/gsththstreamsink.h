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

#ifndef __GST_HTHSTREAMSINK_H__
#define __GST_HTHSTREAMSINK_H__

#include <gst/gst.h>
#include <glib.h>

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_HTHSTREAMSINK (gst_hthstreamsink_get_type())
#define GST_HTHSTREAMSINK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_HTHSTREAMSINK,Gsththstreamsink))
#define GST_HTHSTREAMSINK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_HTHSTREAMSINK,GsththstreamsinkClass))
#define GST_IS_HTHSTREAMSINK(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_HTHSTREAMSINK))
#define GST_IS_HTHSTREAMSINK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_HTHSTREAMSINK))

/**
 * @struct Gsththstreamsink
 *
 * @brief This struct defines the internal elements of the plugin
 * Ej. Pads, internal plugins(bin plugin case)
 *
 */

typedef struct _Gsththstreamsink      Gsththstreamsink;

struct _Gsththstreamsink{
    
    GstBin parent; /**< Parent struct. This element defines the plugin type */
    
    /** Video stream */
    GstElement *plugin_time_overlay; /**< This element add the stream time in the video window */
    GstElement *plugin_caps_filter;  /**< This element that works between plugins
    									* Modifies the stream original capabilities like the weight or width */
    GstElement *plugin_video_rate;   /**  takes an incoming stream of timestamped video frames
    									* It will produce a perfect stream that matches the source pad's framerate */
    GstElement *plugin_theora_enc;   /** This element encodes raw video into a Theora stream */
    
    /** Audio stream */
    GstElement *plugin_audio_convert; /** This element converts raw audio buffers between various possible formats */
    GstElement *plugin_vorbis_enc;    /** This element encodes raw float audio into a Vorbis stream */
    
    /** Text stream */
    GstElement *plugin_identity; /** This element is used only for watch the text stream */
    
    /** Queues */
    GstElement *plugin_video_queue; /** Tis element will create a new thread on the source pad to
                                    * decouple the processing on sink and source pad*/
    GstElement *plugin_audio_queue;
    GstElement *plugin_text_queue;
    
    /** sink pad's */
    GstPad *videoSinkPad; /**< video stream input pad */
    GstPad *audioSinkPad; /**< audio stream input pad*/
    GstPad *textSinkPad; /**< text stream input pad*/
    
    /** Muxer */
    GstElement *plugin_matroska_mux; /** This element muxes different input streams into a Matroska file */
    
    /** Plugin transport */
    GstElement *plugin_udp_sink; /**< Plugin that ends UDP packets to the network */
    
    /** Destination host */
    gchar *host;
    
    /** Destination port */
    gint port;
    
};

/**
 * @struct GsththstreamsinkClass
 *
 * @brief Generic struct that defines the plugin class.
 * This is useful for use the elements of the parent class
 *
 */

typedef struct _GsththstreamsinkClass GsththstreamsinkClass;

struct _GsththstreamsinkClass {
    GstBinClass parent_class; /**< Parent plugin class. Useful for access to elements that only the parent have*/
};

GType gst_hthstreamsink_get_type (void);
G_END_DECLS

#endif /* __GST_HTHSTREAMSINK_H__ */
