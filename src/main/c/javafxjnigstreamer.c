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
jmethodID methodSetPixels;
jobject pixelFormatInstance;
jobject pixelWriter;

jobject plop;
jmethodID methodNotifyFrame;
jmethodID methodNotifyPlop;

int count = 0;

GstFlowReturn appsinkNewSample(GstAppSink *appsink, gpointer user_data) {

	GstSample * sample = gst_app_sink_pull_sample(appsink);
	GstBuffer * buffer = gst_sample_get_buffer(sample);
	GstMemory * memory = gst_buffer_get_all_memory(buffer);
	//GstMemory * memoryCopy = gst_memory_copy(memory, 0, 230400);
	GstMapInfo mapInfo;
	jbyteArray jba;
	jclass classPlop;
	jmethodID methodPlop;
	int index;

	char myArray[230400]; //{ 0xFF, 0x00, 0x00 };


	gint size = gst_buffer_get_size(buffer);
    JNIEnv * env;
    // double check it's all ok
    int getEnvStat = (*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_6);
	
	for (index = 0; index < size; index++) {
		myArray[index] = 0xFF;
	}

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
	
	/*g_print("plop 2 %d, %d, %d, %d\n", size, mapInfo.data[0], mapInfo.data[1], mapInfo.data[2]);
	for (index = 0; index < 320; index++) {
		g_print("> %d, %d\n", index, mapInfo.data[index]);
	}*/
	g_print("> size = %d\n", size);
	g_print("> count = %d\n", count);

	jba = (*env)->NewByteArray(env, 320*240*3);
  
	g_print("> jba = %p\n", jba);
	g_print("> plop = %p\n", plop);
	(*env)->SetByteArrayRegion(env, jba, 0, 320*240*3, (const jbyte*) mapInfo.data );

	classPlop = (*env)->GetObjectClass(env, plop);
	//g_print("> classPlop = %p\n", classPlop);
	methodNotifyFrame = (*env)->GetMethodID(env, classPlop, "notifyFrame", "([B)V");
	//methodPlop = (*env)->GetMethodID(env, classPlop, "notifyPlop", "()V");

	count++;
	//if (count % 50 == 0) {
        g_print("> %p\n", plop);
        g_print("> classPlop = %p\n", classPlop);
        //g_print("> methodNotifyPlop = %p\n", methodPlop);
	(*env)->CallVoidMethod(env, plop, methodNotifyFrame, jba);
	//(*env)->CallVoidMethod(env, plop, methodNotifyPlop);
	
	gst_memory_unmap(memory, &mapInfo);
        g_print("> unref memory...\n");
	//gst_memory_unref(memory);
        g_print("> unref buffer...\n");
	//gst_buffer_unref(buffer);
        g_print("> unref sample...\n");
	gst_sample_unref(sample);
		//(*env)->CallVoidMethod(env, pixelWriter, methodSetPixels, 0, 0, 320, 240, pixelFormatInstance, jba, 0, 320*3);
	//}
	//(*env)->ReleaseByteArrayElements(env, jba, mapInfo.data, 0);
        g_print("> release...\n");
	//(*env)->ReleaseByteArrayElements(env, jba, (jbyte*) mapInfo.data, JNI_ABORT);
	(*env)->DeleteLocalRef(env, jba);
        g_print("> release done.\n");
	//(*env)->ReleaseByteArrayElements(env, jba, mapInfo.data, 0);
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
	unsigned char myArray[240*320*3];
	int index = 0;
	plop = (*env)->NewGlobalRef(env, obj);
	methodNotifyFrame = (*env)->GetMethodID(env, classPlop, "notifyFrame", "([B)V");
	methodNotifyPlop = (*env)->GetMethodID(env, classPlop, "notifyPlop", "()V");

	for (index = 0; index < 240*320*3; index++) {
		myArray[index] = 0xFF;
	}

	(*env)->GetJavaVM(env, &jvm);

  pixelWriter = (*env)->NewGlobalRef(env, (*env)->CallObjectMethod(env, graphicsContext, methodGetPixelWriter));
  methodSetPixels = (*env)->GetMethodID(env, classPixelWriter, "setPixels", "(IIIILjavafx/scene/image/PixelFormat;[BII)V");
  pixelFormatInstance = (*env)->CallStaticObjectMethod(env, classPixelFormat, methodGetByteRgbInstance);
  
  //jba = (*env)->NewByteArray(env, 320*240*3);
  
    //(*env)->SetByteArrayRegion(env, jba, 0, 320*240*3, (const jbyte*) myArray );
	//(*env)->CallVoidMethod(env, pixelWriter, methodSetPixels, 0, 0, 320, 240, pixelFormatInstance, jba, 0, 320);


  gst_init (0, NULL);

  /*if (argc != 2) {
    g_print ("usage: %s <filename>\n", argv[0]);
    return -1;
  }*/

  pipeline = gst_parse_launch ("videotestsrc ! videoconvert ! video/x-raw,format=RGB ! appsink name=my_sink", &error);
  if (!pipeline) {
    g_print ("Parse error: %s\n", error->message);
	return JNI_FALSE;
  }

  sink = gst_bin_get_by_name (GST_BIN (pipeline), "my_sink");
  //g_object_set (filesrc, "location", argv[1], NULL);
  //g_object_unref (filesrc);
    g_print ("prepare cast...\n");
  appsink = GST_APP_SINK_CAST(sink);
    g_print ("cast done.\n");

	callback.eos = NULL;
	callback.new_preroll = NULL;
  callback.new_sample = NULL;
  callback.new_sample = &appsinkNewSample;
    g_print ("new_sample set\n");

  gst_app_sink_set_callbacks (appsink,
	  &callback,
	  NULL,
	  NULL);
    g_print ("gst_app_sink_set_callbacks done\n");

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  bus = gst_element_get_bus (pipeline);

	return JNI_TRUE;
}

