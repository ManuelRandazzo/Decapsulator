#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_calibrazione_touch(lv_event_t * e);
extern void action_logger_page_init(lv_event_t * e);
extern void action_logger_page_deinit(lv_event_t * e);
extern void action_jogger_page_init(lv_event_t * e);
extern void action_jogger_page_deinit(lv_event_t * e);
extern void action_conf_wi_fi_page_init(lv_event_t * e);
extern void action_conf_wi_fi_page_deinit(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/