#include "global.h"
#include "event/event.h"
#include "timer/timer.h"
#include "object/object.h"
#include "render/render.h"
#include "res/res_cache.h"
#include "cfg/cfg_cache.h"

//  ���ø�Ŀ¼
constexpr auto DIR_CONFIG_ROOT = "res/config";

template <class T>
void SafeDelete(T & ptr)
{
    delete ptr;
    ptr = nullptr;
}

Global::Global()
    : _event(nullptr)
    , _timer(nullptr)
    , _object(nullptr)
    , _render(nullptr)
    , _resCache(nullptr)
    , _cfgCache(nullptr)
{ }

Global::~Global()
{
    Clean();
}

void Global::Start()
{
    assert(_event == nullptr 
        && _timer == nullptr 
        && _object == nullptr 
        && _render == nullptr 
        && _resCache == nullptr
        && _cfgCache == nullptr);
    //  ��ʼ�� event
    _event = new Event();

    //  ��ʼ��timer
    _timer = new Timer();

    //  ��ʼ��object
    _object = new Object();

    //  ��ʼ��render
    _render = new Render();

    //  ��ʼ��assetCache
    _resCache = new ResCache();

    //  ��ʼ��configCache
    _cfgCache = new CfgCache();
    _cfgCache->Init(DIR_CONFIG_ROOT);
}

void Global::Clean()
{
    SafeDelete(_event);
    SafeDelete(_timer);
    SafeDelete(_object);
    SafeDelete(_render);
    SafeDelete(_resCache);
    SafeDelete(_cfgCache);
}
