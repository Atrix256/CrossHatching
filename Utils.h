#pragma once

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