#ifndef DWINDOWSVIDEOCAPTURE_H
#define DWINDOWSVIDEOCAPTURE_H
#include <vector>
#include <set>
#include <stdint.h>
struct CamContext;
struct CameraDescriptor
{
    char *name;
    CamContext *context;
    std::set<const char*> fmts;

    const char *format;
    int width;
    int height;
};
enum DWVC_RAWTYPE {

    RAWTYPE_NOTYPE = -1,
    RAWTYPE_RGB24,
    RAWTYPE_RGB32,
    RAWTYPE_RGB555,
    RAWTYPE_RGB565,
    RAWTYPE_YUY2,
    RAWTYPE_YVYU,
    RAWTYPE_YUYV,
    RAWTYPE_IYUV,
    RAWTYPE_UYVY,
    RAWTYPE_YV12,
    RAWTYPE_YVU9,
    RAWTYPE_Y411,
    RAWTYPE_Y41P,
    RAWTYPE_Y211,
    RAWTYPE_AYUV
};


extern std::vector<CameraDescriptor*> cameras;
extern CameraDescriptor *_CURRENT_CAMERA;
int read_all_cams();
CameraDescriptor* get_camera(size_t index);

CameraDescriptor* getCurrentCamera();
const char* getCameraRawTypeName(CameraDescriptor *d = NULL);
DWVC_RAWTYPE getCameraRawType(CameraDescriptor *d = NULL);
int stop_camera(CameraDescriptor* cd);
int init_camera(CameraDescriptor* cd, const char* raw_fmt = nullptr);
int run_camera(CameraDescriptor* cd);
int capture_camera(CameraDescriptor* cd, uint8_t *dst, long *size);
int capture_camera(uint8_t *dst, long *size); //by global current CameraDescriptor
int setCurrentCamera(CameraDescriptor *cd);

int startCamera(int index = 0);
uint8_t* getCameraBuffer(int width, int height, long *bufferSize = NULL, CameraDescriptor *d = NULL);
//libs: win32: LIBS += -lddraw -ldxguid -lole32 -loleaut32  -lstrmiids -luuid
//original: https://github.com/ofTheo/videoInput
#endif // DWINDOWSVIDEOCAPTURE_H
