#include <jni.h>
#include <string>

static void superFastBlur(unsigned char *pix, int w, int h, int blurRadius) {
    int centx = w >> 1;
    int centy = h >> 1;
    int circleRadius = 0;
    int smoothRadius = 5;
    const int byteCount = 1;
    int radius = (blurRadius / 10) * 2;
    int minW_h = (smoothRadius + circleRadius);//min(w,h)/2;
    //bool ischange = false;
    int div;
    int wm, hm, wh;
    int *vMIN, *vMAX;
    unsigned char *r, *g, *b, *dv;
    int rsum, gsum, bsum, x, y, i, p, p1, p2, yp, yi, yw;

    int posx = 0;
    int posy = 0;
    if (radius < 1) return;

    wm = w - 1;
    hm = h - 1;
    wh = w * h;
    div = radius + radius + 1;
    vMIN = (int *) malloc(sizeof(int) * std::max(w, h));
    vMAX = (int *) malloc(sizeof(int) * std::max(w, h));
    //r = (unsigned char *)malloc(sizeof(unsigned char) * wh);
    //g = (unsigned char *)malloc(sizeof(unsigned char) * wh);
    b = (unsigned char *) malloc(sizeof(unsigned char) * wh);
    dv = (unsigned char *) malloc(sizeof(unsigned char) * 256 * div);

    for (i = 0; i < 256 * div; i++) {
        dv[i] = (i / div);
    }

    yw = yi = 0;

    for (y = 0; y < h; y++) {
        rsum = gsum = bsum = 0;
        for (i = -radius; i <= radius; i++) {
            p = (yi + std::min(wm, std::max(i, 0))) * byteCount;
            bsum += pix[p];
            //gsum += pix[p + 1];
            //rsum += pix[p + 2];
        }
        for (x = 0; x < w; x++) {
            //r[yi] = dv[rsum];
            //g[yi] = dv[gsum];
            b[yi] = dv[bsum];

            if (y == 0) {
                vMIN[x] = std::min(x + radius + 1, wm);
                vMAX[x] = std::max(x - radius, 0);
            }
            p1 = (yw + vMIN[x]) * byteCount;
            p2 = (yw + vMAX[x]) * byteCount;

            bsum += pix[p1] - pix[p2];//上一个点后移动１位数
            //gsum += pix[p1 + 1] - pix[p2 + 1];
            //rsum += pix[p1 + 2] - pix[p2 + 2];

            yi++;
        }
        yw += w;
    }

    for (x = 0; x < w; x++) {
        //跳过非模糊区域 即圆内
        posx = x;

        rsum = gsum = bsum = 0;
        yp = -radius * w;
        for (i = -radius; i <= radius; i++) {
            yi = std::max(0, yp) + x;
            //rsum += r[yi];
            //gsum += g[yi];
            bsum += b[yi];
            yp += w;
        }
        yi = x;
        for (y = 0; y < h; y++) {
            //跳过非模糊区域 即圆内
            //posx = x;
            posy = y;//h-y;
            if ((posx - centx) * (posx - centx) + (posy - centy) * (posy - centy) <
                circleRadius * circleRadius) {
                //yi += w;
                //continue;
            } else {

                pix[yi * byteCount] = dv[bsum];
                //pix[yi * byteCount + 1] = dv[gsum];
                //pix[yi * byteCount + 2] = dv[rsum];
            }

            if (x == 0) {
                vMIN[y] = std::min(y + radius + 1, hm) * w;
                vMAX[y] = std::max(y - radius, 0) * w;
            }

            p1 = x + vMIN[y];
            p2 = x + vMAX[y];

            //rsum += r[p1] - r[p2];
            //gsum += g[p1] - g[p2];
            bsum += b[p1] - b[p2];

            yi += w;
        }
    }

    //free(r);
    //free(g);
    free(b);

    free(vMIN);
    free(vMAX);
    free(dv);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_asha_yuvblurlib_YuvBlur_nativeBlur(JNIEnv *env, jclass type, jobject y, jobject u,
                                            jobject v, jint w, jint h, jint radius) {

    unsigned char *plane_y = reinterpret_cast<unsigned char *>(env->GetDirectBufferAddress(y));
    unsigned char* plane_u = reinterpret_cast<unsigned char *>(env->GetDirectBufferAddress(u));
    unsigned char* plane_v = reinterpret_cast<unsigned char *>(env->GetDirectBufferAddress(v));
    superFastBlur(plane_y, w, h, radius);
    superFastBlur(plane_u, w >> 1, h >> 1, radius);
    superFastBlur(plane_v, w >> 1, h >> 1, radius);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_asha_yuvblurlib_YuvBlur_nativeVersion(JNIEnv *env, jclass type) {
    return env->NewStringUTF("0.0.1");
}