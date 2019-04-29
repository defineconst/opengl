#pragma once

class Event;
class Timer;
class Object;
class Render;
class ResCache;
class CfgCache;

class Global {
public:
    inline Event & RefEvent()
    {
        return *_event;
    }

    inline Timer & RefTimer()
    {
        return *_timer;
    }

    inline Object & RefObject()
    {
        return *_object;
    }

    inline Render & RefRender()
    {
        return *_render;
    }

    inline ResCache & RefResCache()
    {
        return *_resCache;
    }

    inline CfgCache & RefCfgCache()
    {
        return *_cfgCache;
    }

    Global();
    ~Global();

    void Start();
    void Clean();

    static Global & Ref()
    {
        static Global global;
        return global;
    }

private:
    Event * _event;
    Timer * _timer;
    Object * _object;
    Render * _render;
    ResCache * _resCache;
    CfgCache * _cfgCache;
};
