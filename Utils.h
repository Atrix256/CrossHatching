#pragma once

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

    T* m_ptr;
};

inline float DegreesToRadians (float degrees)
{
    return degrees * c_pi / 180.0f;
}