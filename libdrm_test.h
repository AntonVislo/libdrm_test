#define CRTC_ID_DP 68
#define CONN_ID_DP 205
#define CRTC_ID_HDMI 112
#define CONN_ID_HDMI 207
#define MAX_CONN 10
#define MAX_STRING_LEN 100
#define MAX_OPT 10
#define MAX_CRTC_BUFS 10
#define MAX_CONNECTORS 20

struct bufferObject{
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t handle;
    uint32_t size;
    uint8_t *vaddr;
    uint32_t fb_id;
};

typedef struct connectorObject{
    uint32_t conn_id;
    uint32_t encod_id;
    uint32_t crtc_id;
    const char *name;
    struct bufferObject buf;
    drmModeModeInfo *crtc_mode;
}connectorObject;

typedef struct crtcObj{
    uint32_t crtcId;
    drmModeModeInfo mode;
    uint32_t cntBufs;
    uint32_t fbId;
    struct bufferObject bufs[MAX_CRTC_BUFS];
}crtcObj;

typedef struct connObj{
    uint32_t connId;
    char *name;
    int countModes;
	drmModeModeInfoPtr modes;
    crtcObj crtc;
    uint32_t status;
}connObj;

typedef struct drmObj{
    char *drmFile;
    uint32_t cntConn;
    connObj connectors[MAX_CONNECTORS];
    uint32_t connectedIndex[MAX_CONNECTORS];
    uint32_t countConnected;
}drmObj;
int enum_drm(void);
int scanDrm(drmObj *drm);