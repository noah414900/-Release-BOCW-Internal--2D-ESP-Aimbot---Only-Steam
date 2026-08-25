#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <intrin.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include "offsets.h"
#include "decryption.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// ─────────────────────────────────────────────────────────────
// GLOBALS
// ─────────────────────────────────────────────────────────────
volatile bool g_Running = true;
HMODULE       g_Module = nullptr;
uintptr_t     g_Base = 0;

// ─────────────────────────────────────────────────────────────
// MATH
// ─────────────────────────────────────────────────────────────
struct Vec2 { float x, y; };
struct Vec3 {
    float x, y, z;
    float dot(const Vec3& o)   const { return x * o.x + y * o.y + z * o.z; }
    float length()             const { return sqrtf(x * x + y * y + z * z); }
    Vec3  operator-(const Vec3& o) const { return { x - o.x,y - o.y,z - o.z }; }
    Vec3  operator+(const Vec3& o) const { return { x + o.x,y + o.y,z + o.z }; }
    Vec3  norm() const {
        float l = length();
        return l > 0.0001f ? Vec3{ x / l,y / l,z / l } : Vec3{ 0,0,0 };
    }
};
struct Vec2Ang { float pitch, yaw; };

// ─────────────────────────────────────────────────────────────
// SAFE READ
// ─────────────────────────────────────────────────────────────
template<typename T>
static T SR(uintptr_t a) {
    T r{}; if (!a) return r;
    __try { r = *(volatile T*)a; }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    return r;
}
static bool CanRead(uintptr_t a, size_t n = 4) {
    if (!a) return false;
    MEMORY_BASIC_INFORMATION m{};
    if (!VirtualQuery((LPCVOID)a, &m, sizeof(m))) return false;
    return m.State == MEM_COMMIT
        && !(m.Protect & PAGE_NOACCESS)
        && !(m.Protect & PAGE_GUARD)
        && (uintptr_t)m.BaseAddress + m.RegionSize >= a + n;
}

// ─────────────────────────────────────────────────────────────
// SETTINGS
// ─────────────────────────────────────────────────────────────
struct Settings {
    bool  espEnabled = true;
    bool  showBoxes = true;
    bool  showHealth = true;
    bool  showNames = true;
    bool  showDistance = true;
    bool  showSnaplines = false;
    int   aimbotBodyPart = 0;
    bool  showStance = true;
    bool  showCrosshair = true;
    bool  teamCheck = true;
    int enemyTeam = 0;
    float maxDistance = 1000.f;
    bool  aimbotEnabled = false;
    float aimbotFOV = 10.f;
    float aimbotSmooth = 5.f;
    int   aimbotKey = VK_RBUTTON;
    bool  menuOpen = false;

    // ── Triggerbot ──
    bool  triggerbotEnabled = false;
    float triggerbotFOV = 2.0f;
    float triggerbotSnap = 0.5f;
    int   triggerbotKey = VK_XBUTTON2;
} g_Set;

static char g_ConfigPath[MAX_PATH] = {};

static void BuildConfigPath() {
    if (g_ConfigPath[0]) return;
    GetModuleFileNameA(g_Module, g_ConfigPath, MAX_PATH);
    char* last = strrchr(g_ConfigPath, '\\');
    if (last) *(last + 1) = '\0';
    else g_ConfigPath[0] = '\0';
    strcat_s(g_ConfigPath, "bocw_config.ini");
}

static void SaveConfig() {
    BuildConfigPath();
    FILE* f = fopen(g_ConfigPath, "w");
    if (!f) return;
    fprintf(f, "[ESP]\n");
    fprintf(f, "espEnabled=%d\n", g_Set.espEnabled);
    fprintf(f, "showBoxes=%d\n", g_Set.showBoxes);
    fprintf(f, "showHealth=%d\n", g_Set.showHealth);
    fprintf(f, "showNames=%d\n", g_Set.showNames);
    fprintf(f, "showDistance=%d\n", g_Set.showDistance);
    fprintf(f, "showSnaplines=%d\n", g_Set.showSnaplines);
    fprintf(f, "showCrosshair=%d\n", g_Set.showCrosshair);
    fprintf(f, "showStance=%d\n", g_Set.showStance);
    fprintf(f, "teamCheck=%d\n", g_Set.teamCheck);
    fprintf(f, "enemyTeam=%d\n", g_Set.enemyTeam);
    fprintf(f, "maxDistance=%.1f\n", g_Set.maxDistance);
    fprintf(f, "\n[Aimbot]\n");
    fprintf(f, "aimbotEnabled=%d\n", g_Set.aimbotEnabled);
    fprintf(f, "aimbotFOV=%.2f\n", g_Set.aimbotFOV);
    fprintf(f, "aimbotSmooth=%.2f\n", g_Set.aimbotSmooth);
    fprintf(f, "aimbotBodyPart=%d\n", g_Set.aimbotBodyPart);
    fprintf(f, "aimbotKey=%d\n", g_Set.aimbotKey);
    fprintf(f, "\n[Triggerbot]\n");
    fprintf(f, "triggerbotEnabled=%d\n", g_Set.triggerbotEnabled);
    fprintf(f, "triggerbotFOV=%.2f\n", g_Set.triggerbotFOV);
    fprintf(f, "triggerbotSnap=%.2f\n", g_Set.triggerbotSnap);
    fprintf(f, "triggerbotKey=%d\n", g_Set.triggerbotKey);
    fclose(f);
}

static int ReadInt(const char* buf, const char* key, int def) {
    const char* p = strstr(buf, key);
    if (!p) return def;
    p += strlen(key);
    if (*p != '=') return def;
    return atoi(p + 1);
}

static float ReadFloat(const char* buf, const char* key, float def) {
    const char* p = strstr(buf, key);
    if (!p) return def;
    p += strlen(key);
    if (*p != '=') return def;
    return (float)atof(p + 1);
}

