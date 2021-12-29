#include "DWindowsVideoCapture.h"

#include <windows.h>
#include <dshow.h>
#include <qedit.h>
extern const CLSID CLSID_SampleGrabber;
extern const IID IID_ISampleGrabber;
extern const CLSID CLSID_NullRenderer;


struct CamContext
{
    ICaptureGraphBuilder2* pCaptureGraph;
    IGraphBuilder* pGraph;
    IMediaControl* pControl;
    IBaseFilter* pGrabberFilter;
    ISampleGrabber* pGrabber;
    IBaseFilter * pDestFilter;
    IMediaFilter *pMediaFilter;
    IAMStreamConfig *pStreamConf;
    AM_MEDIA_TYPE * pAmMediaType;
    PWCHAR wname;

    IBaseFilter* c_filter;

    GUID innerSubType;
    int w;
    int h;
//        MEDIASUBTYPE_RGB24;
    //    MEDIASUBTYPE_RGB32;
    //    MEDIASUBTYPE_RGB555;
    //    MEDIASUBTYPE_RGB565;
    //    MEDIASUBTYPE_YUY2;
    //    MEDIASUBTYPE_YVYU;
    //    MEDIASUBTYPE_YUYV;
    //    MEDIASUBTYPE_IYUV;
    //    MEDIASUBTYPE_UYVY;
    //    MEDIASUBTYPE_YV12;
    //    MEDIASUBTYPE_YVU9;
    //    MEDIASUBTYPE_Y411;
    //    MEDIASUBTYPE_Y41P;
    //    MEDIASUBTYPE_Y211;
    //    MEDIASUBTYPE_AYUV;


    CamContext();
};
CamContext::CamContext() :
    pCaptureGraph(NULL),
    pGraph(NULL),
    pControl(NULL),
    pGrabberFilter(NULL),
    pGrabber(NULL),
    pDestFilter(NULL),
    pMediaFilter(NULL),
    pStreamConf(NULL),
    pAmMediaType(NULL),
    innerSubType(MEDIASUBTYPE_RGB32)
{}

