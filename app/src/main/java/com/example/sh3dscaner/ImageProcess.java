/*
 * Copyright 2020 Xu Ruijun
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.sh3dscaner;
import android.graphics.Bitmap;
import android.util.Size;

import java.nio.ByteBuffer;

public class ImageProcess {
    static {
        System.loadLibrary("native-lib");
    }
    public static class init_para {
        int in_width;
        int in_height;
        int start_x;
        int start_y;
        float rad_start;
        float rad_step;
        int rad_N;
        int N_line;
        int N_length;
        int N_thread;
    }
    public static native void jni_init(init_para para, Bitmap bmp_out);
    public static native void update(ByteBuffer img_in);
}
