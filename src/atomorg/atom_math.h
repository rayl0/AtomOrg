#pragma once

struct m3
{
    union {
        struct
        {
            r32 a00, a01, a02;
            r32 a10, a11, a12;
            r32 a20, a21, a22;
        };
        r32 Data[9];
    };

    r32 operator[](s32 i) {
        return Data[i];
    }
};

inline m3
M3(r32 r) {
    m3 n;

    n.a00 = r;
    n.a11 = r;
    n.a22 = r;

    n.a01 = 0;
    n.a02 = 0;

    n.a10 = 0;
    n.a12 = 0;

    n.a20 = 0;
    n.a21 = 0;

    return n;
}

inline m3
operator*(m3& f, m3& s)
{
    m3 Final = M3(0);

    Final.a00 = f.a00 * s.a00 + f.a01 * s.a10 + f.a02 * s.a20;
    Final.a01 = f.a00 * s.a01 + f.a01 * s.a11 + f.a02 * s.a21;
    Final.a02 = f.a00 * s.a02 + f.a01 * s.a12 + f.a02 * s.a22;

    Final.a10 = f.a10 * s.a00 + f.a11 * s.a10 + f.a12 * s.a20;
    Final.a11 = f.a10 * s.a01 + f.a11 * s.a11 + f.a12 * s.a21;
    Final.a12 = f.a10 * s.a02 + f.a11 * s.a12 + f.a12 * s.a22;

    Final.a20 = f.a20 * s.a00 + f.a21 * s.a10 + f.a22 * s.a20;
    Final.a21 = f.a20 * s.a01 + f.a21 * s.a11 + f.a22 * s.a21;
    Final.a22 = f.a20 * s.a02 + f.a21 * s.a12 + f.a22 * s.a22;

    return Final;
}

inline m3
m3Ortho(r32 Left, r32 Right, r32 Top, r32 Bottom)
{
    m3 Proj = M3(1.0f);

    Proj.a00 = 2 / (Right - Left);
    Proj.a20 = -(Right + Left) / (Right - Left);

    Proj.a11 = 2 / (Bottom - Top);
    Proj.a21 = -(Bottom + Top) / (Bottom - Top);

    Proj.a22 = 1;

    return Proj;
}

struct v2
{
    union
    {
        struct
        {
            r32 x, y;
        };
        r32 Data[2];
    };
};

inline v2
V2(r32 x, r32 y) {
    v2 n;

    n.x = x;
    n.y = y;

    return n;
}

struct c3
{
    union {
        struct {
            r32 r, g, b;
        };
        r32 Data[3];
    };
};

inline c3
C3(u32 r, u32 g, u32 b)
{
    c3 n;

    n.r = r / 256.0f;
    n.g = g / 256.0f;
    n.b = b / 256.0f;

    return n;
}

inline c3
C3(r32 r, r32 g, r32 b)
{
    c3 n;

    n.r = r;
    n.g = g;
    n.b = b;

    return n;
}

struct v4
{
    union
    {
        struct
        {
            v2 min;
            v2 max;
        };
        struct
        {
            r32 x, y, z, w;
        };
        r32 Data[4];
    };
};

inline v4
V4(r32 x, r32 y, r32 z, r32 w)
{
    v4 n;

    n.x = x;
    n.y = y;
    n.z = z;
    n.w = w;

    return n;
}

inline
m3 m3Translate(m3& m, v2 v)
{
    m3 Translation = M3(1.0f);

    Translation.a20 = v.x;
    Translation.a21 = v.y;

    return Translation * m;
}

inline m3
m3Scale(m3& m, v2 v)
{
    m3 Scale = M3(1.0f);

    Scale.a00 = v.x;
    Scale.a11 = v.y;

    return Scale * m;
}
