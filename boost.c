#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include <audacious/configdb.h>
#include <audacious/i18n.h>
#include <audacious/plugin.h>

#include "boost.h"


static gboolean init(void);
static void cleanup(void);

#define MAX_CHANNELS      2
#define BUFFER_SAMPLES    (MAX_SRATE / 2 / MIN_CUTOFF + \
                           MAX_SRATE * MAX_DELAY / 1000)
#define BUFFER_FRAMES     (BUFFER_SAMPLES * MAX_CHANNELS)
#define BUFFER_BYTES      (BUFFER_FRAMES * sizeof(gfloat))

static gfloat *buffer = NULL;
gint boost_delay = 20, boost_feedback = 60, boost_volume = 10;
gint boost_cutoff = 100;
static gint write_pos;

static gboolean init(void)
{
	mcs_handle_t *cfg;

	cfg = aud_cfg_db_open();
	if (! cfg)
		return TRUE;
	aud_cfg_db_get_int(cfg, "boost_plugin", "delay", &boost_delay);
	aud_cfg_db_get_int(cfg, "boost_plugin", "feedback", &boost_feedback);
	aud_cfg_db_get_int(cfg, "boost_plugin", "volume", &boost_volume);
	aud_cfg_db_get_int(cfg, "boost_plugin", "cutoff", &boost_cutoff);
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
		write_pos = 0;
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
	gint i, pos;
	gfloat *data = *d;
	gfloat *end = *d + *samples;

	gint count = boost_rate / 2 / boost_cutoff;

	for (; data < end; data++)
	{
		pos = write_pos - (boost_rate * boost_delay / 1000) * boost_channels - 1;
		out = 0;
		for (i = 0; i < count; ++i) {
			if (pos < 0)
				pos += BUFFER_FRAMES;
			out += buffer[pos];
			pos -= boost_channels;
		}
		out = out / count;
		buffer[write_pos] = *data + out * (boost_feedback / 100.0);
		*data = *data * (boost_volume / 100.0) + out;

		if (++write_pos >= BUFFER_FRAMES)
			write_pos -= BUFFER_FRAMES;
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
