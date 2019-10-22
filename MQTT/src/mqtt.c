/*---------------------- Added header --------------------*/
#include <tizen.h>
#include <service_app.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <MQTTClient.h>


#define ADDRESS     "tcp://broker.hivemq.com:1883"
#define CLIENTID    "ExampleClientSub6"
#define QOS         1
#define TIMEOUT     10000L
volatile MQTTClient_deliveryToken deliveredtoken;

#define PAYLOAD     "Hello World!"
#define TOPIC       "/iot/s3/test"
#define TOPIC2       "/demo/test/iot"

#include "log.h"

/*-------------------------- Done header ----------------------*/
MQTTClient *client1;

void change_view(void *data, Evas_Object *obj, void *event_info);
typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *button;
	Evas_Object *buttonSub;
	Evas_Object *input;
	bool first_view;
} appdata_s;

Evas_Object *label;
Evas_Object *label2;

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void change_text_on_label_mssg_arrived(char msg[], char topic[]){
	/* Change of message text */
	 const int MAX_TEXT_SIZE  = 1024;
	 char text[MAX_TEXT_SIZE] = {0,};
	 char topic_text[MAX_TEXT_SIZE] = {0,};
	 snprintf(text, MAX_TEXT_SIZE, "<font align=center> %s </font>", msg);

	 elm_object_text_set(label2, text);

	/* Change text of topic */
	 snprintf(topic_text, MAX_TEXT_SIZE, "<font align=center> %s </font>", topic);
	elm_object_text_set(label, topic_text);

}

/*-------------------------------------------------- MQTT code ----------------------------------------------------------*/