static void LoadConfig() {
    BuildConfigPath();
    FILE* f = fopen(g_ConfigPath, "r");
    if (!f) return;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz <= 0 || sz > 8192) { fclose(f); return; }
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(sz + 1);
    if (!buf) { fclose(f); return; }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);

    g_Set.espEnabled = ReadInt(buf, "espEnabled", 1) != 0;
    g_Set.showBoxes = ReadInt(buf, "showBoxes", 1) != 0;
    g_Set.showHealth = ReadInt(buf, "showHealth", 1) != 0;
    g_Set.showNames = ReadInt(buf, "showNames", 1) != 0;
    g_Set.showDistance = ReadInt(buf, "showDistance", 1) != 0;
    g_Set.showSnaplines = ReadInt(buf, "showSnaplines", 0) != 0;
    g_Set.showCrosshair = ReadInt(buf, "showCrosshair", 1) != 0;
    g_Set.showStance = ReadInt(buf, "showStance", 1) != 0;
    g_Set.teamCheck = ReadInt(buf, "teamCheck", 1) != 0;
    g_Set.enemyTeam = ReadInt(buf, "enemyTeam", 0);
    g_Set.maxDistance = ReadFloat(buf, "maxDistance", 1000.f);
    g_Set.aimbotEnabled = ReadInt(buf, "aimbotEnabled", 0) != 0;
    g_Set.aimbotFOV = ReadFloat(buf, "aimbotFOV", 10.f);
    g_Set.aimbotSmooth = ReadFloat(buf, "aimbotSmooth", 5.f);
    g_Set.aimbotBodyPart = ReadInt(buf, "aimbotBodyPart", 0);
    g_Set.aimbotKey = ReadInt(buf, "aimbotKey", VK_RBUTTON);

    g_Set.triggerbotEnabled = ReadInt(buf, "triggerbotEnabled", 0) != 0;
    g_Set.triggerbotFOV = ReadFloat(buf, "triggerbotFOV", 2.0f);
    g_Set.triggerbotSnap = ReadFloat(buf, "triggerbotSnap", 0.5f);
    g_Set.triggerbotKey = ReadInt(buf, "triggerbotKey", VK_XBUTTON2);

    // clamp everything
    if (g_Set.maxDistance < 10.f)   g_Set.maxDistance = 10.f;
    if (g_Set.maxDistance > 1000.f) g_Set.maxDistance = 1000.f;
    if (g_Set.aimbotFOV < 1.f)    g_Set.aimbotFOV = 1.f;
    if (g_Set.aimbotFOV > 45.f)   g_Set.aimbotFOV = 45.f;
    if (g_Set.aimbotSmooth < 1.f)    g_Set.aimbotSmooth = 1.f;
    if (g_Set.aimbotSmooth > 20.f)   g_Set.aimbotSmooth = 20.f;
    if (g_Set.aimbotBodyPart < 0 || g_Set.aimbotBodyPart > 3)
        g_Set.aimbotBodyPart = 0;
    if (g_Set.enemyTeam < 0 || g_Set.enemyTeam > 2)
        g_Set.enemyTeam = 0;
    if (g_Set.triggerbotFOV < 0.5f) g_Set.triggerbotFOV = 0.5f;
    if (g_Set.triggerbotFOV > 10.f) g_Set.triggerbotFOV = 10.f;
    if (g_Set.triggerbotSnap < 0.f)  g_Set.triggerbotSnap = 0.f;
    if (g_Set.triggerbotSnap > 1.f)  g_Set.triggerbotSnap = 1.f;

    free(buf);
}


// ─────────────────────────────────────────────────────────────
// CAMERA + ENTITY STATE
// ─────────────────────────────────────────────────────────────
struct CamState {
    Vec3  origin, fwd, right, up;
    float tanHX, tanHY;
    bool  valid;
};
static CamState  g_Cam = {};
static uintptr_t g_LCPtr = 0;
static int       g_LocalTeam = -1;
static int       SCREEN_W = 1920;
static int       SCREEN_H = 1080;

struct EntData {
    Vec3    feet, head, aimPos;
    Vec2    sFeet, sHead;
    float   dist;
    int     health, slot, team;
    char    name[36];
    bool    onScreen;
    uint8_t stanceFlags;
    float   headHeight;
};
static EntData g_Ents[64] = {};
static int     g_EntCount = 0;

// Screen-space smoothing cache - fixes box stretching
struct ScreenCache {
    Vec2  sFeet, sHead;
    float midX, top, bottom;
    float bH, bW;
    float x0, y0, x1, y1;
    bool  valid;
};
static ScreenCache g_SCache[64] = {};

// ─────────────────────────────────────────────────────────────
// DX12 STATE
// ─────────────────────────────────────────────────────────────
#define BOCW_BUFFER_COUNT 3

constexpr uintptr_t WRAPPER_RVA = 0x17CF8AE8;

static ID3D12Device* g_D12Device = nullptr;
static ID3D12DescriptorHeap* g_RtvHeap = nullptr;
static ID3D12DescriptorHeap* g_SrvHeap = nullptr;
static ID3D12CommandQueue* g_CmdQueue = nullptr;
static ID3D12GraphicsCommandList* g_CmdList = nullptr;
static ID3D12CommandAllocator* g_CmdAlloc[BOCW_BUFFER_COUNT] = {};
static ID3D12Resource* g_BackBuf[BOCW_BUFFER_COUNT] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE g_RtvHandle[BOCW_BUFFER_COUNT] = {};
static HWND                        g_GameHwnd = nullptr;
static bool                        g_D12Inited = false;
static UINT                        g_BufferCount = 0;

static uintptr_t* g_PresentVtSlot = nullptr;
static uintptr_t  g_PresentOrig = 0;
static uintptr_t* g_ExecuteVtSlot = nullptr;
static uintptr_t  g_ExecuteOrig = 0;

typedef HRESULT(__stdcall* fnPresent)(IDXGISwapChain3*, UINT, UINT);
typedef void(__stdcall* fnExecute)(ID3D12CommandQueue*, UINT,
    ID3D12CommandList* const*);
static fnPresent g_OrigPresent = nullptr;
static fnExecute g_OrigExecute = nullptr;

static ID3D12CommandQueue* g_DummyQ = nullptr;
static ID3D12Device* g_DummyDev = nullptr;
static IDXGIFactory4* g_DummyFac = nullptr;

// ─────────────────────────────────────────────────────────────
// VTABLE HOOK HELPERS
// ─────────────────────────────────────────────────────────────
static void HookVT(void* obj, int idx, void* hook,
    void** orig,
    uintptr_t** savedSlot,
    uintptr_t* savedOrig) {
    uintptr_t* vt = *(uintptr_t**)obj;
    DWORD old = 0;
    VirtualProtect(&vt[idx], sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &old);
    if (orig)      *orig = (void*)vt[idx];
    if (savedSlot) *savedSlot = &vt[idx];
    if (savedOrig) *savedOrig = vt[idx];
    vt[idx] = (uintptr_t)hook;
    VirtualProtect(&vt[idx], sizeof(uintptr_t), old, &old);
}
static void RestoreVT(uintptr_t* slot, uintptr_t orig) {
    if (!slot || !orig) return;
    DWORD old = 0;
    VirtualProtect(slot, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &old);
    *slot = orig;
    VirtualProtect(slot, sizeof(uintptr_t), old, &old);
}

// ─────────────────────────────────────────────────────────────
// GAME LOGIC
// ─────────────────────────────────────────────────────────────
static uintptr_t GetEntryBase(uintptr_t lc, int slot) {
    return lc + CL_ARRAY_BASE + (uintptr_t)(slot * ENT_STRIDE);
}

static bool UpdateCamera() {
    g_Cam.valid = false;
    uintptr_t lc = 0;
    __try { lc = decrypt_local_client_globals(); }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        g_LCPtr = 0;
        g_LocalTeam = -1;
        return false;
    }
    if (!lc || lc > 0x7FFFFFFFFFFF || !CanRead(lc + CAM_TANHX, 0x50)) {
        g_LocalTeam = -1;
        g_LCPtr = 0;
        return false;
    }
    float tanX = SR<float>(lc + CAM_TANHX);
    float tanY = SR<float>(lc + CAM_TANHY);
    if (tanX < 0.05f || tanX > 5.f) {
        g_LocalTeam = -1;
        g_LCPtr = 0;
        return false;
    }
    if (tanY < 0.05f || tanY > 5.f) tanY = tanX;
    Vec3 fwd = { SR<float>(lc + CAM_FWD_X), SR<float>(lc + CAM_FWD_Y), SR<float>(lc + CAM_FWD_Z) };
    Vec3 left = { SR<float>(lc + CAM_LEFT_X),SR<float>(lc + CAM_LEFT_Y),SR<float>(lc + CAM_LEFT_Z) };
    Vec3 up = { SR<float>(lc + CAM_UP_X),  SR<float>(lc + CAM_UP_Y),  SR<float>(lc + CAM_UP_Z) };
    Vec3 right = { -left.x,-left.y,-left.z };
    if (fwd.length() < 0.85f || fwd.length() > 1.15f) {
        g_LocalTeam = -1;
        g_LCPtr = 0;
        return false;
    }
    g_Cam.origin = { SR<float>(lc + CAM_ORG_X),SR<float>(lc + CAM_ORG_Y),SR<float>(lc + CAM_ORG_Z) };
    g_Cam.fwd = fwd.norm();
    g_Cam.right = right.norm();
    g_Cam.up = up.norm();
    g_Cam.tanHX = tanX;
    g_Cam.tanHY = tanY;
    g_Cam.valid = true;
    g_LCPtr = lc;
    return true;
}

