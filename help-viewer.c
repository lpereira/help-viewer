/*
 *    HelpViewer - Simple Help file browser
 *    Copyright (C) 2009 Leandro A. F. Pereira <leandro@hardinfo.org>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, version 2.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <gtk/gtk.h>
#include "markdown-text-view.h"

typedef struct _HelpViewer	HelpViewer;
struct _HelpViewer {
    GtkWidget *window;
    GtkWidget *status_bar;
    
    GtkWidget *btn_back, *btn_forward;
    GtkWidget *text_view;
    
    GSList *back_stack, *forward_stack;
    gchar *current_file;
};

static void forward_clicked(GtkWidget *widget, gpointer data)
{
    HelpViewer *hv = (HelpViewer *)data;
    GSList *temp;
    
    /* puts the current file on the back stack */
    hv->back_stack = g_slist_prepend(hv->back_stack, g_strdup(hv->current_file));
    
    /* enables the back button */
    gtk_widget_set_sensitive(hv->btn_back, TRUE);

    /* loads the new current file */
    markdown_textview_load_file(MARKDOWN_TEXTVIEW(hv->text_view), hv->forward_stack->data);
    
    /* pops the stack */
    temp = hv->forward_stack->next;
    g_free(hv->forward_stack->data);
    g_slist_free1(hv->forward_stack);
    hv->forward_stack = temp;
    
    /* if there aren't items on forward stack anymore, disables the button */
    if (!hv->forward_stack) {
        gtk_widget_set_sensitive(hv->btn_forward, FALSE);
    }
}

static void back_clicked(GtkWidget *widget, gpointer data)
{
    HelpViewer *hv = (HelpViewer *)data;
    GSList *temp;
    
    /* puts the current file on the forward stack */
    hv->forward_stack = g_slist_prepend(hv->forward_stack, g_strdup(hv->current_file));
    
    /* enables the forward button */
    gtk_widget_set_sensitive(hv->btn_forward, TRUE);

    /* loads the new current file */
    markdown_textview_load_file(MARKDOWN_TEXTVIEW(hv->text_view), hv->back_stack->data);
    
    /* pops the stack */
    temp = hv->back_stack->next;
    g_free(hv->back_stack->data);
    g_slist_free1(hv->back_stack);
    hv->back_stack = temp;
    
    /* if there aren't items on back stack anymore, disables the button */
    if (!hv->back_stack) {
        gtk_widget_set_sensitive(hv->btn_back, FALSE);
    }
}

static void link_clicked(MarkdownTextView *text_view, gchar *link, gpointer data)
{
    HelpViewer *hv = (HelpViewer *)data;
    
    /* adds the current file to the back stack (before loading the new file */
    hv->back_stack = g_slist_prepend(hv->back_stack, g_strdup(hv->current_file));
    gtk_widget_set_sensitive(hv->btn_back, TRUE);

    gtk_statusbar_pop(GTK_STATUSBAR(hv->status_bar), 1);
    markdown_textview_load_file(text_view, link);
}

static void file_load_complete(MarkdownTextView *text_view, gchar *file, gpointer data)
{
    HelpViewer *hv = (HelpViewer *)data;
    
    /* sets the currently-loaded file */
    g_free(hv->current_file);
    hv->current_file = g_strdup(file);
    
    gtk_statusbar_push(GTK_STATUSBAR(hv->status_bar), 1, "Done.");
}

static void hovering_over_link(MarkdownTextView *text_view, gchar *link, gpointer data)
{
    HelpViewer *hv = (HelpViewer *)data;
    
    gtk_statusbar_push(GTK_STATUSBAR(hv->status_bar), 1, link);
}

static void hovering_over_text(MarkdownTextView *text_view, gpointer data)
{
    HelpViewer *hv = (HelpViewer *)data;
    
    gtk_statusbar_pop(GTK_STATUSBAR(hv->status_bar), 1);
}

