#include "global.h"
#include "event/event.h"
#include "timer/timer.h"
#include "object/object.h"
#include "render/render.h"
#include "res/res_cache.h"
#include "config/config_cache.h"

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
    , _assetCache(nullptr)
    , _configCache(nullptr)
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
        && _assetCache == nullptr
        && _configCache == nullptr);
    //  ��ʼ�� event
    _event = new Event();

    //  ��ʼ��timer
    _timer = new Timer();

    //  ��ʼ��object
    _object = new Object();

    //  ��ʼ��render
    _render = new Render();

    //  ��ʼ��assetCache
    _assetCache = new ResCache();

    //  ��ʼ��configCache
    _configCache = new ConfigCache();
    _configCache->Init(DIR_CONFIG_ROOT);
}

void Global::Clean()
{
    SafeDelete(_event);
    SafeDelete(_timer);
    SafeDelete(_object);
    SafeDelete(_render);
    SafeDelete(_assetCache);
    SafeDelete(_configCache);
}
