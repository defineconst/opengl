#include "global.h"
#include "event/event.h"
#include "timer/timer.h"
#include "object/object.h"
#include "render/render.h"
#include "res/res_manager.h"
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
    ASSERT_LOG(_event == nullptr, "_event Error");
    ASSERT_LOG(_timer == nullptr, "_timer Error");
    ASSERT_LOG(_object == nullptr, "_object Error");
    ASSERT_LOG(_render == nullptr, "_render Error");
    ASSERT_LOG(_resCache == nullptr, "_resCache Error");
    ASSERT_LOG(_cfgCache == nullptr, "_cfgCache Error");

    //  ��ʼ�� event
    _event = new Event();

    //  ��ʼ��timer
    _timer = new Timer();

    //  ��ʼ��object
    _object = new Object();

    //  ��ʼ��render
    _render = new Render();

    //  ��ʼ��assetCache
    _resCache = new ResManager();

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