const GUID* getMediaTypeByName(const char* name)
{
    if(strcmp(name, "RGB24") == 0) return &MEDIASUBTYPE_RGB24;
    if(strcmp(name, "RGB32") == 0) return &MEDIASUBTYPE_RGB32;
    if(strcmp(name, "RGB555") == 0) return &MEDIASUBTYPE_RGB555;
    if(strcmp(name, "RGB565") == 0) return &MEDIASUBTYPE_RGB565;
    if(strcmp(name, "YUY2") == 0) return &MEDIASUBTYPE_YUY2;
    if(strcmp(name, "YVYU") == 0) return &MEDIASUBTYPE_YVYU;
    if(strcmp(name, "YUYV") == 0) return &MEDIASUBTYPE_YUYV;
    if(strcmp(name, "IYUV") == 0) return &MEDIASUBTYPE_IYUV;
    if(strcmp(name, "UYVY") == 0) return &MEDIASUBTYPE_UYVY;
    if(strcmp(name, "YV12") == 0) return &MEDIASUBTYPE_YV12;
    if(strcmp(name, "YVU9") == 0) return &MEDIASUBTYPE_YVU9;
    if(strcmp(name, "Y411") == 0) return &MEDIASUBTYPE_Y411;
    if(strcmp(name, "Y41P") == 0) return &MEDIASUBTYPE_Y41P;
    if(strcmp(name, "Y211") == 0) return &MEDIASUBTYPE_Y211;
    if(strcmp(name, "AYUV") == 0) return &MEDIASUBTYPE_AYUV;
    return NULL;
}
const char* getTypeNameByGUID(const GUID &g)
{
    if(g == MEDIASUBTYPE_RGB24) return "RGB24";
    if(g == MEDIASUBTYPE_RGB32) return "RGB32";
    if(g == MEDIASUBTYPE_RGB555) return "RGB555";
    if(g == MEDIASUBTYPE_RGB565) return "RGB565";
    if(g == MEDIASUBTYPE_YUY2) return "YUY2";
    if(g == MEDIASUBTYPE_YVYU) return "YVYU";
    if(g == MEDIASUBTYPE_YUYV) return "YUYV";
    if(g == MEDIASUBTYPE_IYUV) return "IYUV";
    if(g == MEDIASUBTYPE_UYVY) return "UYVY";
    if(g == MEDIASUBTYPE_YV12) return "YV12";
    if(g == MEDIASUBTYPE_YVU9) return "YVU9";
    if(g == MEDIASUBTYPE_Y411) return "Y411";
    if(g == MEDIASUBTYPE_Y41P) return "Y41P";
    if(g == MEDIASUBTYPE_Y211) return "Y211";
    if(g == MEDIASUBTYPE_AYUV) return "AYUV";
    return NULL;
}
DWVC_RAWTYPE getTypeByGUID(const GUID &g) {

    if(g == MEDIASUBTYPE_RGB24) return RAWTYPE_RGB24;
    if(g == MEDIASUBTYPE_RGB32) return RAWTYPE_RGB32;
    if(g == MEDIASUBTYPE_RGB555) return RAWTYPE_RGB555;
    if(g == MEDIASUBTYPE_RGB565) return RAWTYPE_RGB565;
    if(g == MEDIASUBTYPE_YUY2) return RAWTYPE_YUY2;
    if(g == MEDIASUBTYPE_YVYU) return RAWTYPE_YVYU;
    if(g == MEDIASUBTYPE_YUYV) return RAWTYPE_YUYV;
    if(g == MEDIASUBTYPE_IYUV) return RAWTYPE_IYUV;
    if(g == MEDIASUBTYPE_UYVY) return RAWTYPE_UYVY;
    if(g == MEDIASUBTYPE_YV12) return RAWTYPE_YV12;
    if(g == MEDIASUBTYPE_YVU9) return RAWTYPE_YVU9;
    if(g == MEDIASUBTYPE_Y411) return RAWTYPE_Y411;
    if(g == MEDIASUBTYPE_Y41P) return RAWTYPE_Y41P;
    if(g == MEDIASUBTYPE_Y211) return RAWTYPE_Y211;
    if(g == MEDIASUBTYPE_AYUV) return RAWTYPE_AYUV;
    return RAWTYPE_NOTYPE;
}

std::vector<CameraDescriptor*> cameras;
CameraDescriptor *_CURRENT_CAMERA = NULL;
int setCurrentCamera(CameraDescriptor *cd)
{
    if(cd) _CURRENT_CAMERA = cd;
    else return -1;
    return 0;
}
#define PROC_ERROR0(Error) \
    printf("CALL: %s: %s error\n", __func__, Error);
#define PROC_ERROR(RESULT, DESCRIPTOR_FOR_STOP, EPOINT) do{ \
    printf("CALL: %s - %s error: %d\n", __func__, EPOINT, RESULT); \
    if(DESCRIPTOR_FOR_STOP) stop_camera(DESCRIPTOR_FOR_STOP);\
    return RESULT;}while(0)