static bool W2S(const Vec3& world, Vec2& out) {
    if (!g_Cam.valid) return false;
    Vec3  d = world - g_Cam.origin;
    float dep = d.dot(g_Cam.fwd);
    if (dep < 1.f) return false;
    float r = d.dot(g_Cam.right);
    float u = d.dot(g_Cam.up);
    float sx = (1.f + r / (g_Cam.tanHX * dep)) * (float)SCREEN_W * 0.5f;
    float sy = (1.f - u / (g_Cam.tanHY * dep)) * (float)SCREEN_H * 0.5f;
    if (!isfinite(sx) || !isfinite(sy)) return false;
    out = { sx,sy };
    return sx > -500.f && sx<SCREEN_W + 500.f && sy>-500.f && sy < SCREEN_H + 500.f;
}

static void UpdateEntities() {
    g_EntCount = 0;
    if (!g_Cam.valid || !g_LCPtr) return;

    uintptr_t lc = g_LCPtr;

    if (!CanRead(lc + CL_ARRAY_BASE, 8)) {
        g_LCPtr = 0;
        return;
    }

    int   localSlot = -1;
    float bestDist = 300.f;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        uintptr_t e = GetEntryBase(lc, i);
        if (e < 0x10000 || e > 0x7FFFFFFFFFFF) continue;
        if (!CanRead(e, ENT_STRIDE))          continue;
        if (!SR<uint8_t>(e + ENT_INFOVALID))  continue;
        if (SR<int>(e + ENT_VALID255) != 255) continue;
        int t = SR<int>(e + ENT_TEAM);
        if (t != 1 && t != 2) continue;

        float ex = SR<float>(e + ENT_POS_X);
        float ey = SR<float>(e + ENT_POS_Y);
        float ez = SR<float>(e + ENT_POS_Z);
        if (!isfinite(ex) || !isfinite(ey) || !isfinite(ez)) continue;

        Vec3  ep = { ex, ey, ez };
        float d = (ep - g_Cam.origin).length();

        if (d < bestDist) {
            bestDist = d;
            localSlot = i;
            g_LocalTeam = t;
        }
    }

    int enemyTeam = g_Set.enemyTeam;
    if (enemyTeam == 0) {
        if (g_LocalTeam == 1) enemyTeam = 2;
        else if (g_LocalTeam == 2) enemyTeam = 1;
    }

    if (g_Set.teamCheck && enemyTeam == 0) {
        for (int i = 0; i < MAX_PLAYERS; i++)
            g_SCache[i].valid = false;
        return;
    }

    bool activeSlot[64] = {};

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (localSlot >= 0 && i == localSlot) continue;

        uintptr_t entry = GetEntryBase(lc, i);
        if (entry < 0x10000 || entry > 0x7FFFFFFFFFFF) continue;
        if (!CanRead(entry, ENT_STRIDE))          continue;
        if (!SR<uint8_t>(entry + ENT_INFOVALID))  continue;
        if (SR<int>(entry + ENT_VALID255) != 255) continue;

        int team = SR<int>(entry + ENT_TEAM);
        int health = SR<int>(entry + ENT_HEALTH);

        if (team != 1 && team != 2) continue;
        if (health <= 0)             continue;

        if (g_Set.teamCheck && team != enemyTeam) continue;

        float ex = SR<float>(entry + ENT_POS_X);
        float ey = SR<float>(entry + ENT_POS_Y);
        float ez = SR<float>(entry + ENT_POS_Z);
        if (!isfinite(ex) || !isfinite(ey) || !isfinite(ez)) continue;

        Vec3  epos = { ex, ey, ez };
        float selfDst = (epos - g_Cam.origin).length();
        if (selfDst < 100.f) continue;

        char name[36] = {};
        if (CanRead(entry + ENT_NAME, 32)) {
            __try {
                memcpy(name, (void*)(entry + ENT_NAME), 32);
                name[31] = 0;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                strcpy_s(name, "???");
            }
        }
        bool nameOk = false;
        for (int c = 0; c < 32 && name[c]; c++)
            if (name[c] >= 32 && name[c] < 127) {
                nameOk = true; break;
            }
        if (!nameOk) strcpy_s(name, "???");

        // ── Read stance flags and compute real head height ──
        uint8_t stanceFlags = CanRead(entry + ENT_STANCE_FLAGS, 1)
            ? SR<uint8_t>(entry + ENT_STANCE_FLAGS) : 0;

        float headHeight = PLAYER_HEIGHT_STAND;
        if (stanceFlags & STANCE_BIT_PRONE)       headHeight = PLAYER_HEIGHT_PRONE;
        else if (stanceFlags & STANCE_BIT_CROUCH) headHeight = PLAYER_HEIGHT_CROUCH;

        Vec3  feet = { ex, ey, ez };
        Vec3  head = { ex, ey, ez + headHeight };
        Vec3  aim = { ex, ey, ez + PLAYER_AIM_OFFSET };
        float dist = (feet - g_Cam.origin).length();

        if (dist < 5.f || dist >(g_Set.maxDistance / 0.0254f)) continue;

        Vec2 sF = {}, sH = {};
        if (!W2S(feet, sF) || !W2S(head, sH)) continue;

        float top = (sH.y < sF.y) ? sH.y : sF.y;
        float bottom = (sH.y > sF.y) ? sH.y : sF.y;
        float bH = bottom - top;
        if (bH < 2.f || bH >(float)SCREEN_H * 2.f) continue;

        float midX = (sF.x + sH.x) * 0.5f;
        float bW = bH * 0.4f;
        float x0 = midX - bW * 0.5f;
        float y0 = top;
        float x1 = midX + bW * 0.5f;
        float y1 = bottom;
        if (x1 < 0 || x0 > SCREEN_W || y1 < 0 || y0 > SCREEN_H) continue;

        ScreenCache& sc = g_SCache[i];
        if (sc.valid) {
            float dMid = fabsf(midX - sc.midX);
            float dTop = fabsf(top - sc.top);
            if (dMid < 200.f && dTop < 200.f) {
                const float LERP = 0.35f;
                midX = sc.midX + (midX - sc.midX) * LERP;
                top = sc.top + (top - sc.top) * LERP;
                bottom = sc.bottom + (bottom - sc.bottom) * LERP;
                bH = bottom - top;
                bW = bH * 0.4f;
                x0 = midX - bW * 0.5f;
                y0 = top;
                x1 = midX + bW * 0.5f;
                y1 = bottom;
                sF.x = sc.sFeet.x + (sF.x - sc.sFeet.x) * LERP;
                sF.y = sc.sFeet.y + (sF.y - sc.sFeet.y) * LERP;
                sH.x = sc.sHead.x + (sH.x - sc.sHead.x) * LERP;
                sH.y = sc.sHead.y + (sH.y - sc.sHead.y) * LERP;
            }
        }
        sc.sFeet = sF;    sc.sHead = sH;
        sc.midX = midX;   sc.top = top;
        sc.bottom = bottom; sc.bH = bH;
        sc.bW = bW;       sc.x0 = x0;
        sc.y0 = y0;       sc.x1 = x1;
        sc.y1 = y1;       sc.valid = true;
        activeSlot[i] = true;

        EntData& e = g_Ents[g_EntCount];
        e.feet = feet;
        e.head = head;
        e.aimPos = aim;
        e.sFeet = sF;
        e.sHead = sH;
        e.dist = dist;
        e.health = health;
        e.slot = i;
        e.team = team;
        e.onScreen = true;
        e.stanceFlags = stanceFlags;
        e.headHeight = headHeight;
        strcpy_s(e.name, name);
        g_EntCount++;
        if (g_EntCount >= 64) break;
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
        if (!activeSlot[i]) g_SCache[i].valid = false;
}