static void _delivered(void *context, MQTTClient_deliveryToken dt)
{
    DBG("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

static int __msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    char msg_str[1000];
    strncpy(msg_str, message->payload, message->payloadlen);
    msg_str[message->payloadlen] = '\0';

    /* 	Ispis poruke na satu */
    change_text_on_label_mssg_arrived(msg_str, topicName);
    printf("%s",msg_str);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    return 1;
}

static void __connlost(void *context, char *cause)
{
    DBG("Connection lost\n");
    DBG("     cause: %s\n", cause);
}


static bool service_app_create(void *user_data)
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 1020;
    conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(client, NULL, __connlost, __msgarrvd, _delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        DBG("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    MQTTClient_subscribe(client, TOPIC, QOS);
    client1 = client;
    return true;
}
static int __msgsend()
{
	    MQTTClient_message pubmsg = MQTTClient_message_initializer;
	    MQTTClient_deliveryToken token;
	    MQTTClient_setCallbacks(client1, NULL, __connlost, __msgarrvd, _delivered);
	    pubmsg.payload = PAYLOAD;
	    pubmsg.payloadlen = strlen(PAYLOAD);
	    pubmsg.qos = QOS;
	    pubmsg.retained = 0;
	    deliveredtoken = 0;
	    MQTTClient_publishMessage(client1, TOPIC, &pubmsg, &token);
	    printf("Waiting for publication of %s\n"
	            "on topic %s for client with ClientID: %s\n",
	            PAYLOAD, TOPIC, CLIENTID);
	    while(deliveredtoken != token);

	    return MQTTCLIENT_SUCCESS;
}

static void service_app_terminate(void *user_data)
{
	DBG("service_app_terminate");
}

static void service_app_control(app_control_h app_control, void *user_data)
{
	DBG("service_app_control");

	if (app_control == NULL) {
		ERR("app_control is NULL");
		return;
	}
}

/*---------------------------------------------------- Done MQTT code ---------------------------------------------------------*/
void
 clicked_cb(void *data, Evas_Object *obj, void *event_info)
 {
	appdata_s *ad = data;
	if(ad->first_view == true){
		__msgsend();
	}
	else{
		Eina_Bool Empty = elm_entry_is_empty(ad->input);
		if(Empty == EINA_FALSE){
			//TODO: Implement da je nevalidan tekst
			//return;
		}
		elm_entry_select_all(ad->input);
		const char *selection;
		selection = elm_entry_selection_get(ad->input);
		MQTTClient_subscribe(client1, selection, QOS);
	}

 }
void change_view(void *data, Evas_Object *obj, void *event_info)
 {
	appdata_s *ad = data;
	if(ad->first_view == true){
		evas_object_hide(label);
		evas_object_hide(label2);
		elm_object_text_set(ad->button, "Subscirbe");
		elm_object_text_set(ad->buttonSub,"Send");
		evas_object_show(ad->input);
	}else{
		evas_object_show(label);
		evas_object_show(label2);
		elm_object_text_set(ad->button, "Send");
		elm_object_text_set(ad->buttonSub,"Subscribe");
		evas_object_hide(ad->input);
	}
	ad->first_view = ! ad->first_view;
 }
static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	/* Create and initialize elm_win.
	   elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	   elm_conformant is mandatory for base gui to have proper size
	   when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Butoon subscirbe*/

		ad->buttonSub = elm_button_add(ad->conform);
		evas_object_resize(ad->buttonSub, 90, 90);
		evas_object_move(ad->buttonSub, 135, 0);
		//elm_object_text_set(ad->buttonSub, "S");
		//elm_object_style_set(ad->buttonSub, "popup/circle/right");
		evas_object_smart_callback_add(ad->buttonSub, "clicked", change_view, ad);
		//evas_object_show(ad->buttonSub);

		evas_object_size_hint_align_set(ad->buttonSub, -1.0, -1.0);
		evas_object_size_hint_weight_set(ad->buttonSub, 1.0 , 1.0);
		elm_object_text_set(ad->buttonSub,"Subscribe");		//_UIB_LOCALE()
		elm_object_style_set(ad->buttonSub,"bottom");
		evas_object_show(ad->buttonSub);
		ad->first_view=true;

	/* Topic Label */
	/* Create an actual view of the base gui.
	   Modify this part to change the view. */

	label = elm_label_add(ad->conform);

	evas_object_resize(label, 180,80);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	//elm_object_content_set(ad->conform, label);
	evas_object_move(label, (360-240)/2, 360-270);
	evas_object_show(label);
	elm_object_text_set(label, "<align=center>Topic</align>");

	/* Message Label 2 */

	label2= elm_label_add(ad->conform);
	evas_object_resize(label2, 320,100);
	evas_object_size_hint_weight_set(label2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label2, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_move(label2, (360-320)/2, 360-180);
	evas_object_show(label2);
	elm_object_text_set(label2, "<font_size=35, align=center>Message</font_size>");

	/* Input text for subscribe topic */

	ad->input = elm_entry_add(ad->conform);
	elm_object_text_set(ad->input,"Name of topic");
	//elm_entry_input_hint_set(ad->input, "Name of topic");
	elm_entry_single_line_set(ad->input, EINA_FALSE);
	elm_entry_password_set(ad->input, EINA_FALSE);
	elm_entry_editable_set(ad->input, EINA_TRUE);
	elm_entry_scrollable_set(ad->input, EINA_FALSE);
	evas_object_size_hint_align_set(ad->input, -1.0, -1.0);
	evas_object_size_hint_weight_set(ad->input, 1.0, 1.0);
	elm_object_disabled_set(ad->input, EINA_FALSE);
	evas_object_resize(ad->input, 180,80);
	//evas_object_size_hint_weight_set(ad->input, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//evas_object_size_hint_align_set(ad->input, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_move(ad->input, (360-240)/2, 360-270);
	//evas_object_show(ad->input);
	//elm_box_pack_end(vc->box1, vc->entry1);
	evas_object_hide(ad->input);



	/* Butoon */

	ad->button = elm_button_add(ad->conform);
	evas_object_resize(ad->button, 90, 90);
	evas_object_move(ad->button,(360-90)/2, 360-90);
	elm_object_text_set(ad->button, "Send");
	elm_object_style_set(ad->button, "bottom");
	evas_object_smart_callback_add(ad->button, "clicked", clicked_cb, ad);
	evas_object_show(ad->button);
	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}
static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);
	service_app_create(data);
	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	service_app_control(app_control, data);
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	DBG("APPPP terminate");
	service_app_terminate(data);
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;


	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);


	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}


	return ret;
}
