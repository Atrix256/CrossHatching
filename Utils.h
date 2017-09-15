#pragma once

#include <stdio.h>
#include <windows.h>

static const float c_pi = 3.14159265359f;

template <typename T>
struct CAutoReleasePointer
{
    CAutoReleasePointer (T* ptr = nullptr)
    {
        m_ptr = ptr;
    }

    ~CAutoReleasePointer ()
    {
        Clear();
    }

    void Clear ()
    {
        if (m_ptr)
        {
            m_ptr->Release();
            m_ptr = nullptr;
        }
    }

    void SetDebugName (const char* name)
    {
        if (name)
            m_ptr->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
    }

    T* m_ptr;
};

inline float DegreesToRadians (float degrees)
{
    return degrees * c_pi / 180.0f;
}

inline void ReportError(const char* message)
{
    OutputDebugStringA(message);
    fprintf(stderr, message);
}