static void RunAimbot() {
    if (!g_Set.aimbotEnabled) return;
    if (!g_Cam.valid || !g_EntCount) return;
    if (g_Set.menuOpen) return;
    if (!(GetAsyncKeyState(g_Set.aimbotKey) & 0x8000)) return;

    float cx = (float)SCREEN_W * 0.5f;
    float cy = (float)SCREEN_H * 0.5f;
    float fovPx = (float)SCREEN_W
        * tanf(g_Set.aimbotFOV * (float)M_PI / 180.f)
        / (g_Cam.tanHX * 2.f);

    float bestDist = fovPx;
    int   bestIdx = -1;

    for (int i = 0; i < g_EntCount; i++) {
        auto& e = g_Ents[i];
        if (!e.onScreen) continue;

        auto& sc = g_SCache[e.slot];
        if (!sc.valid) continue;

        // Compute target height based on body part + stance scaling
        float stanceScale = e.headHeight / PLAYER_HEIGHT_STAND;
        float targetHeight = e.headHeight;  // default: head
        if (g_Set.aimbotBodyPart == 1)      targetHeight = 58.f * stanceScale;
        else if (g_Set.aimbotBodyPart == 2) targetHeight = 48.f * stanceScale;
        else if (g_Set.aimbotBodyPart == 3) targetHeight = 36.f * stanceScale;

        Vec3 realTarget = { e.feet.x, e.feet.y, e.feet.z + targetHeight };
        Vec2 targetScreen;
        float aimX, aimY;

        if (W2S(realTarget, targetScreen)) {
            aimX = targetScreen.x;
            aimY = targetScreen.y;
        }
        else {
            aimX = sc.midX;
            aimY = sc.y0 + (sc.y1 - sc.y0) * 0.15f;
        }

        float dx = aimX - cx;
        float dy = aimY - cy;
        float d = sqrtf(dx * dx + dy * dy);

        if (d < bestDist) {
            bestDist = d;
            bestIdx = i;
        }
    }

    if (bestIdx < 0) return;

    auto& e = g_Ents[bestIdx];
    auto& sc = g_SCache[e.slot];
    if (!sc.valid) return;

    // Compute target height for chosen enemy
    float stanceScale = e.headHeight / PLAYER_HEIGHT_STAND;
    float targetHeight = e.headHeight;
    if (g_Set.aimbotBodyPart == 1)      targetHeight = 58.f * stanceScale;
    else if (g_Set.aimbotBodyPart == 2) targetHeight = 48.f * stanceScale;
    else if (g_Set.aimbotBodyPart == 3) targetHeight = 36.f * stanceScale;

    Vec3 realTarget = { e.feet.x, e.feet.y, e.feet.z + targetHeight };
    Vec2 targetScreen;
    float aimX, aimY;

    if (W2S(realTarget, targetScreen)) {
        aimX = targetScreen.x;
        aimY = targetScreen.y;
    }
    else {
        aimX = sc.midX;
        aimY = sc.y0 + (sc.y1 - sc.y0) * 0.15f;
    }

    float dx = aimX - cx;
    float dy = aimY - cy;
    float maxD = 80.f;
    dx = max(-maxD, min(maxD, dx));
    dy = max(-maxD, min(maxD, dy));
    dx /= g_Set.aimbotSmooth;
    dy /= g_Set.aimbotSmooth;

    if (fabsf(dx) < 0.5f && fabsf(dy) < 0.5f) return;

    INPUT inp = {};
    inp.type = INPUT_MOUSE;
    inp.mi.dwFlags = MOUSEEVENTF_MOVE;
    inp.mi.dx = (LONG)dx;
    inp.mi.dy = (LONG)dy;
    SendInput(1, &inp, sizeof(INPUT));
}

static void RunTriggerbot() {
    if (!g_Set.triggerbotEnabled) return;
    if (!g_Cam.valid || !g_EntCount) return;
    if (g_Set.menuOpen) return;

    if (g_Set.triggerbotKey != 0 &&
        !(GetAsyncKeyState(g_Set.triggerbotKey) & 0x8000)) return;

    // Snappiness mapping - now MUCH faster since no sleep blocking
    float s = g_Set.triggerbotSnap;
    if (s < 0.f) s = 0.f;
    if (s > 1.f) s = 1.f;

    // Delays: legit at 0.0, insta at 1.0
    int reactDelay = (int)(150.f - 150.f * s); // 150ms -> 0ms
    int cooldown = (int)(180.f - 160.f * s); // 180ms -> 20ms
    int holdTime = (int)(35.f - 30.f * s); // 35ms  -> 5ms
    if (holdTime < 5) holdTime = 5;

    float cx = (float)SCREEN_W * 0.5f;
    float cy = (float)SCREEN_H * 0.5f;

    float fovPx = (float)SCREEN_W
        * tanf(g_Set.triggerbotFOV * (float)M_PI / 180.f)
        / (g_Cam.tanHX * 2.f);

    bool onTarget = false;

    for (int i = 0; i < g_EntCount; i++) {
        auto& e = g_Ents[i];
        if (!e.onScreen) continue;
        auto& sc = g_SCache[e.slot];
        if (!sc.valid) continue;

        if (cx >= sc.x0 && cx <= sc.x1 &&
            cy >= sc.y0 && cy <= sc.y1)
        {
            onTarget = true;
            break;
        }

        float chestX = sc.midX;
        float chestY = sc.y0 + (sc.y1 - sc.y0) * 0.35f;
        float dx = chestX - cx;
        float dy = chestY - cy;
        float d = sqrtf(dx * dx + dy * dy);
        if (d < fovPx) {
            onTarget = true;
            break;
        }
    }

    // State machine - no blocking Sleep()
    static bool  wasOnTarget = false;
    static DWORD onTargetSince = 0;
    static DWORD lastFireTime = 0;
    static DWORD mouseDownTime = 0;
    static bool  mouseIsDown = false;

    DWORD now = GetTickCount();

    // Handle mouse UP if we're currently holding a click
    if (mouseIsDown) {
        if (now - mouseDownTime >= (DWORD)holdTime) {
            INPUT up = {};
            up.type = INPUT_MOUSE;
            up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(1, &up, sizeof(INPUT));
            mouseIsDown = false;
            lastFireTime = now;
        }
        // While holding, don't do anything else this frame
        wasOnTarget = onTarget;
        return;
    }

    // Handle acquiring target and firing
    if (onTarget) {
        if (!wasOnTarget) onTargetSince = now;

        DWORD held = now - onTargetSince;
        DWORD sinceLastFire = now - lastFireTime;

        if (held >= (DWORD)reactDelay
            && sinceLastFire > (DWORD)cooldown)
        {
            // Send mouse DOWN only - UP happens next frame(s)
            INPUT down = {};
            down.type = INPUT_MOUSE;
            down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            SendInput(1, &down, sizeof(INPUT));
            mouseIsDown = true;
            mouseDownTime = now;
        }
    }
    else {
        onTargetSince = 0;
    }

    wasOnTarget = onTarget;
}

