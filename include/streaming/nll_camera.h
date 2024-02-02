/*

NavFPS movements :
* forward, move along camera Z axis
* strafe, move along camera X axis
* up, move along camera Y axis
* rotate yaw, rotate around camera Y/up axis
* rotation pitch, rotate around camera X axis
* drag to look, the 'picked' point on image stays beneath the mouse during the drag

TODO(yan) gérer une notion de zoom pour les vues panoramiques

NOTE(nico) references :

* https://github.com/nlguillemot/arcball_camera
* https://github.com/nlguillemot/flythrough_camera
* https://github.com/nlguillemot/glFast/blob/master/examples/01%20Instancing%20and%20MRT/qfpc.h ('quatFirstPersonCamera' by Constantine Tarasenkov aka @relativetoyou)

* http://donw.io/post/frustum-point-extraction/

* https://blog.umbra3d.com/blog/desiging-interactions-for-augmented-reality-part-2

*/

#include <math.h>

#define NLL_CAM_WS_RIGHT_AXIS 0
#define NLL_CAM_WS_FRONT_AXIS 1
#define NLL_CAM_WS_UP_AXIS 2
#define NLL_CAM_WS_UP_VEC3 {0.f, 0.f, 1.f}
#define NLL_CAM_WS_RIGHT_HANDED 1

typedef struct nllCamera {

    float x[3], y[3], z[3];
    float pos[3];
    float znear;
    // TODO(nico) should aspect ratio be contained here or only in used code ?
}
nllCamera;

typedef struct nllCameraNavFPS_settings {

    float maxSpeed;
    float maxRotationSpeed;
    float minPitchAngleRadians, maxPitchAngleRadians;
}
nllCameraNavFPS_settings;

typedef struct nllCameraNavFPS_state {

    nllCamera camera;           // TODO(nico) should allow user to have its own camera position precision (not 'float')
    float moveSpeeds[3];
    float rotationSpeed[2];     // yaw, pitch
    
    float mouseDragBasePos[2];
    nllCamera mouseDragBaseCam;
    int mousePressed;
}
nllCameraNavFPS_state;

typedef struct nllCameraNavFPS_input {

    float moveFactors[3];
    float rotationFactors[2];
    float mousePos[2];
    int mousePressed;
}
nllCameraNavFPS_input;

//----------------------------------------------------------------------------

nllCameraNavFPS_state nllCameraNavFPS_apply(
    nllCameraNavFPS_settings const *nSettings, 
    nllCameraNavFPS_input const *nInput, 
    nllCameraNavFPS_state const *nState, 
    float dt
);

//============================================================================
//============================================================================

#include "nll_math.h"                   // TODO(nico) single header lib ?

void nllCamera__rayFromCamera(nllCamera const *cam, float ndc_x, float ndc_y, nllVec3 *out_startPos, nllVec3 *out_dir);         // FIXME(nico) in the public API ?

#define NLL_CAM_ASSERT(x) assert(x)
#define NLL_CAM_PI_OVER_2 (3.1415926535f/2.f)

typedef struct nllCameraNavFPS__CosSin {

    float cos, sin;
}
nllCameraNavFPS__CosSin;

nllCameraNavFPS__CosSin nllCameraNavFPS__cosSinZero() {

    nllCameraNavFPS__CosSin result = {1, 0};
    return result;    
}

nllCameraNavFPS__CosSin nllCameraNavFPS__cosSinAdd(const nllCameraNavFPS__CosSin *a, const nllCameraNavFPS__CosSin *b) {
        
    nllCameraNavFPS__CosSin result;
    // cos(a+b) = cos(a)*cos(b) - sin(a)*sin(b)
    // sin(a+b) = sin(a)*cos(b) + cos(a)*sin(b)
    result.cos = a->cos * b->cos - a->sin * b->sin;
    result.sin = a->sin * b->cos + a->cos * b->sin;
    return result;
}

nllCameraNavFPS__CosSin nllCameraNavFPS__cosSinSub(const nllCameraNavFPS__CosSin *a, const nllCameraNavFPS__CosSin *b) {
    
    nllCameraNavFPS__CosSin result;
    // cos(a-b) = cos(a)*cos(b) + sin(a)*sin(b)
    // sin(a-b) = sin(a)*cos(b) - cos(a)*sin(b)
    result.cos = a->cos * b->cos + a->sin * b->sin;
    result.sin = a->sin * b->cos - a->cos * b->sin;
    return result;
}
    
