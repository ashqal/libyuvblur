package com.asha.yuvblurlib;

import java.nio.ByteBuffer;

/**
 * Created by hzqiujiadi on 2018/7/12.
 * hzqiujiadi ashqalcn@gmail.com
 */
public class YuvBlur {

    static {
        System.loadLibrary("yuvblur");
    }

    public static void blur(ByteBuffer y, ByteBuffer u, ByteBuffer v, int w, int h, int radius) {
        nativeBlur(y, u, v, w, h, radius);
    }

    public static String version() {
        return nativeVersion();
    }

    private static native String nativeVersion();

    private static native void nativeBlur(ByteBuffer y, ByteBuffer u, ByteBuffer v, int w, int h, int radius);

}
