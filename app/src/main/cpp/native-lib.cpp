#include <jni.h>
#include <string>
#include <pthread.h>
#include "ImageProcess.h"
#include <android/bitmap.h>
#include <mutex>

array2d<uint8_t> *img; //相机照片
array2d<uint8_t> *tmp; //lines_search结果
array2d<uint32_t> *bmp;    //输出窗口
uint32_t *bmp_ptr;
line_search_para *ls_para;
extern "C"
JNIEXPORT void JNICALL
Java_com_example_sh3dscaner_ImageProcess_update(JNIEnv *env, jclass thiz, jobject img_in) {
    //jlong Buf_Cap = env->GetDirectBufferCapacity(img_in);
    //assert(img->prod() == Buf_Cap);
    auto *in_ptr = (uint8_t*)env->GetDirectBufferAddress(img_in);
    if(update(in_ptr) != 0) //BUSY
        return;
    // TODO: too slow, want change to use OpenGL
    /*pthread_mutex_lock(&mutex);
    uint32_t *bmp_ptr_tmp = bmp->data;
    uint8_t *tmp_ptr = tmp->data;
    uint64_t prod = bmp->prod();
    for(uint64_t i=0; i<prod; i++) {
        *bmp_ptr_tmp++ = (*tmp_ptr++)*0x00010101 + 0xff000000;
    }
    pthread_mutex_unlock(&mutex);*/
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_sh3dscaner_ImageProcess_jni_1init(JNIEnv *env, jclass thiz, jobject para,
                                                   jobject bmp_out) {
    jfieldID id;
    jclass c = env->FindClass("com/example/sh3dscaner/ImageProcess$init_para");
    id = env->GetFieldID(c, "in_width", "I");
    jint in_width = env->GetIntField(para, id);
    id = env->GetFieldID(c, "in_height", "I");
    jint in_height = env->GetIntField(para, id);
    id = env->GetFieldID(c, "start_x", "I");
    jint start_x = env->GetIntField(para, id);
    id = env->GetFieldID(c, "start_y", "I");
    jint start_y = env->GetIntField(para, id);
    id = env->GetFieldID(c, "rad_start", "F");
    jfloat rad_start = env->GetFloatField(para, id);
    id = env->GetFieldID(c, "rad_step", "F");
    jfloat rad_step = env->GetFloatField(para, id);
    id = env->GetFieldID(c, "rad_N", "I");
    jint rad_N = env->GetIntField(para, id);
    id = env->GetFieldID(c, "N_line", "I");
    jint n_line = env->GetIntField(para, id);
    id = env->GetFieldID(c, "N_length", "I");
    jint n_length = env->GetIntField(para, id);
    id = env->GetFieldID(c, "N_thread", "I");
    jint n_thread = env->GetIntField(para, id);
    AndroidBitmapInfo bmpInfo = AndroidBitmapInfo{0};
    AndroidBitmap_getInfo(env, bmp_out, &bmpInfo);
    AndroidBitmap_lockPixels(env, bmp_out, (void**)&bmp_ptr);
    tmp = new array2d<uint8_t>(rad_N, n_line, true);
    bmp = new array2d<uint32_t>(bmpInfo.width, bmpInfo.height, false);
    bmp->set_ptr(bmp_ptr);
    ls_para = new line_search_para{tmp,                                                   //out
                                   point_u32(start_x, start_y),                     //point
                                   rads_range{rad_start, rad_step, {0,static_cast<uint32_t>(rad_N)}},
                                   int_range{0, static_cast<uint32_t>(n_line)},   //N_line
                                   static_cast<uint32_t>(n_length)};                      //N_length
    init(n_thread, ls_para, in_width, in_height);
}
