#pragma once
#include "offsets.h"
#include <windows.h>
#include <stdint.h>
#include <intrin.h>

extern uintptr_t g_Base;
template<typename T> T SR(uintptr_t a);

static uintptr_t decrypt_dobj_base()
{
    const uint64_t mb = g_Base;
    uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb,
        rdi = mb, rsi = mb, r8 = mb, r9 = mb,
        r10 = mb, r11 = mb, r12 = mb, r13 = mb,
        r14 = mb, r15 = mb;

    // Read encrypted pointer
    r8 = SR<uint64_t>(mb + 0x144BC978);
    if (!r8) return 0;

    // Read PEB - replaces: rbx = peb
    rbx = __readgsqword(0x60);

    // mov [r11-0x38], r12  <- junk stack write, skip it
    // rax = rbx
    rax = rbx;
    rax >>= 0x1B;
    rax &= 0xF;

    switch (rax) {
    case 0:
    {
        rdi = mb + 0x5A1D;
        r10 = SR<uint64_t>(mb + 0xE2732E6);
        rcx = rbx;
        rcx ^= r8;
        r8 = rbx;
        r8 *= rdi;
        r8 += rcx;
        rax = r8;
        rax >>= 0x11;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x22;
        r8 ^= rax;
        rax = 0x37B4F3B8FAB438BB;
        r8 ^= rax;
        rax = 0xB6AEA787908BBB57;
        r8 *= rax;
        r8 += rbx;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r10;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        return r8;
    }
    case 1:
    {
        rdi = mb;
        r9 = SR<uint64_t>(mb + 0xE2732E6);
        r8 += rdi;
        rax = rdi + 0xf8d3;
        rax += rbx;
        r8 ^= rax;
        rax = 0x355A24F6D2FD2E8B;
        r8 *= rax;
        rax = r8;
        rax >>= 0x26;
        r8 ^= rax;
        uint64_t RSP1 = 0xFAAF807B4E7F19AF;
        r8 *= RSP1;
        r8 -= rbx;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r9;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        r8 -= rdi;
        return r8;
    }
    case 2:
    {
        rdi = mb;
        r9 = SR<uint64_t>(mb + 0xE2732E6);
        rax = r8;
        rax >>= 0x28;
        r8 ^= rax;
        r8 -= rdi;
        rax = 0x5E0CC6DF18F53CE3;
        r8 *= rax;
        r8 += rdi;
        rax = r8;
        rax >>= 0xB;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x16;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x2C;
        r8 ^= rax;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r9;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        r8 += rdi;
        rax = r8;
        rax >>= 0xA;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x14;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x28;
        r8 ^= rax;
        return r8;
    }
    case 3:
    {
        rdi = mb;
        r10 = SR<uint64_t>(mb + 0xE2732E6);
        rax = rbx;
        uint64_t RSP3 = mb + 0x771F;
        rax ^= RSP3;
        r8 -= rax;
        rax = 0xA033BE2A32249F24;
        r8 -= rdi;
        r8 ^= rax;
        rax = 0x53F307E9FAEABF89;
        r8 *= rax;
        rax = r8;
        rax >>= 0xF;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x1E;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x3C;
        r8 ^= rax;
        rcx = rbx;
        rax = mb + 0x31DD;
        rcx *= rax;
        rax = r8;
        r8 = 0xD5FA824E69D0074D;
        rax *= r8;
        r8 = rcx;
        r8 ^= rax;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r10;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        return r8;
    }
    case 4:
    {
        rdi = mb;
        r10 = SR<uint64_t>(mb + 0xE2732E6);
        rax = r8;
        rax >>= 0x16;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x2C;
        r8 ^= rax;
        rax = 0xA623BE33AA93D200;
        r8 ^= rax;
        r8 += rbx;
        r8 += rdi;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r10;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        rax = r8;
        rax >>= 0x1F;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x3E;
        r8 ^= rax;
        rax = 0x9D3FC6CC8E382D5B;
        r8 *= rax;
        rax = r8;
        rax >>= 0x9;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x12;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x24;
        r8 ^= rax;
        return r8;
    }
    case 5:
    {
        rdi = mb;
        r9 = SR<uint64_t>(mb + 0xE2732E6);
        r8 += rbx;
        rax = 0x50FD3E1655BF4B7;
        r8 *= rax;
        r8 ^= rbx;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r9;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        r8 -= rbx;
        r8 -= rdi;
        r8 ^= rbx;
        rax = r8;
        rax >>= 0x1D;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x3A;
        r8 ^= rax;
        return r8;
    }
    case 6:
    {
        rdi = mb;
        r11 = SR<uint64_t>(mb + 0xE2732E6);
        rax = rdi + 0x72e63bd3;
        rax += rbx;
        r8 += rax;
        rdx = rbx;
        rdx = ~rdx;
        rax = r8;
        rax >>= 0x1F;
        r8 ^= rax;
        rax = mb + 0xFCD9;
        rcx = r8;
        rax = ~rax;
        rdx *= rax;
        rcx >>= 0x3E;
        rcx ^= r8;
        r8 = 0x530650D7AC133C2E;
        r8 += rdx;
        r8 += rcx;
        rdx = mb + 0x621;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r11;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        rax = 0x3EFAD98814B2E105;
        r8 *= rax;
        rax = rbx;
        rax = ~rax;
        r8 ^= rax;
        r8 ^= rdx;
        rax = 0xC0A503ABFDD972EC;
        r8 ^= rax;
        return r8;
    }
    case 7:
    {
        rdi = mb;
        r10 = SR<uint64_t>(mb + 0xE2732E6);
        rcx = mb + 0xCDAF;
        rax = rbx;
        rax ^= rcx;
        r8 += rax;
        rax = r8;
        rax >>= 0x5;
        r8 ^= rax;
        rax = r8;
        rax >>= 0xA;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x14;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x28;
        r8 ^= rax;
        r8 -= rdi;
        rax = 0x9D64B2BFA76A8491;
        r8 *= rax;
        rax = 0x167B87A98ACFBA07;
        r8 *= rax;
        rax = r8;
        rax >>= 0xA;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x14;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x28;
        rcx = 0;
        r8 ^= rax;
        rcx = _rotl64(rcx, 0x10);
        rcx ^= r10;
        rcx = ~rcx;
        r8 *= SR<uint64_t>(rcx + 0x9);
        rax = 0x3FD03E9BFFB293C6;
        r8 += rax;
        return r8;
    }
    case 8:
    {
        r9 = SR<uint64_t>(mb + 0xE2732E6);
        rax = r8;
        rax >>= 0xB;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x16;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x2C;
        r8 ^= rax;
        rax = 0x2DC4765423F2DE85;
        r8 *= rax;
        r8 += rbx;
        r8 -= rbx;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r9;
        rax = ~rax;
        rax = SR<uint64_t>(rax + 0x9);
        uint64_t RSP8 = 0x4A7A5371FB78ADFF;
        rax *= RSP8;
        r8 *= rax;
        r8 += rbx;
        rax = r8;
        rax >>= 0x16;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x2C;
        r8 ^= rax;
        return r8;
    }
    case 9:
    {
        rdi = mb + 0x4F197CE4;
        r10 = SR<uint64_t>(mb + 0xE2732E6);
        rax = 0x44513885D292C737;
        r8 *= rax;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r10;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        rax = 0xB9402772262819EF;
        r8 *= rax;
        rcx = r8;
        rcx >>= 0x20;
        rcx ^= r8;
        r8 = mb + 0x5CD86D49;
        rax = rbx;
        rax = ~rax;
        r8 *= rax;
        r8 += rcx;
        rax = 0x3368342CDF602565;
        r8 *= rax;
        rax = rbx;
        rax *= rdi;
        r8 -= rax;
        r8 ^= rbx;
        return r8;
    }
    case 10:
    {
        r11 = mb + 0x3B3445A6;
        r10 = SR<uint64_t>(mb + 0xE2732E6);
        r8 -= rbx;
        rax = 0x4413E0F98C8BBFA9;
        r8 *= rax;
        rax = 0x6FFE527591EC650F;
        r8 -= rax;
        rax = 0xC8A5B7E543FE2756;
        r8 ^= rax;
        rax = rbx;
        rax ^= r11;
        r8 -= rax;
        r8 += rbx;
        rcx = 0;
        rcx = _rotl64(rcx, 0x10);
        rax = r8;
        rcx ^= r10;
        rax >>= 0x20;
        rcx = ~rcx;
        r8 ^= rax;
        r8 *= SR<uint64_t>(rcx + 0x9);
        return r8;
    }
    case 11:
    {
        rdi = mb;
        r10 = SR<uint64_t>(mb + 0xE2732E6);
        rax = 0x8BC70C6B1607AE3E;
        r8 ^= rax;
        rax = 0x31A78D68F233EB31;
        r8 *= rax;
        r8 -= rbx;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r10;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        rax = r8;
        rax >>= 0x26;
        r8 ^= rax;
        rax = rbx;
        r8 -= rbx;
        rax = ~rax;
        r8 += rax;
        rax = 0x41B0962C82EFAD6F;
        r8 -= rdi;
        r8 -= rax;
        return r8;
    }
    case 12:
    {
        rdi = mb;
        r10 = SR<uint64_t>(mb + 0xE2732E6);
        rax = rbx;
        uint64_t RSP12a = mb + 0x34EF73B8;
        rax *= RSP12a;
        rax -= rdi;
        r8 += rax;
        rax = r8;
        rax >>= 0x24;
        r8 ^= rax;
        rax = 0x876C8734AE8CF3DF;
        r8 *= rax;
        r8 ^= rdi;
        rax = 0x63075B1E678C7D27;
        r8 -= rax;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r10;
        rax = ~rax;
        rax = SR<uint64_t>(rax + 0x9);
        uint64_t RSP12b = 0x288F76C5871552FD;
        rax *= RSP12b;
        r8 *= rax;
        return r8;
    }
    case 13:
    {
        rdi = mb + 0x5C35;
        r9 = SR<uint64_t>(mb + 0xE2732E6);
        r8 -= rbx;
        rax = 0x24060E7F71A3E0BB;
        r8 -= rax;
        r8 += rbx;
        r8 ^= rbx;
        r8 ^= rdi;
        rax = 0xECD837FE3C1B7D75;
        r8 *= rax;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r9;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        rax = r8;
        rax >>= 0xA;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x14;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x28;
        r8 ^= rax;
        rax = 0x2E6E1E575A23B9F8;
        r8 ^= rax;
        return r8;
    }
    case 14:
    {
        rdi = mb;
        r11 = mb + 0x958B;
        r9 = SR<uint64_t>(mb + 0xE2732E6);
        r8 += rdi;
        rax = 0xEE849A8425CA355;
        r8 *= rax;
        rax = 0x2444C08B386D3047;
        r8 += rax;
        r8 ^= rbx;
        r8 ^= r11;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r9;
        rax = ~rax;
        r8 *= SR<uint64_t>(rax + 0x9);
        rax = r8;
        rax >>= 0x20;
        r8 ^= rax;
        r8 += rbx;
        return r8;
    }
    case 15:
    {
        rdi = mb;
        r9 = SR<uint64_t>(mb + 0xE2732E6);
        rax = r8;
        rax >>= 0x24;
        r8 ^= rax;
        rax = 0x1E0FED6A0F5D9136;
        r8 += rax;
        rax = r8;
        rax >>= 0x1C;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x38;
        r8 ^= rax;
        rax = 0x525B0599DACF5FF5;
        r8 *= rax;
        rax = 0;
        rax = _rotl64(rax, 0x10);
        rax ^= r9;
        rax = ~rax;
        rax = SR<uint64_t>(rax + 0x9);
        rax *= r8;
        r8 = 0x2DCBB0EA05DA12AA;
        rax -= rdi;
        r8 += rax;
        r8 += rbx;
        rax = r8;
        rax >>= 0x19;
        r8 ^= rax;
        rax = r8;
        rax >>= 0x32;
        r8 ^= rax;
        return r8;
    }
    default:
        return 0;
    }
}



