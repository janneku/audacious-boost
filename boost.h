#ifndef BOOST_H
#define BOOST_H

#include <glib.h>

#define MAX_DELAY         100 /* msec */
#define MIN_CUTOFF        50 /* Hz */
#define MAX_SRATE         50000 /* Hz */

void boost_about(void);
void boost_configure(void);

extern gint boost_delay, boost_feedback, boost_volume, boost_cutoff;

#endif
