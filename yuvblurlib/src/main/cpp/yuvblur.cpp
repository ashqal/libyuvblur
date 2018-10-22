#include <jni.h>
#include <string>

static void superFastBlur2(unsigned char *pixR, unsigned char *pixG, unsigned char *pixB, int width, int height, int radius) {
    if (radius<1) return;

    int wm = width - 1;
    int hm = height - 1;
    int wh = width * height;
    int div = radius + radius + 1;

    unsigned char *r = new unsigned char[wh];
    unsigned char *g = new unsigned char[wh];
    unsigned char *b = new unsigned char[wh];

    int rsum, gsum, bsum, x, y, i, p, p1, p2, yp, yi, yw;

    int *vMIN = new int[std::min(width,height)];
    int *vMAX = new int[std::max(width,height)];

    unsigned char *dv = new unsigned char[256*div];
    for (i = 0; i < 256*div; i++) dv[i] = (i/div);

    yw = yi = 0;

    for (y = 0; y < height; y++) {
        rsum = gsum = bsum = 0;
        for (i = -radius; i <= radius; i++) {
            p = (yi + std::min(wm, std::max(i,0)));
            rsum += pixR[p];
            gsum += pixG[p];
            bsum += pixB[p];
        }
        for (x = 0; x < width; x++) {
            r[yi] = dv[rsum];
            g[yi] = dv[gsum];
            b[yi] = dv[bsum];

            if (y == 0) {
                vMIN[x] = std::min(x + radius + 1, wm);
                vMAX[x] = std::max(x - radius, 0);
            }
            p1 = (yw + vMIN[x]);
            p2 = (yw + vMAX[x]);

            rsum += pixR[p1] - pixR[p2];
            gsum += pixG[p1] - pixG[p2];
            bsum += pixB[p1] - pixB[p2];

            yi++;
        }
        yw += width;
    }

    for (x = 0; x < width; x++) {
        rsum = gsum = bsum = 0;
        yp = -radius * width;
        for (i = -radius; i <= radius; i++) {
            yi = std::max(0, yp) + x;
            rsum += r[yi];
            gsum += g[yi];
            bsum += b[yi];
            yp += width;
        }

        yi = x;

        for (y = 0; y < height; y++) {
            pixR[yi] = dv[rsum];
            pixG[yi] = dv[gsum];
            pixB[yi] = dv[bsum];
            if (x == 0) {
                vMIN[y] = std::min(y + radius + 1, hm) * width;
                vMAX[y] = std::max(y - radius, 0) * width;
            }
            p1 = x + vMIN[y];
            p2 = x + vMAX[y];

            rsum += r[p1] - r[p2];
            gsum += g[p1] - g[p2];
            bsum += b[p1] - b[p2];

            yi += width;
        }
    }

    delete r;
    delete g;
    delete b;

    delete vMIN;
    delete vMAX;
    delete dv;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_asha_yuvblurlib_YuvBlur_nativeBlur(JNIEnv *env, jclass type, jobject oy, jobject ou,
                                            jobject ov, jint w, jint h, jint radius) {

    if (w * h == 0) {
        return;
    }

    unsigned char* plane_y = reinterpret_cast<unsigned char *>(env->GetDirectBufferAddress(oy));
    unsigned char* plane_u = reinterpret_cast<unsigned char *>(env->GetDirectBufferAddress(ou));
    unsigned char* plane_v = reinterpret_cast<unsigned char *>(env->GetDirectBufferAddress(ov));
    int width = w >> 1;
    int height = h >> 1;
    unsigned char* plane_y_tmp = new unsigned char[width * height];

    // scale down
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            plane_y_tmp[x + width * y] = plane_y[2 * x + w * y * 2];
        }
    }

    superFastBlur2(plane_y_tmp, plane_u, plane_v, width, height, radius);

    // scale up
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char value = plane_y_tmp[x + width * y];
            int a1 = 2 * x + width * 2 * y * 2;
            int a2 = 2 * x + 1 + width * 2 * y * 2;
            int a3 = 2 * x +  width * 2 * (y * 2 + 1);
            int a4 = 2 * x + 1 + width * 2 * (y * 2 + 1);
            plane_y[a1] = value;
            plane_y[a2] = value;
            plane_y[a3] = value;
            plane_y[a4] = value;
        }
    }

    delete(plane_y_tmp);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_asha_yuvblurlib_YuvBlur_nativeVersion(JNIEnv *env, jclass type) {
    return env->NewStringUTF("0.0.2");
}