// ─────────────────────────────────────────────────────────────
// IMGUI DRAW
// ─────────────────────────────────────────────────────────────
static void DrawESP(ImDrawList* dl) {
    if (!g_Set.espEnabled || !g_Cam.valid) return;
    if (g_Set.showCrosshair) {
        float cx = (float)SCREEN_W * 0.5f, cy = (float)SCREEN_H * 0.5f;
        dl->AddLine({ cx - 10,cy }, { cx + 10,cy }, IM_COL32(255, 255, 255, 200), 1.5f);
        dl->AddLine({ cx,cy - 10 }, { cx,cy + 10 }, IM_COL32(255, 255, 255, 200), 1.5f);
    }
    if (g_Set.aimbotEnabled && g_Cam.valid) {
        float fovPx = (float)SCREEN_W
            * tanf(g_Set.aimbotFOV * (float)M_PI / 180.f)
            / (g_Cam.tanHX * 2.f);
        dl->AddCircle({ (float)SCREEN_W * 0.5f,(float)SCREEN_H * 0.5f },
            fovPx, IM_COL32(255, 255, 255, 60), 64, 1.f);
    }
    for (int i = 0; i < g_EntCount; i++) {
        auto& e = g_Ents[i];
        auto& sc = g_SCache[e.slot];
        if (!sc.valid) continue;

        // Use smoothed screen cache values
        float x0 = sc.x0, y0 = sc.y0;
        float x1 = sc.x1, y1 = sc.y1;
        float bH = sc.bH, midX = sc.midX;
        Vec2  sF = sc.sFeet, sH = sc.sHead;

        if (bH < 2.f) continue;
        if (x1 < 0 || x0 > SCREEN_W || y1 < 0 || y0 > SCREEN_H) continue;

        float hPct = (float)e.health / (float)MAX_HEALTH;
        if (hPct > 1.f) hPct = 1.f;
        ImU32 hCol = hPct > 0.6f ? IM_COL32(0, 220, 0, 255)
            : hPct > 0.3f ? IM_COL32(255, 165, 0, 255)
            : IM_COL32(220, 0, 0, 255);

        if (g_Set.showBoxes) {
            dl->AddRect({ x0 - 1,y0 - 1 }, { x1 + 1,y1 + 1 }, IM_COL32(0, 0, 0, 180), 0, 0, 3.f);
            dl->AddRect({ x0,y0 }, { x1,y1 }, hCol, 0, 0, 1.5f);
        }
        if (g_Set.showHealth) {
            float bx = x0 - 7.f, bw = 4.f;
            dl->AddRectFilled({ bx,y0 }, { bx + bw,y1 }, IM_COL32(30, 30, 30, 200));
            float fh = bH * hPct;
            dl->AddRectFilled({ bx,y1 - fh }, { bx + bw,y1 }, hCol);
            dl->AddRect({ bx,y0 }, { bx + bw,y1 }, IM_COL32(0, 0, 0, 150), 0, 0, 1.f);
            char hps[8]; sprintf_s(hps, "%d", e.health);
            dl->AddText({ bx - 20.f,y0 + bH * 0.5f - 7.f }, IM_COL32(255, 255, 255, 220), hps);
        }
        if (g_Set.showNames && e.name[0] && strcmp(e.name, "???") != 0) {
            ImVec2 ts = ImGui::CalcTextSize(e.name);
            dl->AddText({ midX - ts.x * 0.5f,y0 - 16.f }, IM_COL32(255, 255, 255, 255), e.name);
        }

        // ── Stance indicator ──
        if (g_Set.showStance) {
            if (e.stanceFlags & STANCE_BIT_PRONE) {
                dl->AddText({ x1 + 3.f, y0 }, IM_COL32(255, 100, 100, 255), "P");
            }
            else if (e.stanceFlags & STANCE_BIT_CROUCH) {
                dl->AddText({ x1 + 3.f, y0 }, IM_COL32(255, 220, 100, 255), "C");
            }
        }

        if (g_Set.showDistance) {
            char ds[32]; sprintf_s(ds, "%.0fm", e.dist * 0.0254f);
            ImVec2 ts = ImGui::CalcTextSize(ds);
            dl->AddText({ midX - ts.x * 0.5f,y1 + 3.f }, IM_COL32(200, 200, 200, 220), ds);
        }
        if (g_Set.showSnaplines)
            dl->AddLine({ (float)SCREEN_W * 0.5f,(float)SCREEN_H },
                { sF.x,sF.y }, IM_COL32(255, 255, 50, 100), 1.f);
    }
}