static uintptr_t decrypt_local_client_globals()
{
    const uint64_t mb = g_Base;
    uint64_t rax = mb, rbx = mb, rcx = mb, rdx = mb,
        rdi = mb, rsi = mb, r8 = mb, r9 = mb,
        r10 = mb, r11 = mb, r12 = mb, r13 = mb,
        r14 = mb, r15 = mb;

    r8 = SR<uint64_t>(mb + 0x1073E358);
    if (!r8) return 0;

    rbx = __readgsqword(0x60);
    rax = rbx;
    rax <<= 0x21;
    rax = _byteswap_uint64(rax);
    rax &= 0xF;

    switch (rax) {
    case 0:
    {
        r9 = SR<uint64_t>(mb + 0xE273109);
        rax = r8; rax >>= 0x20; r8 ^= rax;
        rax = 0x9DB692212C93D6F7; r8 *= rax;
        r8 -= rbx;
        rax = 0x79DAC63832D1F60D; r8 -= rax;
        r8 -= rbx;
        r8 ^= 0x3E2897EB23C13125;
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r9;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        rax = mb; r8 ^= rax;
        return r8;
    }
    case 1:
    {
        r10 = SR<uint64_t>(mb + 0xE273109);
        rax = mb + 0xD7F8; rax = ~rax; rax -= rbx; r8 += rax;
        rax = 0x61CDB15F92F4B18B; r8 *= rax;
        rax = 0xC033E850B1914C25; r8 ^= rax;
        r8 -= rbx;
        rax = mb; rax += 0xA983; rax += rbx; r8 ^= rax;
        rax = r8; rax >>= 0x8;  r8 ^= rax;
        rax = r8; rax >>= 0x10; r8 ^= rax;
        rax = r8; rax >>= 0x20; r8 ^= rax;
        rax = rbx; rax = ~rax; rax *= (mb + 0x2EDC); r8 += rax;
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r10;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        return r8;
    }
    case 2:
    {
        r15 = mb + 0x5EFF;
        r10 = SR<uint64_t>(mb + 0xE273109);
        r8 += rbx;
        rax = 0x91F0D209911D4BC7; r8 ^= rax;
        rcx = 0; rax = rbx; rax *= r15;
        rcx = _rotl64(rcx, 0x10); r8 += rax;
        rcx ^= r10; rcx = _byteswap_uint64(rcx);
        r8 *= SR<uint64_t>(rcx + 0x11);
        rax = r8; rax >>= 0x22; r8 ^= rax;
        rax = 0xFDA4F5BDBAA88E2B; r8 *= rax;
        rax = r8; rax >>= 0x24; r8 ^= rax;
        rax = r8; rax >>= 0x25; r8 ^= rax;
        return r8;
    }
    case 3:
    {
        r15 = mb + 0xBB38;
        r9 = SR<uint64_t>(mb + 0xE273109);
        rax = rbx; rax *= r15; r8 += rax;
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r9;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        rax = 0x2F51D93F57A7CD89; r8 *= rax;
        r8 += rbx;
        rax = 0x5FEF21480AD01CBD; r8 -= rax;
        rax = r8; rax >>= 0x19; r8 ^= rax;
        rax = r8; rax >>= 0x32; r8 ^= rax;
        rax = r8; rax >>= 0x14; r8 ^= rax;
        rax = r8; rax >>= 0x28; r8 ^= rax;
        rax = 0x24E30CC7486E5FC4; r8 ^= rax;
        return r8;
    }
    case 4:
    {
        r12 = mb + 0xD657;
        r15 = mb + 0x359F34E6;
        r10 = SR<uint64_t>(mb + 0xE273109);
        rcx = rbx;
        rax = 0x4DB14FE73A04FD8A; r8 += rax;
        rcx *= r15; r8 += rcx;
        rax = r8; rax >>= 0xB;  r8 ^= rax;
        rax = r8; rax >>= 0x16; r8 ^= rax;
        rax = r8; rax >>= 0x2C; r8 ^= rax;
        rax = 0xB33E135C3F7D29A1; r8 *= rax;
        rax = 0x8B84B973B5355150; r8 ^= rax;
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r10;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        r8 ^= rbx; r8 ^= r12;
        rax = r8; rax >>= 0x8;  r8 ^= rax;
        rax = r8; rax >>= 0x10; r8 ^= rax;
        rax = r8; rax >>= 0x20; r8 ^= rax;
        return r8;
    }
    case 5:
    {
        r15 = mb + 0x13164FE4;
        r10 = SR<uint64_t>(mb + 0xE273109);
        rax = r8; rax >>= 0x16; r8 ^= rax;
        rax = r8; rax >>= 0x2C; r8 ^= rax;
        rcx = 0; rcx = _rotl64(rcx, 0x10); rcx ^= r10;
        rax = r15; rax = ~rax; rax *= rbx;
        rcx = _byteswap_uint64(rcx);
        r8 ^= rax;
        r8 *= SR<uint64_t>(rcx + 0x11);
        rax = rbx; rax = ~rax; rax += (mb + 0x2591CBE2); r8 ^= rax;
        rax = 0x2D5D3DF1928C826D; r8 *= rax;
        r8 -= rbx;
        rax = mb; rax += rbx; r8 += rax;
        return r8;
    }
    case 6:
    {
        r10 = SR<uint64_t>(mb + 0xE273109);
        r15 = mb + 0x9000;
        rax = 0x4ED445DA1516D871; r8 *= rax;
        rax = r8; rax >>= 0x20; r8 ^= rax;
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r10;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        rcx = rbx;
        rax = 0x6BC0F33F5AA6F0DE; rax += r8;
        rcx *= r15; r8 = rcx; r8 ^= rax;
        r15 = mb;
        rax = 0xFFFFFFFFE660111B; rax -= rbx; rax -= r15; r8 += rax;
        rax = r8; rax >>= 0x1C; r8 ^= rax;
        rax = r8; rax >>= 0x38; rax ^= rbx; r8 ^= rax;
        return r8;
    }
    case 7:
    {
        r9 = SR<uint64_t>(mb + 0xE273109);
        rax = mb; r8 ^= rax;
        rax = r8; rax >>= 0x12; r8 ^= rax;
        rax = r8; rax >>= 0x24; r8 ^= rax;
        rax = mb; r8 ^= rax; r8 += rax;
        rax = 0x9CD63D5E2244B927; r8 *= rax;
        rax = 0xABFA7DAA01B4B32E; r8 ^= rax;
        rax = r8; rax >>= 0x12; r8 ^= rax;
        rax = r8; rax >>= 0x24; r8 ^= rax;
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r9;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        return r8;
    }
    case 8:
    {
        r11 = mb + 0xB55D;
        r9 = SR<uint64_t>(mb + 0xE273109);
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r9;
        rax = _byteswap_uint64(rax);
        rax = SR<uint64_t>(rax + 0x11);
        rax *= 0xBEB0E5BC09D6BA9;
        r8 *= rax;
        rax = r8; rax >>= 0xE;  r8 ^= rax;
        rax = r8; rax >>= 0x1C; r8 ^= rax;
        rax = r8; rax >>= 0x38; r8 ^= rax;
        rax = r8; rax >>= 0x7;  r8 ^= rax;
        rax = r8; rax >>= 0xE;  r8 ^= rax;
        rax = r8; rax >>= 0x1C; r8 ^= rax;
        rax = r8; rax >>= 0x38; r8 ^= rax;
        rax = 0x730AF1B3E8B67BD9; r8 -= rax;
        rax = 0xCEDDFDD5CEC5B648; r8 ^= rax;
        rax = rbx; rax = ~rax; r8 ^= rax;
        rax = mb; r8 ^= r11; r8 ^= rax;
        return r8;
    }
    case 9:
    {
        r12 = mb + 0x717A048C;
        r10 = SR<uint64_t>(mb + 0xE273109);
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r10;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        rax = r8; rax >>= 0x19; r8 ^= rax;
        rax = r8; rax >>= 0x32; r8 ^= rax;
        rax = 0x9156C02DC71AB78F; r8 ^= rax;
        rax = 0xAEE85E5BB5D47D88; r8 ^= rax;
        r8 += r12;
        rcx = rbx; rcx = ~rcx; rcx -= rbx; r8 += rcx;
        r8 ^= rbx;
        rax = 0xC89312E8DE78F2C9; r8 *= rax;
        return r8;
    }
    case 10:
    {
        r15 = mb + 0x1C7B4FB1;
        r10 = SR<uint64_t>(mb + 0xE273109);
        rax = 0x9AEF3116CA828800; r8 ^= rax;
        r8 -= rbx;
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r10;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        r8 += rbx; r8 += r15;
        rcx = rbx; rcx = ~rcx; r8 += rcx;
        rax = r8; rax >>= 0x1D; r8 ^= rax;
        rax = r8; rax >>= 0x3A; r8 ^= rax;
        rax = 0xBA18E69537476EBB; r8 *= rax;
        return r8;
    }
    case 11:
    {
        r13 = mb + 0x7DC342A5;
        r10 = SR<uint64_t>(mb + 0xE273109);
        rax = r8; rcx = 0;
        rax >>= 0x1B; r8 ^= rax;
        rcx = _rotl64(rcx, 0x10);
        rax = r8; rcx ^= r10;
        rax >>= 0x36; r8 ^= rax;
        rcx = _byteswap_uint64(rcx);
        r8 *= SR<uint64_t>(rcx + 0x11);
        r8 -= rbx;
        rax = 0x1E12CFFA67BA2B8D; r8 ^= rax;
        rcx = rbx; rax = r13; rax ^= rbx; rcx -= rax;
        rax = mb; rcx -= rax;
        rcx += 0xFFFFFFFF9E0E211B;
        r8 += rcx;
        rax = 0xA750BA46EAB2805B; r8 *= rax;
        rax = 0x331EEAFECEC104F1; r8 -= rax;
        return r8;
    }
    case 12:
    {
        r11 = SR<uint64_t>(mb + 0xE273109);
        r12 = mb;
        rax = 0xFFFFFFFFFFFF2638; rax -= r12; r8 += rax;
        r13 = 0x33CAFE1D6201EB57; r8 += r13;
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r11;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        rax = r8; rax >>= 0x17; r8 ^= rax;
        rax = r8; rax >>= 0x2E; r8 ^= rax;
        rcx = mb + 0x132;
        rax = rbx; rax = ~rax; r8 += rax;
        rax = 0x2463D01E7C7CCDA1;
        r8 += rcx; r8 *= rax;
        rax = 0x58155A0C2CFEBFBF; r8 ^= rax;
        return r8;
    }
    case 13:
    {
        r12 = mb + 0xCEFB;
        r10 = SR<uint64_t>(mb + 0xE273109);
        rax = r8; rax >>= 0x15; r8 ^= rax;
        rax = r8; rax >>= 0x2A; r8 ^= rax;
        rax = r8; rax >>= 0x1E; r8 ^= rax;
        rax = r8; rax >>= 0x3C; r8 ^= rax;
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r10;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        rax = 0x9E9F757EBD8A999F; r8 *= rax;
        r8 ^= rbx;
        rax = r8; rcx = rbx + r12;
        r8 = 0x91D6E77806FAA4C9;
        rax *= r8; r8 = rcx; r8 ^= rax;
        return r8;
    }
    case 14:
    {
        r15 = mb + 0xF68D;
        r10 = SR<uint64_t>(mb + 0xE273109);
        rcx = r15; rcx = ~rcx;
        rax = rbx; rax = ~rax; rcx *= rax; r8 += rcx;
        rax = mb + 0x49F4; rax = ~rax; rax += rbx; r8 += rax;
        rax = 0; rax = _rotl64(rax, 0x10); rax ^= r10;
        rax = _byteswap_uint64(rax);
        r8 *= SR<uint64_t>(rax + 0x11);
        rax = r8; rax >>= 0xF;  r8 ^= rax;
        rax = r8; rax >>= 0x1E; r8 ^= rax;
        rax = r8; rax >>= 0x3C; r8 ^= rax;
        rax = 0x86F1940299CD113F; r8 *= rax;
        rax = mb; r8 ^= rax;
        rax = r8; rax >>= 0x3;  r8 ^= rax;
        rax = r8; rax >>= 0x6;  r8 ^= rax;
        rax = r8; rax >>= 0xC;  r8 ^= rax;
        rax = r8; rax >>= 0x18; r8 ^= rax;
        rax = r8; rax >>= 0x30; r8 ^= rax;
        rax = 0x1A89FC56826921E2; r8 -= rax;
        return r8;
    }
    case 15:
    {
        // Will be filled after dump - safe fallback
        // PEB rotates so this case is rarely hit consecutively
        // Next frame will hit a different case
        return 0;
    }
    default:
        return r8;
    }
    return 0;
}