HelpViewer *
help_viewer_new (const gchar *help_file)
{
  HelpViewer *hv;
  GtkWidget *help_viewer;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *toolbar1;
  GtkIconSize tmp_toolbar_icon_size;
  GtkWidget *btn_back;
  GtkWidget *btn_forward;
  GtkWidget *separatortoolitem1;
  GtkWidget *btn_home;
  GtkWidget *toolbar2;
  GtkWidget *toolitem3;
#if !GTK_CHECK_VERSION(2,16,0)
  GtkWidget *label1;
#endif	/* GTK_CHECK_VERSION(2,16,0) */
  GtkWidget *toolitem4;
  GtkWidget *txt_search;
  GtkWidget *scrolledhelp_viewer;
  GtkWidget *markdown_textview;
  GtkWidget *status_bar;
  GdkPixbuf *icon;

  help_viewer = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_size_request(help_viewer, 300, 200);
  gtk_window_set_default_size(GTK_WINDOW(help_viewer), 640, 480);
  gtk_window_set_title (GTK_WINDOW (help_viewer), "Help Viewer");
  
  icon = gtk_widget_render_icon(help_viewer, GTK_STOCK_HELP, 
                                GTK_ICON_SIZE_DIALOG,
                                NULL);
  gtk_window_set_icon(GTK_WINDOW(help_viewer), icon);
  g_object_unref(icon);
  
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (help_viewer), vbox);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  toolbar1 = gtk_toolbar_new ();
  gtk_widget_show (toolbar1);
  gtk_box_pack_start (GTK_BOX (hbox), toolbar1, TRUE, TRUE, 0);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar1), GTK_TOOLBAR_BOTH_HORIZ);
  tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (toolbar1));

  btn_back = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-go-back");
  gtk_widget_show (btn_back);
  gtk_container_add (GTK_CONTAINER (toolbar1), btn_back);
  gtk_tool_item_set_is_important (GTK_TOOL_ITEM (btn_back), TRUE);
  gtk_widget_set_sensitive(btn_back, FALSE);

  btn_forward = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-go-forward");
  gtk_widget_show (btn_forward);
  gtk_container_add (GTK_CONTAINER (toolbar1), btn_forward);
  gtk_widget_set_sensitive(btn_forward, FALSE);

  separatortoolitem1 = (GtkWidget*) gtk_separator_tool_item_new ();
  gtk_widget_show (separatortoolitem1);
  gtk_container_add (GTK_CONTAINER (toolbar1), separatortoolitem1);

  btn_home = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-home");
  gtk_widget_show (btn_home);
  gtk_container_add (GTK_CONTAINER (toolbar1), btn_home);

  toolbar2 = gtk_toolbar_new ();
  gtk_widget_show (toolbar2);
  gtk_box_pack_end (GTK_BOX (hbox), toolbar2, FALSE, TRUE, 0);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar2), GTK_TOOLBAR_BOTH_HORIZ);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (toolbar2), FALSE);
  tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (toolbar2));

  toolitem3 = (GtkWidget*) gtk_tool_item_new ();
  gtk_widget_show (toolitem3);
  gtk_container_add (GTK_CONTAINER (toolbar2), toolitem3);

#if !GTK_CHECK_VERSION(2,16,0)
  label1 = gtk_label_new_with_mnemonic ("_Search:");
  gtk_widget_show (label1);
  gtk_container_add (GTK_CONTAINER (toolitem3), label1);
#endif	/* GTK_CHECK_VERSION(2,16,0) */

  toolitem4 = (GtkWidget*) gtk_tool_item_new ();
  gtk_widget_show (toolitem4);
  gtk_container_add (GTK_CONTAINER (toolbar2), toolitem4);

  txt_search = gtk_entry_new ();
  gtk_widget_show (txt_search);
  gtk_container_add (GTK_CONTAINER (toolitem4), txt_search);
  gtk_entry_set_invisible_char (GTK_ENTRY (txt_search), 9679);
#if GTK_CHECK_VERSION(2,16,0)
  gtk_entry_set_icon_from_stock(GTK_ENTRY(txt_search), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FIND);
#endif	/* GTK_CHECK_VERSION(2,16,0) */

  scrolledhelp_viewer = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledhelp_viewer);
  gtk_box_pack_start (GTK_BOX (vbox), scrolledhelp_viewer, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledhelp_viewer), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  markdown_textview = markdown_textview_new();
  gtk_widget_show (markdown_textview);
  gtk_container_add (GTK_CONTAINER (scrolledhelp_viewer), markdown_textview);

  status_bar = gtk_statusbar_new ();
  gtk_widget_show (status_bar);
  gtk_box_pack_start (GTK_BOX (vbox), status_bar, FALSE, FALSE, 0);

  hv = g_new0(HelpViewer, 1);
  hv->window = help_viewer;
  hv->status_bar = status_bar;
  hv->btn_back = btn_back;
  hv->btn_forward = btn_forward;
  hv->text_view = markdown_textview;

  g_signal_connect(markdown_textview, "link-clicked", G_CALLBACK(link_clicked), hv);
  g_signal_connect(markdown_textview, "hovering-over-link", G_CALLBACK(hovering_over_link), hv);
  g_signal_connect(markdown_textview, "hovering-over-text", G_CALLBACK(hovering_over_text), hv);
  g_signal_connect(markdown_textview, "file-load-complete", G_CALLBACK(file_load_complete), hv);

  g_signal_connect(btn_back, "clicked", G_CALLBACK(back_clicked), hv);
  g_signal_connect(btn_forward, "clicked", G_CALLBACK(forward_clicked), hv);
  
  gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar), "bla");
  markdown_textview_load_file(MARKDOWN_TEXTVIEW(markdown_textview), help_file);

  return hv;
}


int main(int argc, char **argv)
{
    HelpViewer *hv;
    
    gtk_init(&argc, &argv);
    
    hv = help_viewer_new("README");
    gtk_widget_show_all(hv->window);
    
    gtk_main();
}
