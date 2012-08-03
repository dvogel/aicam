#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

static cairo_surface_t *room_surface = NULL;
static int room_pixel_width = 0;
static int room_pixel_height = 0;
static int room_meter_width = 12;
static int room_meter_height = 4;
static double pixels_per_meter_horz = 0.0;
static double pixels_per_meter_vert = 0.0;

static gboolean on_delete_event (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	if (room_surface != NULL) {
		cairo_surface_destroy(room_surface);
		room_surface = NULL;
	}
	return FALSE;
}

static void clear_surface (cairo_surface_t *surface)
{
	cairo_t *cairo;
	cairo = cairo_create(surface);
	cairo_set_source_rgb(cairo, 1, 1, 1);
	cairo_paint(cairo);
	cairo_destroy(cairo);
}

static gboolean configure_event_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	if (room_surface != NULL) {
		cairo_surface_destroy(room_surface);
	}

	room_pixel_width = gtk_widget_get_allocated_width(widget);
	room_pixel_height = gtk_widget_get_allocated_height(widget);
	pixels_per_meter_horz = room_pixel_width / room_meter_width;
	pixels_per_meter_vert = room_pixel_height / room_meter_height;
	room_surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
													 CAIRO_CONTENT_COLOR,
													 room_pixel_width,
													 room_pixel_height);
	clear_surface(room_surface);
	return TRUE;
}

static gboolean draw_cb (GtkWidget *widget, cairo_t *cairo, gpointer data)
{
	cairo_set_source_surface(cairo, room_surface, 0, 0);
	cairo_set_source_rgba(cairo, 1.0, 1.0, 1.0, 1.0);
	cairo_rectangle(cairo, 0, 0, room_pixel_width, room_pixel_height);
	cairo_fill(cairo);

	cairo_set_line_width(cairo, 1);
	cairo_set_source_rgba(cairo, 0.8, 0.8, 0.8, 0.6);
	for (int horz_meter = 1; horz_meter < room_meter_width; horz_meter++) {
		cairo_move_to(cairo, horz_meter * pixels_per_meter_horz, 0);
		cairo_line_to(cairo, horz_meter * pixels_per_meter_horz, room_pixel_height);
		cairo_stroke(cairo);
	}

	for (int vert_meter = 1; vert_meter < room_meter_height; vert_meter++) {
		cairo_move_to(cairo, 0, vert_meter * pixels_per_meter_vert);
		cairo_line_to(cairo, room_pixel_width, vert_meter * pixels_per_meter_vert);
		cairo_stroke(cairo);
	}

	cairo_paint(cairo);
	return FALSE;
}

static gboolean room_drawing_button_press_cb (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (room_surface == NULL) {
		return FALSE;
	}

	return TRUE;
}

static gboolean update_audio_channel (gpointer data)
{
	GtkProgressBar *balance_bar = data;

	gtk_progress_bar_set_fraction(balance_bar, 0.5);

	return TRUE;
}

int main(int argc, char *argv[])
{
	GtkBuilder *builder = NULL;
	GtkWidget *window = NULL;
	GtkWidget *room_drawing = NULL;
	GtkWidget *balance_bar = NULL;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "trainer.ui", NULL);

	window = gtk_builder_get_object(builder, "main_window");

	room_drawing = gtk_builder_get_object(builder, "room_drawing");

	balance_bar = gtk_builder_get_object(builder, "balance_bar");

	gtk_widget_set_events(room_drawing,
	                      (gtk_widget_get_events(room_drawing)
						  | GDK_BUTTON_PRESS_MASK
						  | GDK_POINTER_MOTION_MASK));

	g_timeout_add(100, update_audio_channel, balance_bar);

	g_signal_connect(room_drawing, "draw", G_CALLBACK(draw_cb), NULL);

	g_signal_connect(room_drawing, "configure-event", G_CALLBACK(configure_event_cb), NULL);

	g_signal_connect(room_drawing, "button-press-event", G_CALLBACK(room_drawing_button_press_cb), NULL);

	g_signal_connect(window, "delete-event", G_CALLBACK(on_delete_event), NULL);

	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_widget_show(window);

	gtk_main();

	return 0;
}