static void DrawMenu() {
    if (!g_Set.menuOpen) return;
    ImGui::SetNextWindowSize({ 440, 620 }, ImGuiCond_Once);
    ImGui::SetNextWindowPos({ 50,  50 }, ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(0.88f);
    ImGui::Begin("BOCW Internal", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    ImGui::TextColored(
        g_Cam.valid ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0.2f, 0.2f, 1),
        g_Cam.valid ? "CAM OK" : "CAM FAIL");
    ImGui::SameLine();
    ImGui::Text("  Ents:%d  Team:%d", g_EntCount, g_LocalTeam);
    ImGui::Separator();

    if (ImGui::BeginTabBar("Tabs")) {

        // ── ESP TAB ───────────────────────────────────────
        if (ImGui::BeginTabItem("ESP")) {
            ImGui::Checkbox("Enable ESP", &g_Set.espEnabled);
            ImGui::Separator();
            ImGui::Checkbox("Boxes", &g_Set.showBoxes);
            ImGui::Checkbox("Health Bar", &g_Set.showHealth);
            ImGui::Checkbox("Names", &g_Set.showNames);
            ImGui::Checkbox("Distance", &g_Set.showDistance);
            ImGui::Checkbox("Snaplines", &g_Set.showSnaplines);
            ImGui::Checkbox("Crosshair", &g_Set.showCrosshair);
            ImGui::Checkbox("Stance Indicator (C/P)", &g_Set.showStance);
            ImGui::Separator();

            ImGui::Checkbox("Team Check", &g_Set.teamCheck);
            if (g_Set.teamCheck) {
                ImGui::Text("Enemy Team:");
                ImGui::SameLine();
                if (ImGui::RadioButton("Auto",
                    g_Set.enemyTeam == 0)) g_Set.enemyTeam = 0;
                ImGui::SameLine();
                if (ImGui::RadioButton("1",
                    g_Set.enemyTeam == 1)) g_Set.enemyTeam = 1;
                ImGui::SameLine();
                if (ImGui::RadioButton("2",
                    g_Set.enemyTeam == 2)) g_Set.enemyTeam = 2;
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1),
                    "(Click 1 or 2 if boxes on teammates)");
            }
            ImGui::Separator();
            ImGui::SliderFloat("Max Dist (m)",
                &g_Set.maxDistance, 10.f, 1000.f, "%.0f");
            ImGui::EndTabItem();
        }

        // ── AIMBOT TAB ────────────────────────────────────
        if (ImGui::BeginTabItem("Aimbot")) {
            ImGui::Checkbox("Enable", &g_Set.aimbotEnabled);
            if (g_Set.aimbotEnabled) {
                ImGui::Separator();

                ImGui::Text("Target Body Part:");
                ImGui::RadioButton("Head", &g_Set.aimbotBodyPart, 0);
                ImGui::SameLine();
                ImGui::RadioButton("Neck", &g_Set.aimbotBodyPart, 1);
                ImGui::SameLine();
                ImGui::RadioButton("Chest", &g_Set.aimbotBodyPart, 2);
                ImGui::SameLine();
                ImGui::RadioButton("Stomach", &g_Set.aimbotBodyPart, 3);

                ImGui::Separator();
                ImGui::SliderFloat("FOV",
                    &g_Set.aimbotFOV, 1.f, 45.f, "%.1f deg");
                ImGui::SliderFloat("Smooth",
                    &g_Set.aimbotSmooth, 1.f, 20.f, "%.1f");

                ImGui::Separator();
                ImGui::Text("Aimbot Hold Key:");
                if (ImGui::RadioButton("Right Mouse",
                    g_Set.aimbotKey == VK_RBUTTON))
                    g_Set.aimbotKey = VK_RBUTTON;
                ImGui::SameLine();
                if (ImGui::RadioButton("Left Mouse",
                    g_Set.aimbotKey == VK_LBUTTON))
                    g_Set.aimbotKey = VK_LBUTTON;
                ImGui::SameLine();
                if (ImGui::RadioButton("Mouse4",
                    g_Set.aimbotKey == VK_XBUTTON1))
                    g_Set.aimbotKey = VK_XBUTTON1;
                ImGui::SameLine();
                if (ImGui::RadioButton("Mouse5",
                    g_Set.aimbotKey == VK_XBUTTON2))
                    g_Set.aimbotKey = VK_XBUTTON2;

                if (ImGui::RadioButton("Shift",
                    g_Set.aimbotKey == VK_SHIFT))
                    g_Set.aimbotKey = VK_SHIFT;
                ImGui::SameLine();
                if (ImGui::RadioButton("Alt",
                    g_Set.aimbotKey == VK_MENU))
                    g_Set.aimbotKey = VK_MENU;
                ImGui::SameLine();
                if (ImGui::RadioButton("Ctrl",
                    g_Set.aimbotKey == VK_CONTROL))
                    g_Set.aimbotKey = VK_CONTROL;
                ImGui::SameLine();
                if (ImGui::RadioButton("Caps",
                    g_Set.aimbotKey == VK_CAPITAL))
                    g_Set.aimbotKey = VK_CAPITAL;

                ImGui::Separator();

                // Show which key is currently set
                const char* keyName = "?";
                switch (g_Set.aimbotKey) {
                case VK_RBUTTON:  keyName = "Right Mouse"; break;
                case VK_LBUTTON:  keyName = "Left Mouse";  break;
                case VK_XBUTTON1: keyName = "Mouse4";      break;
                case VK_XBUTTON2: keyName = "Mouse5";      break;
                case VK_SHIFT:    keyName = "Shift";       break;
                case VK_MENU:     keyName = "Alt";         break;
                case VK_CONTROL:  keyName = "Ctrl";        break;
                case VK_CAPITAL:  keyName = "Caps";        break;
                }
                ImGui::TextColored(ImVec4(1, 1, 0, 1),
                    "Hold: %s", keyName);
            }
            ImGui::EndTabItem();
        }

        // ── TRIGGER TAB ───────────────────────────────────
        if (ImGui::BeginTabItem("Trigger")) {
            ImGui::Checkbox("Enable Triggerbot",
                &g_Set.triggerbotEnabled);
            if (g_Set.triggerbotEnabled) {
                ImGui::Separator();

                ImGui::SliderFloat("FOV",
                    &g_Set.triggerbotFOV, 0.5f, 10.f, "%.1f deg");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1),
                    "Tighter = only fires when crosshair on enemy");

                ImGui::Separator();
                ImGui::SliderFloat("Snappiness",
                    &g_Set.triggerbotSnap, 0.f, 1.f, "%.2f");

                const char* snapLabel = "Ultra Legit";
                ImVec4 snapColor = ImVec4(0.5f, 1, 0.5f, 1);
                if (g_Set.triggerbotSnap > 0.8f) {
                    snapLabel = "INSTANT RAGE";
                    snapColor = ImVec4(1, 0.3f, 0.3f, 1);
                }
                else if (g_Set.triggerbotSnap > 0.6f) {
                    snapLabel = "Fast";
                    snapColor = ImVec4(1, 0.7f, 0.3f, 1);
                }
                else if (g_Set.triggerbotSnap > 0.4f) {
                    snapLabel = "Balanced";
                    snapColor = ImVec4(1, 1, 0.3f, 1);
                }
                else if (g_Set.triggerbotSnap > 0.2f) {
                    snapLabel = "Legit";
                    snapColor = ImVec4(0.7f, 1, 0.5f, 1);
                }
                ImGui::TextColored(snapColor, "Mode: %s", snapLabel);
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1),
                    "0.0=human slow, 1.0=instant spam");

                ImGui::Separator();
                ImGui::Text("Trigger Hold Key:");
                if (ImGui::RadioButton("Always On##t",
                    g_Set.triggerbotKey == 0))
                    g_Set.triggerbotKey = 0;
                ImGui::SameLine();
                if (ImGui::RadioButton("Mouse4##t",
                    g_Set.triggerbotKey == VK_XBUTTON1))
                    g_Set.triggerbotKey = VK_XBUTTON1;
                ImGui::SameLine();
                if (ImGui::RadioButton("Mouse5##t",
                    g_Set.triggerbotKey == VK_XBUTTON2))
                    g_Set.triggerbotKey = VK_XBUTTON2;

                if (ImGui::RadioButton("Shift##t",
                    g_Set.triggerbotKey == VK_SHIFT))
                    g_Set.triggerbotKey = VK_SHIFT;
                ImGui::SameLine();
                if (ImGui::RadioButton("Alt##t",
                    g_Set.triggerbotKey == VK_MENU))
                    g_Set.triggerbotKey = VK_MENU;
                ImGui::SameLine();
                if (ImGui::RadioButton("Ctrl##t",
                    g_Set.triggerbotKey == VK_CONTROL))
                    g_Set.triggerbotKey = VK_CONTROL;

                ImGui::Separator();
                ImGui::TextColored(ImVec4(1, 0.5f, 0, 1),
                    "Uses same enemy filter as ESP");
            }
            ImGui::EndTabItem();
        }

        // ── INFO TAB ──────────────────────────────────────
        if (ImGui::BeginTabItem("Info")) {
            ImGui::Text("Base: 0x%llX", (unsigned long long)g_Base);
            ImGui::Text("LC:   0x%llX", (unsigned long long)g_LCPtr);
            ImGui::Text("Enemy Team: %d (0=auto)", g_Set.enemyTeam);
            ImGui::Text("Local Team: %d", g_LocalTeam);
            ImGui::Text("Org:  %.0f %.0f %.0f",
                g_Cam.origin.x, g_Cam.origin.y, g_Cam.origin.z);
            ImGui::Separator();
            for (int i = 0; i < g_EntCount && i < 8; i++) {
                auto& e = g_Ents[i];
                const char* stance = "S";
                if (e.stanceFlags & STANCE_BIT_PRONE)       stance = "P";
                else if (e.stanceFlags & STANCE_BIT_CROUCH) stance = "C";

                ImGui::Text("[%d] %s HP:%d T:%d %.0fm [%s]",
                    e.slot, e.name, e.health, e.team,
                    e.dist * 0.0254f, stance);
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::Separator();
    ImGui::Spacing();
    if (ImGui::Button("Save Config", ImVec2(200.f, 28.f))) {
        SaveConfig();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Config", ImVec2(200.f, 28.f))) {
        LoadConfig();
    }
    if (g_ConfigPath[0]) {
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 0.8f),
            "%s", g_ConfigPath);
    }
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1),
        "INSERT=menu  END=quit");
    ImGui::End();
}