int read_all_cams()
{
    HRESULT hr;
    ICreateDevEnum* pSysDevEnum = NULL;
    IEnumMoniker* pEnumCat = NULL;
    IPropertyBag* pPropBag = NULL;
    IMoniker *pMoniker = NULL;
    ULONG cFetched = 0;

    /*
     * Call CoInitialize to initialize COM library in this thread in case when we use multithreading
     * Return value:
     * S_OK - The COM library was initialized successfully on this thread.
     * S_FALSE - The COM library is already initialized on this thread.
     * RPC_E_CHANGED_MODE - A previous call to CoInitializeEx specified
     * the concurrency model for this thread as multithread apartment (MTA).
     * This could also indicate that a change from neutral-threaded apartment
     * to single-threaded apartment has occurred.
     *
     * Single parameter is reserved and must be NULL.
     */
    hr =     CoInitialize(NULL);
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
    if(SUCCEEDED(hr))
    {
        hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
        if(SUCCEEDED(hr))
        {
            if(!pEnumCat) PROC_ERROR(-1, NULL, "No device");
            while( (hr = pEnumCat->Next(1, &pMoniker, &cFetched)) == S_OK )
            {
                hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
                if(SUCCEEDED(hr))
                {
                    VARIANT varName;
                    VariantInit(&varName);
                    hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                    if(SUCCEEDED(hr))
                    {
                        int count = 0;
                        CameraDescriptor* cd = new CameraDescriptor();
                        cd->context = new CamContext();
                        while( varName.bstrVal[count] != 0x00 ) ++count;
                        cd->context->wname = (WCHAR*)malloc(sizeof(WCHAR) * (count+1));
                        cd->name = (CHAR*)malloc(sizeof(CHAR) * (count+1));
                        count=0;
                        while( varName.bstrVal[count] != 0x00 )
                        {
                            cd->context->wname[count] = varName.bstrVal[count];
                            cd->name[count] = (char)varName.bstrVal[count];
                            ++count;
                        }
                        cd->context->wname[count] = 0x00;
                        cd->name[count] = '\0';

                        cd->context->c_filter = NULL;
                        hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&cd->context->c_filter);
                        if(SUCCEEDED(hr))
                        {
                            cameras.push_back(cd);
                        } else PROC_ERROR(hr, NULL, "BindToObject");

                        ICaptureGraphBuilder2* _pCaptureGraph = NULL;
                        IAMStreamConfig *_pStreamConf = NULL;
                        hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&_pCaptureGraph);
                        if(FAILED(hr)) PROC_ERROR(hr, NULL, "CoCreateInstance:IID_ICaptureGraphBuilder2");
                        hr = _pCaptureGraph->FindInterface(
                                    &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, cd->context->c_filter, IID_IAMStreamConfig, (void **)&_pStreamConf);
                        if(FAILED(hr)) PROC_ERROR(hr, NULL, "FindInterface:IID_IAMStreamConfig");
                        //-------------------------------------------------------------
                        int iCount, iSize;
                        hr = _pStreamConf->GetNumberOfCapabilities(&iCount, &iSize);
                        if(FAILED(hr)) PROC_ERROR(hr, NULL, "GetNumberOfCapabilities");
                        AM_MEDIA_TYPE *pmt = NULL;
                        for(int I=0;I!=iCount;++I)
                        {
                            VIDEO_STREAM_CONFIG_CAPS video_caps;
                            hr = _pStreamConf->GetStreamCaps(I, &pmt, (BYTE*)&video_caps);
                            if(SUCCEEDED(hr))
                            {
                                cd->fmts.insert(getTypeNameByGUID(pmt->subtype));

                                if (pmt->cbFormat != 0)
                                {
                                    CoTaskMemFree((PVOID)pmt->pbFormat);
                                    pmt->cbFormat = 0;
                                    pmt->pbFormat = NULL;
                                }
                                if (pmt->pUnk != NULL)
                                {
                                    pmt->pUnk->Release();
                                    pmt->pUnk = NULL;
                                }
                                CoTaskMemFree(pmt);
                                pmt = NULL;
                            }
                            else break;
                        }
                        _pStreamConf->Release();
                        _pStreamConf = NULL;
                        _pCaptureGraph->Release();
                        _pCaptureGraph = NULL;

                        VariantClear(&varName);
                        pPropBag->Release();
                        pPropBag = NULL;
                        pMoniker->Release();
                        pMoniker = NULL;

                        if(FAILED(hr)) break;
                    } else PROC_ERROR(hr, NULL, "Read");
                }
                else PROC_ERROR(hr, NULL, "BindToStorage");
            }
        } else PROC_ERROR(hr, NULL, "CreateClassEnumerator");
    } else PROC_ERROR(hr, NULL, "IID_ICreateDevEnum");

    pEnumCat->Release();
    pEnumCat = NULL;
    pSysDevEnum->Release();
    pSysDevEnum = NULL;

    if(FAILED(hr)) PROC_ERROR(hr, NULL, "GetStreamCaps");
    if(cameras.size())
        _CURRENT_CAMERA = cameras[0];
    else
        _CURRENT_CAMERA = NULL;
    return S_OK;
}
CameraDescriptor *get_camera(size_t index)
{
    if(index >= 0 && index < cameras.size()) return cameras[index];
    return NULL;
}
void NukeDownstream(IBaseFilter *pBF, IGraphBuilder* pGraph){
        IPin *pP, *pTo;
        ULONG u;
        IEnumPins *pins = NULL;
        PIN_INFO pininfo;
        HRESULT hr = pBF->EnumPins(&pins);
        pins->Reset();
        while (hr == NOERROR)
        {
                hr = pins->Next(1, &pP, &u);
                if (hr == S_OK && pP)
                {
                        pP->ConnectedTo(&pTo);
                        if (pTo)
                        {
                                hr = pTo->QueryPinInfo(&pininfo);
                                if (hr == NOERROR)
                                {
                                        if (pininfo.dir == PINDIR_INPUT)
                                        {
                                                NukeDownstream(pininfo.pFilter, pGraph);
                                                pGraph->Disconnect(pTo);
                                                pGraph->Disconnect(pP);
                                                pGraph->RemoveFilter(pininfo.pFilter);
                                        }
                                        pininfo.pFilter->Release();
                                        pininfo.pFilter = NULL;
                                }
                                pTo->Release();
                        }
                        pP->Release();
                }
        }
        if (pins) pins->Release();
}
void destroyGraph(IGraphBuilder* pGraph){
    HRESULT hr = NOERROR;

    while (hr == NOERROR)
    {
        IEnumFilters * pEnum = NULL;

        hr = pGraph->EnumFilters(&pEnum);
        if (FAILED(hr)) { printf("CALL: destroyGraph - EnumFilters error\n"); return; }

        IBaseFilter * pFilter = NULL;
        if (pEnum->Next(1, &pFilter, NULL) == S_OK)
        {
            FILTER_INFO FilterInfo={0};
            hr = pFilter->QueryFilterInfo(&FilterInfo);
            FilterInfo.pGraph->Release();
            hr = pGraph->RemoveFilter(pFilter);
            if (FAILED(hr)) { printf("CALL: destroyGraph - RemoveFilter error\n"); return; }

            pFilter->Release();
            pFilter = NULL;
        }
        else hr = 1;
        pEnum->Release();
        pEnum = NULL;
    }
}

