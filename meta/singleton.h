#pragma once

template <class T>
class singleton
{
public:
    singleton& operator=(const singleton&) = delete;
    singleton& operator=(singleton&&) = delete;

    static T& get_instance()
    {
        if (!instance)
            instance = new _T_inst;
        return *instance;
    }

protected:
    singleton() {}

private:
    struct _T_inst : public T
    {
        _T_inst() : T() {}
    };

    static inline T* instance = nullptr;
};
