#pragma once
#include <stdint.h>

// ─────────────────────────────────────────────────────────────
// BOCW OFFSETS  (confirmed via IDA + runtime diagnostic)
// IDA base: 0x7FF61B400000
// ─────────────────────────────────────────────────────────────

// ── Camera (from LC pointer) ──────────────────────────────────
constexpr int CAM_TANHX = 0x26E7C;
constexpr int CAM_TANHY = 0x26E80;
constexpr int CAM_ORG_X = 0x26E8C;
constexpr int CAM_ORG_Y = 0x26E90;
constexpr int CAM_ORG_Z = 0x26E94;
constexpr int CAM_FWD_X = 0x26E98;
constexpr int CAM_FWD_Y = 0x26E9C;
constexpr int CAM_FWD_Z = 0x26EA0;
constexpr int CAM_LEFT_X = 0x26EA4;
constexpr int CAM_LEFT_Y = 0x26EA8;
constexpr int CAM_LEFT_Z = 0x26EAC;
constexpr int CAM_UP_X = 0x26EB0;
constexpr int CAM_UP_Y = 0x26EB4;
constexpr int CAM_UP_Z = 0x26EB8;

// ── Entity array (LC + CL_ARRAY_BASE + slot*ENT_STRIDE) ───────
constexpr uintptr_t CL_ARRAY_BASE = 0x799980;
constexpr int       ENT_STRIDE = 0x19F8;
constexpr int       MAX_PLAYERS = 18;

// ── Entity offsets (relative to entry base) ───────────────────
constexpr int ENT_INFOVALID = 0x8A;
constexpr int ENT_NAME = 0x8B;
constexpr int ENT_HEALTH = 0x278;
constexpr int ENT_POS_X = 0x1444;
constexpr int ENT_POS_Y = 0x1448;
constexpr int ENT_POS_Z = 0x144C;
constexpr int ENT_VALID255 = 0x15D4;
constexpr int ENT_TEAM = 0x11C8;
constexpr uintptr_t LOCAL_CLIENT_ARRAY_RVA = 0x19289050;
constexpr int       LOCAL_CLIENT_STRUCT_SZ = 0x3C;
constexpr int       LOCAL_CLIENT_NUM_OFFSET = 0xC;
constexpr int ENT_STANCE_FLAGS = 0x11AF;

constexpr uint8_t STANCE_BIT_MOVING = 0x10;   // 16
constexpr uint8_t STANCE_BIT_CROUCH = 0x40;   // 64
constexpr uint8_t STANCE_BIT_PRONE = 0x80;   // 128

// Player height offsets by stance
constexpr float PLAYER_HEIGHT_STAND = 66.f;
constexpr float PLAYER_HEIGHT_CROUCH = 44.f;
constexpr float PLAYER_HEIGHT_PRONE = 22.f;

// ── Player dimensions ─────────────────────────────────────────
constexpr float PLAYER_HEIGHT = 66.f;
constexpr float PLAYER_AIM_OFFSET = 50.f;

// ── Max health ────────────────────────────────────────────────
constexpr int MAX_HEALTH = 150;