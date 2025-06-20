#ifndef __WEBWINDOW_H__
#define __WEBWINDOW_H__

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>
#include <webkit2/webkit2.h>

// https://developer.gnome.org/gtk3/stable/GtkWindow.html
// https://webkitgtk.org/reference/webkit2gtk/2.5.1/WebKitWebView.html

typedef struct {
    GtkWindow *win;
    GtkWidget *web;
    gchar* title;
    enum {  wsEmpty, wsCreated, wsLoad, wsLoaded, wsClose } status;
    GdkRectangle rect;
    gchar* uri;
    gchar* username;
    gchar* passw;
    gchar* exitScriptSign;
    gchar* exitScript;
    gchar* authScript;
    void (*message)(struct WebWindow* ww, gchar* text);
    void (*onClose)(struct WebWindow* ww);
} WebWindow;


WebWindow *webWindowCreate(gchar* title);

void webWindowClose(WebWindow* ww);

void webWindowSetIcon(WebWindow *ww, GdkPixbuf *icon);

gboolean webWindowLoadUri(WebWindow *ww);

void webWindowShow(WebWindow *ww);

void webWindowSetRect(WebWindow *ww, GdkRectangle* rect);

gboolean webWindowIsActive(WebWindow *ww);


#endif
