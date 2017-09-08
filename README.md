# Gstreamer plugins

## hthstreamsink

### Internal elements:

#### Network 
* udpsink - Is a network sink that sends UDP packets to the network.

#### Muxer
* matroskamux Muxes different input streams into a Matroska file.

#### Audio:
* vorbisenc - Encodes raw float audio into a Vorbis stream.
* audioconvert - Converts raw audio buffers between various possible formats.

#### Video:
* timeoverlay - This element overlays the buffer time stamps of a video stream on top of itself.
* capsfilter - The element does not modify data as such, but can enforce limitations on the data format.
* videorate - This element takes an incoming stream of timestamped video frames. It will produce a perfect stream that matches the source pad's framerate.
* theoraenc - This element encodes raw video into a Theora stream.

#### Text:
* Identity - Dummy element that passes incoming data through unmodified. Useful for monitoring of text stream (handoff signal)

#### Queue:
* queue2 - The queue will create a new thread on the source pad to decouple the processing on sink and source pad.

### Plugin internal pipeline
The internal plugin pipeline can be illustrated in such way:
	
	$ For request pads
	@ For static pads

			 ----------------------------------------------------------------------------------------------------------------------------------------
			 -	 																											  .---------------------.		  
			 -	       .---------------.       .--------------.       .-------------.       .-------------.    .------.       |			    	    |  
	-vid-> @ sink -> sink timeoverlay src -> sink capsfilter src -> sink videorate src -> sink theoraenc src -> queue2 -> $ sink video              |       
			 -         '---------------'       '--------------'       '-------------'       '-------------'    '------'       | 		            |       
			 -         .------------.       .----------.                                                                      |                     |
	-txt-> @ sink -> sink identity src -> sink queue2 src --------------------------------------------------------------> $ sink text matroskamux sink ->
			 -         '------------'       '----------'                                                                      |                     |
			 -   	   .----------------.	    .-------------.	   .------.											          |                     |       
	-aud-> @ sink -> sink audioconvert src -> sink vorbisenc src -> queue2  --------------------------------------------> $ sink audio              |       
			 -		   '----------------'       '-------------'    '------'		                                              |	                    |       
			 -     																											  '---------------------'       
			 ----------------------------------------------------------------------------------------------------------------------------------------

			 ------------------------------------------------------------------------------------
			                                                                                    -
			 .-------------------------.                                                        -
			 |						   |						                                -
			 |			               |													    -
	       $ sink video	               |														-
			 |			   			   |          .-----------.									-
		   $ sink text    matroskamux  @ sink -> sink udpsink | host = dst_host port = dst_port	-
			 |			               |          '-----------'									-
	       $ sink audio                |														-
			 |			               |														-
			 |                         |                                                        -
			 '-------------------------'														-
			 ------------------------------------------------------------------------------------

### Bash pipeline
The pipeline can be illustrated in such way:

																	 
		                    .----------.
    Device = /dev/video0	|  v4l2src | -
    	                    '----------'  - - -              .---------------.
     	   		   	        .---------------.  -     	     |			     |          
    Device = /dev/ttyS0     | serialtextsrc |  - - --------> | hthstreamsink | 
                            '---------------'  -             |			     |          
                            .----------.  - - -              '---------------'
    Device = hw:0,0         |  alsasrc |- 
                            '----------'


### How to use:

```
$ gst-launch-1.0 v4l2src ! mezclador. alsasrc ! mezclador. serialtextsrc ! mezclador. hthstreamsink *host=x.x.x.x* *port=xxxx* name=mezclador

```

## hthstreamsrc

### Internal elements:

#### Network 
* udpsrc - Is a network source that reads UDP packets from the network.

#### Demuxer
* matroskademux - Demuxes a Matroska file into the different contained streams.


#### Audio:
* vorbisdec - Decodes a Vorbis stream to raw float audio.
* audioconvert - Converts raw audio buffers between various possible formats.
* audioresample - Resamples raw audio buffers to different sample rates using a configurable windowing function to enhance quality.

#### Video:
* theoradec - Decodes theora streams into raw video.
* videoconvert - Convert video frames between a great variety of video formats.

#### Text:
* Identity Dummy element that passes incoming data through unmodified. Useful for monitoring of text stream (handoff signal)

#### Queue:
* queue2 - Create a new thread on the source pad to decouple the processing on sink and source pad.

