/*
 * interface.c
 *
 *  Created on: 5 февр. 2021 г.
 *      Author: Mikhail
 */
#include "interface.h"

#include "lvgl.h"

#define MP 2

static void lv_spinbox_increment_event_cb(lv_obj_t * btn, lv_event_t e)
{
	lv_obj_t * parent = lv_obj_get_parent(btn);
	lv_obj_t * btn1 = lv_obj_get_child_back(parent, NULL);
	lv_obj_t * spinbox = lv_obj_get_child_back(parent, btn1);

    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox);
    }
}

static void lv_spinbox_decrement_event_cb(lv_obj_t * btn, lv_event_t e)
{
	lv_obj_t * parent = lv_obj_get_parent(btn);
	lv_obj_t * btn1 = lv_obj_get_child_back(parent, NULL);
	lv_obj_t * spinbox = lv_obj_get_child_back(parent, btn1);

	if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox);
    }
}

static void create_spinbox(lv_obj_t * parent){
		lv_obj_t *container = lv_cont_create(parent, NULL);
		lv_obj_set_width(container, 250);
		lv_cont_set_layout(container, LV_LAYOUT_CENTER);
		lv_cont_set_fit(container, LV_FIT_TIGHT);
		lv_obj_set_height_margin(container, MP);
		lv_obj_set_width_margin(container, MP);
		lv_obj_set_height_fit(container, MP);
		lv_obj_set_width_fit(container, MP);


	    lv_obj_t * btn = lv_btn_create(container, NULL);
//		lv_obj_set_height_margin(btn, MP);
//		lv_obj_set_width_margin(btn, MP);
//		lv_obj_set_height_fit(btn, MP);
//		lv_obj_set_width_fit(btn, MP);


		lv_obj_t *spinbox = lv_spinbox_create(container, NULL);
//		lv_obj_set_height_margin(spinbox, MP);
//		lv_obj_set_width_margin(spinbox, MP);
//		lv_obj_set_height_fit(spinbox, MP);
//		lv_obj_set_width_fit(spinbox, MP);

		lv_spinbox_set_digit_format(spinbox, 5, 0);
		lv_spinbox_set_range(spinbox, 0, 99999);
		lv_spinbox_set_step(spinbox, 50);

		lv_textarea_set_cursor_hidden(spinbox, true);

		lv_obj_set_width(spinbox, 125);
		lv_obj_align(spinbox, NULL, LV_ALIGN_CENTER, 0, 0);

	    lv_coord_t h = lv_obj_get_height(spinbox);


	    lv_obj_set_size(btn, h, h);
	    lv_theme_apply(btn, LV_THEME_SPINBOX_BTN);
	    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);
	    lv_obj_set_event_cb(btn, lv_spinbox_increment_event_cb);



	    btn = lv_btn_create(container, btn);
	    lv_obj_set_size(btn, h, h);
	    lv_theme_apply(btn, LV_THEME_SPINBOX_BTN);
	    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_MINUS);
	    lv_obj_set_event_cb(btn, lv_spinbox_decrement_event_cb);

//		lv_obj_set_height_margin(btn, MP);
//		lv_obj_set_width_margin(btn, MP);
//		lv_obj_set_height_fit(btn, MP);
//		lv_obj_set_width_fit(btn, MP);

}

static void cancel_pouring_cb(struct _lv_obj_t * obj, lv_event_t event){
	if (event == LV_EVENT_VALUE_CHANGED){
		//TODO (mikhailche): include code to cancel pouring process
	}
}

static void create_pouring_popup(lv_obj_t * parent){
	lv_obj_t * popup = lv_msgbox_create(parent, NULL);

	lv_msgbox_set_text(popup, "Pouring...");
	static const char * btns2[] = {"STOP", ""};
	lv_msgbox_add_btns(popup, btns2);

	lv_msgbox_start_auto_close(popup, 3000); // TODO: parametrize in some way
	lv_obj_set_event_cb(popup, cancel_pouring_cb);
}


static void create_pouring_popup_cb(struct _lv_obj_t *obj, lv_event_t event) {
	if (event == LV_EVENT_CLICKED) {
		create_pouring_popup(lv_scr_act());
	}
}

static void create_pour_column(lv_obj_t * parent, const char* ml){
	lv_obj_t *container = lv_cont_create(parent, NULL);
	lv_obj_set_width(container, 250);
	lv_cont_set_layout(container, LV_LAYOUT_CENTER);
	lv_cont_set_fit(container, LV_FIT_TIGHT);
	lv_obj_set_height_margin(container, MP);
	lv_obj_set_width_margin(container, MP);
	lv_obj_set_height_fit(container, MP);
	lv_obj_set_width_fit(container, MP);


	create_spinbox(container);

	lv_obj_t * btn = lv_btn_create(container, NULL);
	lv_obj_set_width(btn, 125);

	lv_obj_t * label = lv_label_create(btn, NULL);
	lv_label_set_text(label, ml);

	lv_obj_set_event_cb(btn, create_pouring_popup_cb);

}


struct createInterfaceResp create_interface() {
	struct createInterfaceResp response;

	lv_obj_t *window = lv_win_create(lv_scr_act(), NULL);

	lv_win_set_title(window, "Got milk?");

	lv_win_add_btn(window, LV_SYMBOL_SAVE);
	lv_win_set_scrollbar_mode(window, LV_SCROLLBAR_MODE_OFF);

	lv_win_set_layout(window, LV_LAYOUT_PRETTY_TOP);

	create_pour_column(window, "25");
	create_pour_column(window, "50");
	create_pour_column(window, "150");

	return response;
}

