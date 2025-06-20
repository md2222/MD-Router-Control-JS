#include "webwin.h"
#include <JavaScriptCore/JSStringRef.h>


extern gchar* appName;
static GdkRectangle defRect = { 200, 100, 1280, 900 };


static void message(struct WebWindow* ww, gchar* text)
{
    printf("%s\n", text);
}


static gboolean onKeyPress(GtkWidget* win, GdkEventKey* event, WebWindow* ww)
{
    if (event->keyval == GDK_KEY_Escape)
    {
        gtk_window_close(GTK_WINDOW(win));

        return TRUE;
    }
    
    return FALSE;
}


static void onLoadChanged (WebKitWebView  *web_view, WebKitLoadEvent load_event, WebWindow* ww)
{
    switch (load_event) 
    {
    case WEBKIT_LOAD_STARTED:
        /* New load, we have now a provisional URI */
//        provisional_uri = webkit_web_view_get_uri (web_view);
        /* Here we could start a spinner or update the
         * location bar with the provisional URI */
        //printf("WEBKIT_LOAD_STARTED\n");
        break;
    case WEBKIT_LOAD_REDIRECTED:
//        redirected_uri = webkit_web_view_get_uri (web_view);
        break;
    case WEBKIT_LOAD_COMMITTED:
        /* The load is being performed. Current URI is
         * the final one and it won't change unless a new
         * load is requested or a navigation within the
         * same page is performed */
//        uri = webkit_web_view_get_uri (web_view);
        break;
    case WEBKIT_LOAD_FINISHED:
        /* Load finished, we can now stop the spinner */
        printf("Event:  WEBKIT_LOAD_FINISHED    status=%d\n", ww->status);
        
        if (ww->status == wsLoad)
        {
            if (!ww->authScript)
            {
                ww->message(ww, "Authorization script is empty.");
            }
            else
            {
                webkit_web_view_run_javascript (web_view,
                                        ww->authScript,
                                        NULL, // GCancellable *cancellable,
                                        NULL, // onJavascriptFinished, // GAsyncReadyCallback callback,
                                        NULL // web // gpointer user_data
                                    );
                                    
                secret_password_free(ww->authScript);
                ww->authScript = 0;
            }
            
            ww->status = wsLoaded;
        }
        // when closing, after moving to auth page
        else if (ww->status >= wsClose)
            webWindowClose(ww);
                            
        break;
    }
}

// we requested a string as a sign of the script
static void onCloseJavascriptFinished(GObject *object, GAsyncResult *result, gpointer data)
{
    //printf("onCloseJavascriptFinished:  \n");
    WebKitJavascriptResult *js_result;
    JSValueRef              value;
    JSGlobalContextRef      context;
    GError                 *error = NULL;

    WebWindow* ww = (WebWindow*)data;

    js_result = webkit_web_view_run_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
    
    if (!js_result) 
    {
        printf("Error running javascript\n");
        //g_warning ("Error running javascript: %s", error->message);
        g_error_free (error);
        webWindowClose(ww);
        return;
    }

    context = webkit_javascript_result_get_global_context (js_result);
    value = webkit_javascript_result_get_value (js_result);
    
    webkit_javascript_result_unref (js_result);
    
    if (JSValueIsString (context, value)) 
    {
        JSStringRef js_str_value;
        gchar      *str_value;
        gsize       str_length;

        js_str_value = JSValueToStringCopy (context, value, NULL);
        str_length = JSStringGetMaximumUTF8CStringSize (js_str_value);
        str_value = (gchar *)g_malloc (str_length);
        JSStringGetUTF8CString (js_str_value, str_value, str_length);
        JSStringRelease (js_str_value);
        g_print ("Script result: \"%s\"\n", str_value);
        g_free (str_value);
        
        webkit_web_view_run_javascript (ww->web,
                            ww->exitScript,
                            NULL, // GCancellable *cancellable,
                            NULL, // onJavascriptFinished, // GAsyncReadyCallback callback,
                            NULL // wv // gpointer user_data
                        );
    } 
    else 
    {
        //printf("Not JSValueIsString\n");
        printf("Exit script not found in current content\n");
        // no string - no script, close
        webWindowClose(ww);
    }
}