### Plugin internal pipeline
The internal plugin pipeline can be illustrated in such way:
	
	% For sometimes pads
	@ For static pads

	-------------------------------------------------------------------------------------------------------------------------------------------------------------
    -																																			                -
    -             	                       .-----------------.																					                -
    -                                      |                   |              .------.     .-------------.       .----------------.       .-----------------.   -    
    -                                      |                   % src audio ->  queue2 -> sink vorbisdec src -> sink audioconvert src -> sink audioresample src -> @ video src 
    -              	    	               |			       |              '------'     '-------------'       '----------------'       '-----------------'   - 
	-                 .------------.       |                   |              .------.     .------------.                                                       -
	- port = src_port | udpsource src -->  @ src matroskademux % src text  ->  queue2 -> sink identity src -----------------------------------------------------> @ text src
	-				  '------------'       |                   |              '------'     '------------'                                                       - 
	-                                      |                   |              .------.     .-------------.       .----------------.                             -
	-                                      |                   % src video ->  queue2 -> sink theoradec src -> sink videoconvert src ---------------------------> @ audio src
	-                                      |                   |              '------'     '-------------'       '----------------'                             - 
	-                                      '-------------------'                                                                                                -
	-                                                                                                                                                           -
	-------------------------------------------------------------------------------------------------------------------------------------------------------------


### Plugin internal pipeline
The bash pipeline can be illustrated in such way:												


### How to use:

```bash
$ gst-launch-1.0 hthstreamsrc *port=xxxx* name=demux demux. ! alsasink sync=false demux. ! xvimagesink sync=false demux. ! fakesink

```

## serialtextsrc

### Internal elements:

#### Stream generation 
* appsrc - This element can be used by applications to insert data into a GStreamer pipeline.

### Bash pipeline
The internal plugin pipeline can be illustrated in such way:												
	
	.---------------.
    |               |
    |               |			
    |  .--------.   |
    |   appsrc src -> src text 
    |  '--------'   |
    |               |
    |               |     
    '---------------'

### How to use:

```bash
$ gst-launch-1.0 serialtextsrc device=/dev/pts/19,9600,8n1 ! fakesink dump=true

```

Where "/dev/pts/19" is the device, "9600" the speed and "8n1" the settings.
You need to use this specific format, otherwise the plugin doesn't work.¿

### How to create a serial virtual port
Socat tool is required. You can install with apt install.

* Step 1

```bash
$ user@myuser sudo apt install socat

```

* Step 2

Run the command:

```bash
$ user@myuser socat -d -d pty,raw,echo=0 pty,raw,echo=0

```

This generates something like this

```bash
2013/11/01 13:47:27 socat[2506] N PTY is /dev/pts/2
2013/11/01 13:47:27 socat[2506] N PTY is /dev/pts/3
2013/11/01 13:47:27 socat[2506] N starting data transfer loop with FDs [3,3] and [5,5]

```

* Step 3

Run the python script


### Script to generate text

If you want to generate text for send to the serialtextsrc, run the follow python script.
The device name can be changed according to the pc. It's depends on the previous output.

E.g /dev/pts/2 or /dev/pts/3. The other one is going to be the parameter of the serialtextsrc.

#### Required modules
Yo require python 2.7 and subprocess and time modules.
If you want to use python 3.x, you need to change subprocess.call() per subprocess.run()

#### How to run
```bash
$ user@myuser python textgen.py /dev/name

```

## Compile plugins instructions

First you need to do is enter to mux/demux dir. In this case we are going to use mux directory.

* Step 1

Once in the directory, you need to copy hthstreamsink.c, hthstreamsink.h and Makefile.am into gst-plugin/src 

```bash
$ user@myuser ~/gstreamer-plugin/mux cp hthstreamsink.c hthstreamsink.h Makefile.am ../gst-plugin/src

```

* Step 2

Enter in gst-plugin/ and run ./autogen.sh

```bash
$ user@myuser ~/gst-plugin ./autogen.sh

```

This is going to enter in src/ and grab the files that Makefile.am uses in its declarations

* Step 3

Run make command for compile

```bash
$ user@myuser ~/gst-plugin make

```

* Step 4

Run sudo make install for install the files in a directory

```bash
$ user@myuser ~/gst-plugin sudo make install

```

This is going to generate libhthstreamsink.la and libhthstreamsink.so and put them in /usr/local/lib/gstreamer-1.0

* Step 5

Copy the .la and .so files into /usr/lib/x86_64-linux-gnu/gstreamer-1.0/

```bash
$ user@myuser /usr/local/lib/gstreamer-1.0 sudo cp libhthstreamsink.la libhthstreamsink.so /usr/lib/x86_64-linux-gnu/gstreamer-1.0/

```

* Step 6

Test the plugin and have a lot of fun



