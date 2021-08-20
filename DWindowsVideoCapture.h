#ifndef DWINDOWSVIDEOCAPTURE_H
#define DWINDOWSVIDEOCAPTURE_H
#include <vector>
#include <set>
#include <stdint.h>
struct CamContext;
struct CameraDescriptor
{
    char*  name;
    CamContext* context;
    std::set<const char*> fmts;
};
extern std::vector<CameraDescriptor*> cameras;
extern CameraDescriptor *_CURRENT_CAMERA;
int read_all_cams();
CameraDescriptor* get_camera(size_t index);
int stop_camera(CameraDescriptor* cd);
int init_camera(CameraDescriptor* cd, const char* raw_fmt = nullptr);
int run_camera(CameraDescriptor* cd);
int capture_camera(CameraDescriptor* cd, uint8_t *dst, long *size);
int capture_camera(uint8_t *dst, long *size); //by global current CameraDescriptor
int setCurrentCamera(CameraDescriptor *cd);
//libs: win32: LIBS += -lddraw -ldxguid -lole32 -loleaut32  -lstrmiids -luuid
//original: https://github.com/ofTheo/videoInput
#endif // DWINDOWSVIDEOCAPTURE_H