// ─────────────────────────────────────────────────────────────
// WNDPROC HOOK
// ─────────────────────────────────────────────────────────────
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
static WNDPROC g_OrigWndProc = nullptr;
static LRESULT CALLBACK WndProcHook(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (ImGui_ImplWin32_WndProcHandler(h, m, w, l)) return true;
    if (g_Set.menuOpen) {
        switch (m) {
        case WM_LBUTTONDOWN: case WM_LBUTTONUP:
        case WM_RBUTTONDOWN: case WM_RBUTTONUP:
        case WM_MBUTTONDOWN: case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:  case WM_MOUSEMOVE:
        case WM_KEYDOWN:     case WM_KEYUP:
        case WM_CHAR:        return 0;
        }
    }
    return CallWindowProc(g_OrigWndProc, h, m, w, l);
}

// ─────────────────────────────────────────────────────────────
// EXECUTE COMMAND LISTS HOOK
// ─────────────────────────────────────────────────────────────
static void __stdcall HookedExecute(
    ID3D12CommandQueue* queue, UINT num, ID3D12CommandList* const* lists)
{
    if (!g_CmdQueue) {
        D3D12_COMMAND_QUEUE_DESC d = queue->GetDesc();
        if (d.Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
            g_CmdQueue = queue;
    }
    g_OrigExecute(queue, num, lists);
}

// ─────────────────────────────────────────────────────────────
// PRESENT HOOK
// ─────────────────────────────────────────────────────────────
static HRESULT __stdcall HookedPresent(
    IDXGISwapChain3* sc, UINT sync, UINT flags)
{
    if (!g_CmdQueue)
        return g_OrigPresent(sc, 0, flags);

    if (!g_D12Inited) {
        if (FAILED(sc->GetDevice(__uuidof(ID3D12Device), (void**)&g_D12Device)))
            return g_OrigPresent(sc, 0, flags);

        DXGI_SWAP_CHAIN_DESC scd = {};
        sc->GetDesc(&scd);
        g_GameHwnd = scd.OutputWindow;

        DXGI_SWAP_CHAIN_DESC1 scd1 = {};
        sc->GetDesc1(&scd1);
        SCREEN_W = (int)scd1.Width;
        SCREEN_H = (int)scd1.Height;
        g_BufferCount = min(scd1.BufferCount, (UINT)BOCW_BUFFER_COUNT);
        DXGI_FORMAT fmt = scd1.Format;

        D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
        rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvDesc.NumDescriptors = g_BufferCount;
        if (FAILED(g_D12Device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&g_RtvHeap))))
            return g_OrigPresent(sc, 0, flags);

        D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
        srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvDesc.NumDescriptors = 1;
        srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (FAILED(g_D12Device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&g_SrvHeap))))
            return g_OrigPresent(sc, 0, flags);

        UINT rtvSize = g_D12Device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
            g_RtvHeap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0;i < g_BufferCount;i++) {
            if (FAILED(g_D12Device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_CmdAlloc[i]))))
                return g_OrigPresent(sc, 0, flags);
            if (FAILED(sc->GetBuffer(i, IID_PPV_ARGS(&g_BackBuf[i]))))
                return g_OrigPresent(sc, 0, flags);
            g_D12Device->CreateRenderTargetView(g_BackBuf[i], nullptr, rtvHandle);
            g_RtvHandle[i] = rtvHandle;
            rtvHandle.ptr += rtvSize;
        }

        if (FAILED(g_D12Device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            g_CmdAlloc[0], nullptr, IID_PPV_ARGS(&g_CmdList))))
            return g_OrigPresent(sc, 0, flags);
        g_CmdList->Close();

        g_OrigWndProc = (WNDPROC)SetWindowLongPtr(
            g_GameHwnd, GWLP_WNDPROC, (LONG_PTR)WndProcHook);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.IniFilename = nullptr;
        io.DisplaySize = ImVec2((float)SCREEN_W, (float)SCREEN_H);
        io.DeltaTime = 1.0f / 60.0f;

        // Build font atlas before DX12 init
        io.Fonts->AddFontDefault();
        {
            unsigned char* pixels = nullptr;
            int fw = 0, fh = 0;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &fw, &fh);
        }

        ImGui::StyleColorsDark();
        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowRounding = 6.f; s.FrameRounding = 4.f;
        s.GrabRounding = 4.f;   s.WindowBorderSize = 1.f;
        s.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.92f);
        s.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.50f, 1.f);
        s.Colors[ImGuiCol_CheckMark] = ImVec4(0.3f, 0.9f, 0.3f, 1.f);
        s.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3f, 0.6f, 0.9f, 1.f);
        s.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.70f, 0.60f);
        s.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.80f, 0.70f);
        s.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.80f, 0.55f);
        s.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.90f, 0.65f);

        ImGui_ImplWin32_Init(g_GameHwnd);
        ImGui_ImplDX12_Init(g_D12Device, (int)g_BufferCount, fmt,
            g_SrvHeap,
            g_SrvHeap->GetCPUDescriptorHandleForHeapStart(),
            g_SrvHeap->GetGPUDescriptorHandleForHeapStart());

        // Upload fonts to GPU
        __try {
            g_CmdAlloc[0]->Reset();
            g_CmdList->Reset(g_CmdAlloc[0], nullptr);
            ImGui_ImplDX12_NewFrame();
            g_CmdList->Close();
            ID3D12CommandList* ul[] = { g_CmdList };
            g_CmdQueue->ExecuteCommandLists(1, ul);
            ID3D12Fence* fence = nullptr;
            if (SUCCEEDED(g_D12Device->CreateFence(
                0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
                HANDLE ev = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                fence->SetEventOnCompletion(1, ev);
                g_CmdQueue->Signal(fence, 1);
                WaitForSingleObject(ev, 2000);
                CloseHandle(ev);
                fence->Release();
            }
            g_CmdAlloc[0]->Reset();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            return g_OrigPresent(sc, 0, flags);
        }

        g_D12Inited = true;
        return g_OrigPresent(sc, 0, flags);
    }

    // ── Keys ─────────────────────────────────────────────────
        // ── Keys ─────────────────────────────────────────────────
    if (GetAsyncKeyState(VK_END) & 1)    g_Running = false;
    if (GetAsyncKeyState(VK_INSERT) & 1) {
        g_Set.menuOpen = !g_Set.menuOpen;
        ShowCursor(g_Set.menuOpen);
    }

    // ═══ CRASH GUARD - reset state on any game logic crash ═══
    __try {
        UpdateCamera();
        UpdateEntities();
        RunAimbot();
        RunTriggerbot();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        // Game state went bad - reset everything and let next frame recover
        g_Cam.valid = false;
        g_LCPtr = 0;
        g_LocalTeam = -1;
        g_EntCount = 0;
        for (int i = 0; i < 64; i++) {
            g_SCache[i].valid = false;
        }
    }

    UINT bufIdx = sc->GetCurrentBackBufferIndex();
    if (bufIdx >= g_BufferCount)
        return g_OrigPresent(sc, 0, flags);

    __try {
        g_CmdAlloc[bufIdx]->Reset();
        g_CmdList->Reset(g_CmdAlloc[bufIdx], nullptr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return g_OrigPresent(sc, 0, flags);
    }

    __try {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        __try { g_CmdList->Close(); }
        __except (1) {}
        return g_OrigPresent(sc, 0, flags);
    }

    ImGui::GetIO().DisplaySize = ImVec2((float)SCREEN_W, (float)SCREEN_H);
    ImGui::GetIO().DeltaTime = 1.0f / 60.0f;

    __try {
        ImGui::NewFrame();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        __try { g_CmdList->Close(); }
        __except (1) {}
        return g_OrigPresent(sc, 0, flags);
    }

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = g_BackBuf[bufIdx];
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    __try {
        g_CmdList->ResourceBarrier(1, &barrier);
        g_CmdList->OMSetRenderTargets(1, &g_RtvHandle[bufIdx], FALSE, nullptr);
        g_CmdList->SetDescriptorHeaps(1, &g_SrvHeap);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        __try { ImGui::EndFrame(); }
        __except (1) {}
        __try { g_CmdList->Close(); }
        __except (1) {}
        return g_OrigPresent(sc, 0, flags);
    }

    __try {
        DrawESP(ImGui::GetBackgroundDrawList());
        DrawMenu();
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_CmdList);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        __try { g_CmdList->Close(); }
        __except (1) {}
        return g_OrigPresent(sc, 0, flags);
    }

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    __try {
        g_CmdList->ResourceBarrier(1, &barrier);
        g_CmdList->Close();
        ID3D12CommandList* lists[] = { g_CmdList };
        g_CmdQueue->ExecuteCommandLists(1, lists);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return g_OrigPresent(sc, 0, flags);
    }

    return g_OrigPresent(sc, 0, flags);
}

