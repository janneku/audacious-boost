#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include <audacious/configdb.h>
#include <audacious/i18n.h>
#include <audacious/plugin.h>

#include "boost.h"


static gboolean init (void);
static void cleanup(void);

#define MAX_SRATE 50000
#define MAX_CHANNELS 2
#define BYTES_PS sizeof(gfloat)
#define BUFFER_SAMPLES (MAX_SRATE * MAX_DELAY / 1000)
#define BUFFER_SHORTS (BUFFER_SAMPLES * MAX_CHANNELS)
#define BUFFER_BYTES (BUFFER_SHORTS * BYTES_PS)

static gfloat *buffer = NULL;
gint boost_delay = 500, boost_feedback = 50, boost_volume = 50;
static int w_ofs;

static gboolean init (void)
{
	mcs_handle_t *cfg;

	cfg = aud_cfg_db_open();
	if (! cfg)
		return TRUE;
	aud_cfg_db_get_int(cfg, "boost_plugin", "delay", &boost_delay);
	aud_cfg_db_get_int(cfg, "boost_plugin", "feedback", &boost_feedback);
	aud_cfg_db_get_int(cfg, "boost_plugin", "volume", &boost_volume);
	aud_cfg_db_close(cfg);

	return TRUE;
}

static void cleanup(void)
{
	g_free(buffer);
	buffer = NULL;
}

static gint boost_channels = 0;
static gint boost_rate = 0;

static void boost_start(gint *channels, gint *rate)
{
	static gint old_srate, old_nch;

	if (buffer == NULL)
		buffer = g_malloc0(BUFFER_BYTES + sizeof(gfloat));

	boost_channels = *channels;
	boost_rate = *rate;

	if (boost_channels != old_nch || boost_rate != old_srate)
	{
		memset(buffer, 0, BUFFER_BYTES);
		w_ofs = 0;
		old_nch = boost_channels;
		old_srate = boost_rate;
	}
}

static void boost_flush(void)
{

}

static gint boost_decoder_to_output_time(gint time)
{
	return time;
}

static gint boost_output_to_decoder_time(gint time)
{
	return time;
}

static void boost_process(gfloat **d, gint *samples)
{
	gfloat in, out, buf;
	gint r_ofs;
	gfloat *data = *d;
	gfloat *end = *d + *samples;

	r_ofs = w_ofs - (boost_rate * boost_delay / 1000) * boost_channels;
	if (r_ofs < 0)
		r_ofs += BUFFER_SHORTS;

	for (; data < end; data++)
	{
		in = *data;

		buf = buffer[r_ofs];
		out = in + buf * boost_volume / 100;
		buf = in + buf * boost_feedback / 100;
		buffer[w_ofs] = buf;
		*data = out;

		if (++r_ofs >= BUFFER_SHORTS)
			r_ofs -= BUFFER_SHORTS;
		if (++w_ofs >= BUFFER_SHORTS)
			w_ofs -= BUFFER_SHORTS;
	}
}

static void boost_finish(gfloat **d, gint *samples)
{
	boost_process(d, samples);
}

AUD_EFFECT_PLUGIN
(
	.name = "Boost",
	.init = init,
	.cleanup = cleanup,
	.about = boost_about,
	.configure = boost_configure,
	.start = boost_start,
	.process = boost_process,
	.flush = boost_flush,
	.finish = boost_finish,
	.decoder_to_output_time = boost_decoder_to_output_time,
	.output_to_decoder_time = boost_output_to_decoder_time
)