int stop_camera(CameraDescriptor* cd)
{
    HRESULT hr;
    if(_CURRENT_CAMERA == cd) {
        _CURRENT_CAMERA = NULL;
    }
    if(cd)
    {
        if( cd->context && cd->context->pControl )
        {
            hr = cd->context->pControl->Pause();
            if (FAILED(hr)) PROC_ERROR(hr, NULL, "Pause");
            hr = cd->context->pControl->Stop();
            if (FAILED(hr)) PROC_ERROR(hr, NULL, "Stop");
        }
        if( cd->context->c_filter )NukeDownstream(cd->context->c_filter, cd->context->pGraph);
        if( cd->context->pDestFilter ){cd->context->pDestFilter->Release(); cd->context->pDestFilter = NULL;}

        if( cd->context->c_filter ){cd->context->c_filter->Release(); cd->context->c_filter = NULL;}
        if( cd->context->pGrabberFilter ){cd->context->pGrabberFilter->Release(); cd->context->pGrabberFilter = NULL;}
        if( cd->context->pGrabber ){cd->context->pGrabber->Release(); cd->context->pGrabber = NULL;}
        if( cd->context->pControl ){cd->context->pControl->Release(); cd->context->pControl = NULL;}
        if( cd->context->pStreamConf ){cd->context->pStreamConf->Release(); cd->context->pStreamConf = NULL;}

        if(cd->context->pAmMediaType)
        {
            if (cd->context->pAmMediaType->cbFormat != 0)
            {
                CoTaskMemFree((PVOID)cd->context->pAmMediaType->pbFormat);
                cd->context->pAmMediaType->cbFormat = 0;
                cd->context->pAmMediaType->pbFormat = NULL;
            }
            if (cd->context->pAmMediaType->pUnk != NULL)
            {
                cd->context->pAmMediaType->pUnk->Release();
                cd->context->pAmMediaType->pUnk = NULL;
            }
            CoTaskMemFree(cd->context->pAmMediaType);
        }
        if(cd->context->pGraph) destroyGraph(cd->context->pGraph);

        if( cd->context->pCaptureGraph ){cd->context->pCaptureGraph->Release();cd->context->pCaptureGraph = NULL;}
        if( cd->context->pGraph ){cd->context->pGraph->Release();cd->context->pGraph = NULL;}
    }
    return 0;
}
int init_camera(CameraDescriptor* cd, const char *raw_fmt)
{
    IAMStreamConfig *streamConfTest = NULL;
    GUID CAPTURE_MODE = PIN_CATEGORY_CAPTURE;


    HRESULT hr;
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&cd->context->pCaptureGraph);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "CLSID_CaptureGraphBuilder2");
    hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,IID_IGraphBuilder, (void**)&cd->context->pGraph);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "CLSID_FilterGraph");
    hr = cd->context->pCaptureGraph->SetFiltergraph(cd->context->pGraph);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "SetFiltergraph");
    hr = cd->context->pGraph->QueryInterface(IID_IMediaControl, (void**)&cd->context->pControl);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "IID_IMediaControl");
    hr = cd->context->pGraph->AddFilter(cd->context->c_filter, cd->context->wname);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "AddFilter 1");


    hr = cd->context->pCaptureGraph->FindInterface(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, cd->context->c_filter, IID_IAMStreamConfig, (void **)&streamConfTest);