static gboolean onWinClose(GtkWidget *win, GdkEventKey* event, WebWindow *ww)
{
    if (ww->status == wsClose)
    {
        if (ww->onClose)  ww->onClose(ww);
        free(ww);
        return FALSE;
    }
    
    ww->status = wsClose;

    webkit_web_view_run_javascript (ww->web, ww->exitScriptSign, NULL, onCloseJavascriptFinished, ww);
    
    return TRUE;  // handled, no propagation
}


WebWindow *webWindowCreate(gchar* title)
{
    WebWindow* webWin = malloc(sizeof(WebWindow));
    webWin->win = 0;
    webWin->web = 0;
    webWin->title = title;
    webWin->status = wsEmpty;
    //webWin->rect;
    webWin->uri = 0;
    webWin->exitScriptSign = 0;
    webWin->exitScript = 0;
    webWin->authScript = 0;
    webWin->message = message;
    webWin->onClose = 0;
    
    GtkWindow *win = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(win), defRect.width, defRect.height);
    if (title)  
        gtk_window_set_title(GTK_WINDOW(win), title);

    g_signal_connect_after(win, "key_press_event", G_CALLBACK(onKeyPress), webWin);
    g_signal_connect(win, "delete-event", G_CALLBACK(onWinClose), webWin);
    
    webWin->win = win;
    webWin->rect = defRect;

    // Websites will not store any data in the client storage. This is normally used to implement private instances.
    WebKitWebContext *context = webkit_web_context_new_ephemeral();
    GtkWidget *web = WEBKIT_WEB_VIEW(webkit_web_view_new_with_context(context));

    g_signal_connect(web, "load-changed", G_CALLBACK(onLoadChanged), webWin);  //+
    
    webWin->web = web;

    gtk_container_add(GTK_CONTAINER(win), GTK_WIDGET(web));

    gtk_widget_grab_focus(GTK_WIDGET(web));
    gtk_widget_show_all(GTK_WIDGET(win));
    
    webWin->status = wsCreated;

    return webWin;
}


void webWindowClose(WebWindow* ww)
{
    if (!ww)  return;

    ww->status = wsClose;
    gtk_window_close(ww->win);
}


void webWindowSetIcon(WebWindow *ww, GdkPixbuf *icon)
{
    if (!ww || !icon)  return;
    
    gtk_window_set_icon (ww->win, icon);
}


void webWindowSetRect(WebWindow *ww, GdkRectangle* rect)
{
    if (!ww || !rect)  return;
    
    if (rect->x * rect->y > 0)
        gtk_window_move(ww->win, rect->x, rect->y);
    if (rect->width * rect->height > 0)
        gtk_window_resize(ww->win, rect->width, rect->height);
        
    ww->rect = *rect;    
}


GtkWidget *webWindowGetWeb(WebWindow *ww)
{
    if (!ww)  return NULL;
    
    return ww->web;
}


gboolean webWindowLoadUri(WebWindow *ww)
{
    if (!ww)  return FALSE;
    
    if (!ww->uri)
    {
        ww->message(ww, "URI not set.");
        return FALSE;
    }

    if (!ww->authScript || !ww->exitScriptSign || !ww->exitScript)
    {
        ww->message(ww, "Not all scripts are set.");
        return FALSE;
    }

    ww->status = wsLoad;

    webkit_web_view_load_uri(ww->web, ww->uri);
    
    return TRUE;
}


void webWindowShow(WebWindow *ww)
{
    if (!ww)  return;
    gtk_window_present(ww->win);
}


gboolean webWindowIsActive(WebWindow *ww)
{
    if (!ww)  return FALSE;
    return gtk_window_is_active (ww->win);
}
