#ifndef UIWEBBROWSER_H__
#define UIWEBBROWSER_H__

bool uiWebBrowser_goto_url(const char* url);
void uiWebBrowser_get_web_reset_data(const char * url);
void uiWebBrowser_menu_service(void);
void uiWebBrowser_go_back();

//internal
void uiWebBrowser_exit_handling(void);
bool uiWebBrowser_initialize_service(int page_width, int page_height, int page_view_top, int page_view_left, int page_view_bottom, int page_view_right );
void uiWebBrowser_update_screen_position(int x, int y, float z_layer);
void uiWebBowser_generate_events(bool * eventActivated);
void uiWebBrowser_render(void);
void uiWebBrowser_set_cursor(void);
bool uiWebBrowser_has_focus(void);
#endif

/* End of File */