// ─────────────────────────────────────────────────────────────
// HOOK SETUP
// ─────────────────────────────────────────────────────────────
static bool SetupHooks() {
    WNDCLASSEX wc = { sizeof(wc),CS_HREDRAW | CS_VREDRAW,
        DefWindowProc,0,0,GetModuleHandle(0),
        0,0,0,0,L"BOCW_DQ",0 };
    RegisterClassEx(&wc);
    HWND hw = CreateWindow(wc.lpszClassName, L"",
        WS_OVERLAPPEDWINDOW, 0, 0, 10, 10,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&g_DummyFac)))) {
        DestroyWindow(hw); return false;
    }
    if (FAILED(D3D12CreateDevice(nullptr,
        D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&g_DummyDev)))) {
        DestroyWindow(hw); return false;
    }
    D3D12_COMMAND_QUEUE_DESC qd = {};
    if (FAILED(g_DummyDev->CreateCommandQueue(&qd, IID_PPV_ARGS(&g_DummyQ)))) {
        DestroyWindow(hw); return false;
    }

    HookVT(g_DummyQ, 10, HookedExecute,
        (void**)&g_OrigExecute,
        &g_ExecuteVtSlot, &g_ExecuteOrig);

    DestroyWindow(hw);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    // Wait for game's CommandQueue
    for (int i = 0;i < 300 && !g_CmdQueue;i++) Sleep(100);
    if (!g_CmdQueue) return false;

    // Create swapchain using game's queue to get matching vtable
    HWND tmpHwnd = CreateWindowA("STATIC", nullptr,
        WS_POPUP, 0, 0, 8, 8, nullptr, nullptr, GetModuleHandle(0), nullptr);

    IDXGIFactory4* fac2 = nullptr;
    IDXGISwapChain1* sc1 = nullptr;
    bool hookOk = false;

    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&fac2)))) {
        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.BufferCount = 2;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        if (SUCCEEDED(fac2->CreateSwapChainForHwnd(
            g_CmdQueue, tmpHwnd, &sd, nullptr, nullptr, &sc1)) && sc1) {
            HookVT(sc1, 8, HookedPresent,
                (void**)&g_OrigPresent,
                &g_PresentVtSlot, &g_PresentOrig);
            hookOk = g_OrigPresent != nullptr;
            sc1->Release();
        }
        fac2->Release();
    }

    DestroyWindow(tmpHwnd);
    return hookOk;
}

// ─────────────────────────────────────────────────────────────
// MAIN THREAD
// ─────────────────────────────────────────────────────────────
DWORD WINAPI MainThread(LPVOID param) {
    g_Module = (HMODULE)param;
    g_Base = (uintptr_t)GetModuleHandleA(NULL);

    __try {
        if (!SetupHooks()) {
            FreeLibraryAndExitThread(g_Module, 0);
            return 0;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        FreeLibraryAndExitThread(g_Module, 0);
        return 0;
    }
    LoadConfig();

    while (g_Running) Sleep(100);

    if (g_GameHwnd && g_OrigWndProc)
        SetWindowLongPtr(g_GameHwnd, GWLP_WNDPROC, (LONG_PTR)g_OrigWndProc);
    ShowCursor(TRUE);
    RestoreVT(g_PresentVtSlot, g_PresentOrig);
    RestoreVT(g_ExecuteVtSlot, g_ExecuteOrig);
    if (g_D12Inited) {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
    if (g_CmdList) g_CmdList->Release();
    for (UINT i = 0;i < g_BufferCount;i++) {
        if (g_CmdAlloc[i]) g_CmdAlloc[i]->Release();
        if (g_BackBuf[i])  g_BackBuf[i]->Release();
    }
    if (g_SrvHeap)   g_SrvHeap->Release();
    if (g_RtvHeap)   g_RtvHeap->Release();
    if (g_D12Device) g_D12Device->Release();
    if (g_DummyQ)    g_DummyQ->Release();
    if (g_DummyDev)  g_DummyDev->Release();
    if (g_DummyFac)  g_DummyFac->Release();
    Sleep(200);
    FreeLibraryAndExitThread(g_Module, 0);
    return 0;
}

// ─────────────────────────────────────────────────────────────
// DLLMAIN
// ─────────────────────────────────────────────────────────────
BOOL APIENTRY DllMain(HMODULE h, DWORD r, LPVOID) {
    if (r == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(h);
        CreateThread(nullptr, 0, MainThread, h, 0, nullptr);
    }
    return TRUE;
}