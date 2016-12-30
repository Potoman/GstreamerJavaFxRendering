#include <stdio.h>
#include "com_potoman_Plop.h"

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#ifdef TESTOPT
#error "TESTOPT is set!"
#endif

JNIEXPORT jstring JNICALL Java_com_potoman_Plop_sayHello( JNIEnv *env, jobject obj ) {
	jstring value;           /* the return value */

	char buf[40];            /* working buffer (really only need 20 ) */


	sprintf ( buf, "%s", "Hello NAR World!" );

	value = (*env)->NewStringUTF( env, buf );

	return value;
}

JNIEXPORT jint JNICALL Java_com_potoman_Plop_timesHello
  (JNIEnv *env, jobject obj, jint x, jint y) {
	return x * y;
}

JavaVM* jvm;

// First try :
jmethodID methodSetPixels;
jobject pixelFormatInstance;
jobject pixelWriter;

// Second try successfull :
jobject plop;
jmethodID methodNotifyFrame;
jmethodID methodNotifyPlop;

int count = 0;

GstFlowReturn appsinkNewSample(GstAppSink *appsink, gpointer user_data) {

	GstSample * sample = gst_app_sink_pull_sample(appsink);
	GstBuffer * buffer = gst_sample_get_buffer(sample);
	GstMemory * memory = gst_buffer_get_all_memory(buffer);
	GstMapInfo mapInfo;
	jbyteArray jba;
	jclass classPlop;
	jmethodID methodPlop;
	int index;

	gint size = gst_buffer_get_size(buffer);
    JNIEnv * env;
    // double check it's all ok
    int getEnvStat = (*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_6);

	if (getEnvStat == JNI_EDETACHED) {
        g_print("> GetEnv: not attached\n");
        if ((*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL) != 0) {
			g_print("> Failed to attach\n");
        }
    } else if (getEnvStat == JNI_OK) {
        g_print("> GetEnv: ok\n");
        //
    } else if (getEnvStat == JNI_EVERSION) {
        g_print("> GetEnv: version not supported\n");
    }

	gst_memory_map(memory, &mapInfo, GST_MAP_READ);
	
	jba = (*env)->NewByteArray(env, 320*240*3);
  
	(*env)->SetByteArrayRegion(env, jba, 0, 320*240*3, (const jbyte*) mapInfo.data );

	classPlop = (*env)->GetObjectClass(env, plop);
	
	methodNotifyFrame = (*env)->GetMethodID(env, classPlop, "notifyFrame", "([B)V");

	(*env)->CallVoidMethod(env, plop, methodNotifyFrame, jba);
	
	/*
	
	Important to mention :
	
	At first, I try to set directly canvas pixel here, to avoid user to manage it.
	But... I don't know why but... If I use setPixels method here, it doesn't work and the application crash.
	So this is why I let the code to init jni method for setPixels and other field but I keep the second method : notify byte array of frame to java
	and this is in the java that I setPixels of canvas.
	To have better perform, it could be better to pass size of frame by notifyFrame method. I let the user to that. :)
	
	*/
	
	gst_memory_unmap(memory, &mapInfo);
	gst_sample_unref(sample);
	(*env)->DeleteLocalRef(env, jba);

	return GST_FLOW_OK;
}


JNIEXPORT jboolean JNICALL Java_com_potoman_Plop_initCanvas
  (JNIEnv *env, jobject obj, jobject canvas) {

	GstElement *pipeline;
	GstElement * sink;
	GstMessage *msg;
	GstBus *bus;
	GError *error = NULL;
	GstAppSink * appsink;
	GstAppSinkCallbacks callback;

	jclass classPixelFormat = (*env)->FindClass(env, "javafx/scene/image/PixelFormat");
	jclass classPixelWriter = (*env)->FindClass(env, "javafx/scene/image/PixelWriter");
	jclass classGraphicsContext = (*env)->FindClass(env, "javafx/scene/canvas/GraphicsContext");
	jclass classCanvas = (*env)->GetObjectClass(env, canvas);
	jclass classPlop = (*env)->GetObjectClass(env, obj);

	jmethodID methodGetGraphicsContext2D = (*env)->GetMethodID(env, classCanvas, "getGraphicsContext2D", "()Ljavafx/scene/canvas/GraphicsContext;");

	jobject graphicsContext = (*env)->CallObjectMethod(env, canvas, methodGetGraphicsContext2D);

	jmethodID methodGetPixelWriter = (*env)->GetMethodID(env, classGraphicsContext, "getPixelWriter", "()Ljavafx/scene/image/PixelWriter;");

	jmethodID methodGetByteRgbInstance = (*env)->GetStaticMethodID(env, classPixelFormat, "getByteRgbInstance", "()Ljavafx/scene/image/PixelFormat;");
	jbyteArray jba;
	
	int index = 0;
	plop = (*env)->NewGlobalRef(env, obj);
	methodNotifyFrame = (*env)->GetMethodID(env, classPlop, "notifyFrame", "([B)V");
	methodNotifyPlop = (*env)->GetMethodID(env, classPlop, "notifyPlop", "()V");

	(*env)->GetJavaVM(env, &jvm);

	pixelWriter = (*env)->NewGlobalRef(env, (*env)->CallObjectMethod(env, graphicsContext, methodGetPixelWriter));
	methodSetPixels = (*env)->GetMethodID(env, classPixelWriter, "setPixels", "(IIIILjavafx/scene/image/PixelFormat;[BII)V");
	pixelFormatInstance = (*env)->CallStaticObjectMethod(env, classPixelFormat, methodGetByteRgbInstance);

	gst_init (0, NULL);

	pipeline = gst_parse_launch ("videotestsrc ! videoconvert ! video/x-raw,format=RGB ! appsink name=my_sink", &error);
	if (!pipeline) {
		g_print ("Parse error: %s\n", error->message);
		return JNI_FALSE;
	}

	sink = gst_bin_get_by_name (GST_BIN (pipeline), "my_sink");
	
	appsink = GST_APP_SINK_CAST(sink);

	callback.eos = NULL;
	callback.new_preroll = NULL;
	callback.new_sample = NULL;
	callback.new_sample = &appsinkNewSample;

	gst_app_sink_set_callbacks (appsink,
			&callback,
			NULL,
			NULL);

	gst_element_set_state (pipeline, GST_STATE_PLAYING);

	bus = gst_element_get_bus (pipeline);

	return JNI_TRUE;
}