//    if(FAILED(hr)) INIT_FAILED(hr, cd, "IID_IAMStreamConfig");
    hr = cd->context->pCaptureGraph->FindInterface(&CAPTURE_MODE, &MEDIATYPE_Video, cd->context->c_filter, IID_IAMStreamConfig, (void **)&cd->context->pStreamConf);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "IID_IAMStreamConfig2");
    hr = cd->context->pStreamConf->GetFormat(&cd->context->pAmMediaType);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "GetFormat");


    VIDEOINFOHEADER *pVih =  reinterpret_cast<VIDEOINFOHEADER*>(cd->context->pAmMediaType->pbFormat);
    cd->context->w	= pVih->bmiHeader.biWidth;
    cd->context->h 	= pVih->bmiHeader.biHeight;


    cd->width = cd->context->w;
    cd->height = cd->context->h;


    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER ,IID_IBaseFilter, (void**)&cd->context->pGrabberFilter);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "CoCreateInstance:CLSID_SampleGrabber");
    hr = cd->context->pGraph->AddFilter(cd->context->pGrabberFilter, L"Sample Grabber");
    if(FAILED(hr)) PROC_ERROR(hr, cd, "AddFilter 2");
    hr = cd->context->pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&cd->context->pGrabber);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "QueryInterface:IID_ISampleGrabber");
    hr = cd->context->pGrabber->SetOneShot(FALSE);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "SetOneShot");
    hr = cd->context->pGrabber->SetBufferSamples(TRUE);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "SetBufferSamples");

    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype 	= MEDIATYPE_Video;
    mt.subtype 		= cd->context->pAmMediaType->subtype;
    mt.formattype 	= FORMAT_VideoInfo;

    if(raw_fmt)
    {
        const GUID *pSubtype = getMediaTypeByName(raw_fmt);
        if(pSubtype) mt.subtype 		= *pSubtype;
    }
    cd->format = getTypeNameByGUID(mt.subtype);


    hr = cd->context->pGrabber->SetMediaType(&mt);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "SetMediaType");
    hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&cd->context->pDestFilter);
    if(FAILED(hr)) PROC_ERROR(hr, cd, "CoCreateInstance:CLSID_NullRenderer");
    hr = cd->context->pGraph->AddFilter(cd->context->pDestFilter, L"NullRenderer");
    if(FAILED(hr)) PROC_ERROR(hr, cd, "AddFilter 3");

    return S_OK;
}
int run_camera(CameraDescriptor *cd)
{
    HRESULT hr;
    hr = cd->context->pCaptureGraph->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, cd->context->c_filter,
                                                  cd->context->pGrabberFilter, cd->context->pDestFilter);

    if(FAILED(hr))
    {
        printf("CALL: run_camera() - RenderStream error: %d\n", hr);
        return hr;
    }
    IMediaFilter *pMediaFilter = NULL;
    hr = cd->context->pGraph->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
    if(FAILED(hr))
    {
        printf("CALL: run_camera() - QueryInterface:IID_IMediaFilter error: %d\n", hr);
        return hr;
    }
    hr = cd->context->pControl->Run();
    if(FAILED(hr))
    {
        printf("CALL: run_camera() - Run error: %d\n", hr);
        return hr;
    }

    return 0;
}
int capture_camera(CameraDescriptor *cd, uint8_t *dest, long *size)
{
    HRESULT hr = cd->context->pGrabber->GetCurrentBuffer(size, (long*)dest);
    if(FAILED(hr))
    {
        while(hr != S_OK)
        {
            hr = cd->context->pGrabber->GetCurrentBuffer(size, (long*)dest);
            Sleep(10);
        }
    }
    return hr;
}
int capture_camera(uint8_t *dst, long *size)
{
//    printf("CALL CAPTURE_CAMERA dst: %p size: %p curr cam: %s\n", dst, size, _CURRENT_CAMERA->name);
    if(_CURRENT_CAMERA) return capture_camera(_CURRENT_CAMERA, dst, size);
    else PROC_ERROR(-1, NULL, "No current capture device");
}



