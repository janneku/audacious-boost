#include <gtk/gtk.h>

#include <audacious/configdb.h>
#include <audacious/gtk-compat.h>
#include <audacious/plugin.h>
#include <audacious/i18n.h>
#include <libaudgui/libaudgui.h>
#include <libaudgui/libaudgui-gtk.h>

#include "boost.h"

static const char *boost_about_text =
    "Subwoofer simulation plugin\n"
    "By Janne Kulmala 2012\n"
    "Based on XMMS/Audacious echo plugin by Johan Levin 1999.";

static GtkWidget *conf_dialog = NULL;
static GtkAdjustment *boost_delay_adj, *boost_feedback_adj, *boost_volume_adj;
static GtkAdjustment *boost_cutoff_adj;

void boost_about(void)
{
	static GtkWidget *boost_about_dialog = NULL;

	audgui_simple_message(&boost_about_dialog, GTK_MESSAGE_INFO,
			      "About Boost Plugin", boost_about_text);
}

static void apply_changes(void)
{
	mcs_handle_t *cfg;
	boost_delay = gtk_adjustment_get_value(boost_delay_adj);
	boost_feedback = gtk_adjustment_get_value(boost_feedback_adj);
	boost_volume = gtk_adjustment_get_value(boost_volume_adj);
	boost_cutoff = gtk_adjustment_get_value(boost_cutoff_adj);

	cfg = aud_cfg_db_open();
	if (!cfg)
		return;
	aud_cfg_db_set_int(cfg, "boost_plugin", "delay", boost_delay);
	aud_cfg_db_set_int(cfg, "boost_plugin", "feedback", boost_feedback);
	aud_cfg_db_set_int(cfg, "boost_plugin", "volume", boost_volume);
	aud_cfg_db_set_int(cfg, "boost_plugin", "cutoff", boost_cutoff);
	aud_cfg_db_close(cfg);
}

static void conf_ok_cb(GtkButton * button, gpointer data)
{
	apply_changes();
	gtk_widget_destroy(GTK_WIDGET(conf_dialog));
}

static void conf_cancel_cb(GtkButton * button, gpointer data)
{
	gtk_widget_destroy(GTK_WIDGET(conf_dialog));
}

static void conf_apply_cb(GtkButton * button, gpointer data)
{
	apply_changes();
}

void boost_configure(void)
{
	GtkWidget *button, *table, *label, *hscale, *bbox;

	if (conf_dialog != NULL)
		return;

	conf_dialog = gtk_dialog_new();
	g_signal_connect(conf_dialog, "destroy",
			 (GCallback) gtk_widget_destroyed, &conf_dialog);
	gtk_window_set_title(GTK_WINDOW(conf_dialog), "Configure Boost");

	boost_delay_adj = (GtkAdjustment *) gtk_adjustment_new(boost_delay, 0,
							       MAX_DELAY + 10,
							       2, 10, 10);
	boost_feedback_adj =
	    (GtkAdjustment *) gtk_adjustment_new(boost_feedback, 0, 100 + 10, 2,
						 10, 10);
	boost_volume_adj =
	    (GtkAdjustment *) gtk_adjustment_new(boost_volume, 0, 100 + 10, 2,
						 10, 10);
	boost_cutoff_adj =
	    (GtkAdjustment *) gtk_adjustment_new(boost_cutoff, MIN_CUTOFF,
						 MIN_CUTOFF * 10 + 10, 2, 10,
						 10);

	table = gtk_table_new(2, 4, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table), 5);
	gtk_container_set_border_width(GTK_CONTAINER(table), 5);
	gtk_box_pack_start((GtkBox *) gtk_dialog_get_content_area((GtkDialog *)
								  conf_dialog),
			   table, TRUE, TRUE, 5);
	gtk_widget_show(table);

	label = gtk_label_new("Reverb delay: (ms)");
	gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL,
			 GTK_FILL, 0, 0);
	gtk_widget_show(label);

	label = gtk_label_new("Feedback amount: (%)");
	gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2, GTK_FILL,
			 GTK_FILL, 0, 0);
	gtk_widget_show(label);

	label = gtk_label_new("Passthrough volume: (%)");
	gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL,
			 GTK_FILL, 0, 0);
	gtk_widget_show(label);

	label = gtk_label_new("Lowpass cutoff: (Hz)");
	gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4, GTK_FILL,
			 GTK_FILL, 0, 0);
	gtk_widget_show(label);

	hscale = gtk_hscale_new(GTK_ADJUSTMENT(boost_delay_adj));
	gtk_scale_set_digits(GTK_SCALE(hscale), 0);
	gtk_table_attach_defaults(GTK_TABLE(table), hscale, 1, 2, 0, 1);
	gtk_widget_show(hscale);

	hscale = gtk_hscale_new(GTK_ADJUSTMENT(boost_feedback_adj));
	gtk_scale_set_digits(GTK_SCALE(hscale), 0);
	gtk_table_attach_defaults(GTK_TABLE(table), hscale, 1, 2, 1, 2);
	gtk_widget_show(hscale);

	hscale = gtk_hscale_new(GTK_ADJUSTMENT(boost_volume_adj));
	gtk_scale_set_digits(GTK_SCALE(hscale), 0);
	gtk_table_attach_defaults(GTK_TABLE(table), hscale, 1, 2, 2, 3);
	gtk_widget_show(hscale);

	hscale = gtk_hscale_new(GTK_ADJUSTMENT(boost_cutoff_adj));
	gtk_scale_set_digits(GTK_SCALE(hscale), 0);
	gtk_table_attach_defaults(GTK_TABLE(table), hscale, 1, 2, 3, 4);
	gtk_widget_show(hscale);

	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_box_pack_start((GtkBox *) gtk_dialog_get_action_area((GtkDialog *)
								 conf_dialog),
			   bbox, TRUE, TRUE, 0);

	button = gtk_button_new_with_label("Ok");
	gtk_widget_set_can_default(button, TRUE);
	gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
	g_signal_connect(button, "clicked", (GCallback) conf_ok_cb, NULL);
	gtk_widget_grab_default(button);
	gtk_widget_show(button);

	button = gtk_button_new_with_label("Cancel");
	gtk_widget_set_can_default(button, TRUE);
	gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
	g_signal_connect(button, "clicked", (GCallback) conf_cancel_cb, NULL);
	gtk_widget_show(button);

	button = gtk_button_new_with_label("Apply");
	gtk_widget_set_can_default(button, TRUE);
	gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
	g_signal_connect(button, "clicked", (GCallback) conf_apply_cb, NULL);
	gtk_widget_show(button);
	gtk_widget_show(bbox);

	gtk_widget_show(conf_dialog);
}
