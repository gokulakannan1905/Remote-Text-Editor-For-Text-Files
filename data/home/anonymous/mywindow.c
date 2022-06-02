#include<gtk/gtk.h>

int main(int argc, char*argv[]){
	GtkWidget* p;
	gtk_init(&argc,&argv);
	p = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(p,"Sample Window");
	g_signal_connect(p,"destroy",gtk_main_quit,NULL);
	gtk_widget_set_size_request(p,300,300);
	gtk_widget_show(p);
	gtk_main();
	return 0;
}
	