typedef struct nllCameraNavFPS__vec3AsYawPitch {

    nllCameraNavFPS__CosSin yaw, pitch;
    float lookYawLength, lookPitchLength;
}
nllCameraNavFPS__vec3AsYawPitch;

nllCameraNavFPS__vec3AsYawPitch nllCameraNavFPS__yawPitchFromLookVec3(const nllVec3 look) {
    
    nllCameraNavFPS__vec3AsYawPitch result;

    result.lookYawLength = sqrtf( look[NLL_CAM_WS_RIGHT_AXIS] * look[NLL_CAM_WS_RIGHT_AXIS] + look[NLL_CAM_WS_FRONT_AXIS] * look[NLL_CAM_WS_FRONT_AXIS] );
    result.lookPitchLength = sqrtf( look[NLL_CAM_WS_UP_AXIS] * look[NLL_CAM_WS_UP_AXIS] + result.lookYawLength * result.lookYawLength );

    result.yaw.cos = look[NLL_CAM_WS_RIGHT_AXIS] / result.lookYawLength;
    result.yaw.sin = look[NLL_CAM_WS_FRONT_AXIS] / result.lookYawLength;
    result.pitch.cos = result.lookYawLength / result.lookPitchLength;
    result.pitch.sin = look[NLL_CAM_WS_UP_AXIS] / result.lookPitchLength;
    
    return result;
}