int startCamera(int index){
    int r = 0;

    //------------------------------------------------------
    r = read_all_cams();
    if(r < 0) PROC_ERROR(r, NULL, "read_all_cams");
    if(cameras.empty()) PROC_ERROR(-1, NULL, "No cameras");
    //------------------------------------------------------
    CameraDescriptor *d = get_camera(index);
    if(d == NULL) PROC_ERROR(r, d, "get_camera");
    //------------------------------------------------------
    r = init_camera(d);
    if(r < 0) PROC_ERROR(r, d, "init_camera");
    //------------------------------------------------------
    r = run_camera(d);
    if(r < 0) PROC_ERROR(r, d, "run_camera");
    //------------------------------------------------------
    r = setCurrentCamera(d);
    if(r < 0) PROC_ERROR(r, d, "setCurrentCamera");

    return r;
}
uint8_t *getCameraBuffer(int width, int height, long *bufferSize, CameraDescriptor *d){
    uint8_t *buffer = nullptr;
    int r = 0;
    long bs = 0;
    if(width <= 0 || height <= 0) {
        PROC_ERROR0("Wrong input args (width or height)");
        goto fail;
    }
    d = d ? d :_CURRENT_CAMERA;
    if(d == NULL) {
        PROC_ERROR0("Bad pointer to CameraDescriptor");
        goto fail;
    }
    bs = width * height * 3;
    if( (buffer = (uint8_t*)malloc(bs)) == NULL ) {
        PROC_ERROR0("Can't alloc mem for buffer");
        goto fail;
    }
    memset(buffer, 0, bs);
    if( (r = capture_camera(buffer, &bs)) < 0) {
        PROC_ERROR0("First capturing fail");
        goto fail;
    }

    if(bufferSize)
        *bufferSize = bs;
    return buffer;
fail:
    if(buffer)
        free(buffer);
    if(bufferSize) *bufferSize = 0;
    return NULL;
}
#undef PROC_ERROR

CameraDescriptor *getCurrentCamera()
{
    return _CURRENT_CAMERA;
}
const char *getCameraRawTypeName(CameraDescriptor *d) {
    d = d ? d : _CURRENT_CAMERA;
    if(d == NULL) return NULL;
    return getTypeNameByGUID(d->context->pAmMediaType->subtype);
}
DWVC_RAWTYPE getCameraRawType(CameraDescriptor *d) {
    d = d ? d : _CURRENT_CAMERA;
    if(d == NULL) return RAWTYPE_NOTYPE;
    return getTypeByGUID(d->context->pAmMediaType->subtype);
}
