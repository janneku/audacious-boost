#ifndef BOOST_H
#define BOOST_H

#include <glib.h>

#define MAX_DELAY 1000

void boost_about(void);
void boost_configure(void);

extern gint boost_delay, boost_feedback, boost_volume;
extern gboolean boost_surround_enable;


#endif