nllCameraNavFPS_state nllCameraNavFPS_apply(nllCameraNavFPS_settings const *nSettings, nllCameraNavFPS_input const *nInput, nllCameraNavFPS_state const *nState, float dt) {

    nllCameraNavFPS_state result = *nState;

    NLL_CAM_ASSERT( nSettings->minPitchAngleRadians <= nSettings->maxPitchAngleRadians );
    NLL_CAM_ASSERT( nSettings->minPitchAngleRadians > -NLL_CAM_PI_OVER_2 );
    NLL_CAM_ASSERT( nSettings->maxPitchAngleRadians <  NLL_CAM_PI_OVER_2 );
    
    // handle mouse drag to look

    nllCameraNavFPS__CosSin deltaYaw = nllCameraNavFPS__cosSinZero(), deltaPitch = nllCameraNavFPS__cosSinZero();
    nllVec3 lookDir;
    nllVec3Assign(lookDir, result.camera.z);

    if (nInput->mousePressed != nState->mousePressed) {

        if (nInput->mousePressed) {

            memcpy(result.mouseDragBasePos, nInput->mousePos, sizeof(nInput->mousePos));
            result.mouseDragBaseCam = nState->camera;
        }
    }
    else {

        if (nInput->mousePressed) {

            nllVec3 unusedPos;
            nllVec3 dragBaseDir, curDir;
            nllCamera__rayFromCamera(&result.mouseDragBaseCam, nState->mouseDragBasePos[0], nState->mouseDragBasePos[1], &unusedPos, &dragBaseDir);
            nllCamera__rayFromCamera(&result.mouseDragBaseCam, nInput->mousePos[0], nInput->mousePos[1], &unusedPos, &curDir);

            nllCameraNavFPS__vec3AsYawPitch base = nllCameraNavFPS__yawPitchFromLookVec3(dragBaseDir);
            nllCameraNavFPS__vec3AsYawPitch cur = nllCameraNavFPS__yawPitchFromLookVec3(curDir);
            
            deltaYaw = nllCameraNavFPS__cosSinSub(&cur.yaw, &base.yaw);
            deltaPitch = nllCameraNavFPS__cosSinSub(&cur.pitch, &base.pitch);

            nllVec3Assign(lookDir, result.mouseDragBaseCam.z);
        }
    }
    result.mousePressed = nInput->mousePressed;
        
    // delta values

    float const dLength = dt * nSettings->maxSpeed;
    float const rArc = dt * nSettings->maxRotationSpeed;
    result.moveSpeeds[0] = 0.5f * (result.moveSpeeds[0] + dLength * nInput->moveFactors[0]);
    result.moveSpeeds[1] = 0.5f * (result.moveSpeeds[1] + dLength * nInput->moveFactors[1]);
    result.moveSpeeds[2] = 0.5f * (result.moveSpeeds[2] + dLength * nInput->moveFactors[2]);
    result.rotationSpeed[0] = 0.5f * (result.rotationSpeed[0] + rArc * nInput->rotationFactors[0]);
    result.rotationSpeed[1] = 0.5f * (result.rotationSpeed[1] + rArc * nInput->rotationFactors[1]);

    // update yaw/pitch

    nllCameraNavFPS__vec3AsYawPitch yawPitch = nllCameraNavFPS__yawPitchFromLookVec3(lookDir);

    nllCameraNavFPS__CosSin const yawRot = { cosf(result.rotationSpeed[0]), sinf(result.rotationSpeed[0]) };
    nllCameraNavFPS__CosSin const pitchRot = { cosf(result.rotationSpeed[1]), sinf(result.rotationSpeed[1]) };
    
    deltaYaw = nllCameraNavFPS__cosSinSub(&yawRot, &deltaYaw);
    deltaPitch = nllCameraNavFPS__cosSinSub(&pitchRot, &deltaPitch);
    
    yawPitch.yaw = nllCameraNavFPS__cosSinAdd(&yawPitch.yaw, &deltaYaw);
    yawPitch.pitch = nllCameraNavFPS__cosSinAdd(&yawPitch.pitch, &deltaPitch);
    
    // clamp pitch to limits // FIXME(nico) bug when dragging mouse below/above PI/2 angle

    const float minPitchSin = sinf(nSettings->minPitchAngleRadians);
    const float maxPitchSin = sinf(nSettings->maxPitchAngleRadians);
    if (minPitchSin > yawPitch.pitch.sin || 0 > yawPitch.pitch.cos) {

        yawPitch.pitch.sin = minPitchSin;
        yawPitch.pitch.cos = cosf(nSettings->minPitchAngleRadians);
    }
    if (maxPitchSin < yawPitch.pitch.sin || 0 > yawPitch.pitch.cos) {

        yawPitch.pitch.sin = maxPitchSin;
        yawPitch.pitch.cos = cosf(nSettings->maxPitchAngleRadians);
    }
    
    // build a new orthonormal basis from yaw/pitch

    result.camera.z[NLL_CAM_WS_UP_AXIS] = yawPitch.pitch.sin * yawPitch.lookPitchLength;
    yawPitch.lookYawLength = yawPitch.pitch.cos * yawPitch.lookPitchLength;
    result.camera.z[NLL_CAM_WS_RIGHT_AXIS] = yawPitch.yaw.cos * yawPitch.lookYawLength;
    result.camera.z[NLL_CAM_WS_FRONT_AXIS] = yawPitch.yaw.sin * yawPitch.lookYawLength;
    
    float const world_up[] = NLL_CAM_WS_UP_VEC3;
#if NLL_CAM_WS_RIGHT_HANDED
	nllVec3Cross(result.camera.x, result.camera.z, world_up);
	nllVec3Normalize(result.camera.x);
	nllVec3Cross(result.camera.y, result.camera.x, result.camera.z);
#else
    nllVec3Cross(result.camera.x, world_up, result.camera.z);
    nllVec3Normalize(result.camera.x);
    nllVec3Cross(result.camera.y, result.camera.z, result.camera.x);
#endif

    // new camera position

    nllVec3 dx, dy, dz, dPos;
    
    nllVec3Mulf(dx, result.camera.x, result.moveSpeeds[0]);
    nllVec3Mulf(dy, result.camera.y, result.moveSpeeds[1]);
    nllVec3Mulf(dz, result.camera.z, result.moveSpeeds[2]);
    
    nllVec3Assign(dPos, dx);
    nllVec3Add(dPos, dPos, dy);
    nllVec3Add(dPos, dPos, dz);
    nllVec3Add(result.camera.pos, result.camera.pos, dPos);         // TODO(nico) camera position can't be in float[3] (not enough precision)
    
    return result;
}

/*
NOTE, `out_dir` is not normalized
*/
void nllCamera__rayFromCamera(nllCamera const *cam, float ndc_x, float ndc_y, nllVec3 *out_startPos, nllVec3 *out_dir) {

    nllVec3 x, y, z, pos, dir;

    nllVec3Mulf(x, cam->x, ndc_x * cam->znear);
    nllVec3Mulf(y, cam->y, ndc_y);
    nllVec3Mulf(z, cam->z, cam->znear);
    nllVec3Add(dir, x, y);
    nllVec3Add(dir, dir, z);

    nllVec3Add(pos, cam->pos, dir);

    nllVec3Assign(*out_startPos, pos);
    nllVec3Assign(*out_dir, dir);
}
