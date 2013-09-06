/*
 *
 */
#include "PerformanceAllocTest.h"

#include <algorithm>

// Enable profiles for this file
#undef CC_PROFILER_DISPLAY_TIMERS
#define CC_PROFILER_DISPLAY_TIMERS() Profiler::getInstance()->displayTimers()
#undef CC_PROFILER_PURGE_ALL
#define CC_PROFILER_PURGE_ALL() Profiler::getInstance()->releaseAllTimers()

#undef CC_PROFILER_START
#define CC_PROFILER_START(__name__) ProfilingBeginTimingBlock(__name__)
#undef CC_PROFILER_STOP
#define CC_PROFILER_STOP(__name__) ProfilingEndTimingBlock(__name__)
#undef CC_PROFILER_RESET
#define CC_PROFILER_RESET(__name__) ProfilingResetTimingBlock(__name__)

#undef CC_PROFILER_START_CATEGORY
#define CC_PROFILER_START_CATEGORY(__cat__, __name__) do{ if(__cat__) ProfilingBeginTimingBlock(__name__); } while(0)
#undef CC_PROFILER_STOP_CATEGORY
#define CC_PROFILER_STOP_CATEGORY(__cat__, __name__) do{ if(__cat__) ProfilingEndTimingBlock(__name__); } while(0)
#undef CC_PROFILER_RESET_CATEGORY
#define CC_PROFILER_RESET_CATEGORY(__cat__, __name__) do{ if(__cat__) ProfilingResetTimingBlock(__name__); } while(0)

#undef CC_PROFILER_START_INSTANCE
#define CC_PROFILER_START_INSTANCE(__id__, __name__) do{ ProfilingBeginTimingBlock( String::createWithFormat("%08X - %s", __id__, __name__)->getCString() ); } while(0)
#undef CC_PROFILER_STOP_INSTANCE
#define CC_PROFILER_STOP_INSTANCE(__id__, __name__) do{ ProfilingEndTimingBlock(    String::createWithFormat("%08X - %s", __id__, __name__)->getCString() ); } while(0)
#undef CC_PROFILER_RESET_INSTANCE
#define CC_PROFILER_RESET_INSTANCE(__id__, __name__) do{ ProfilingResetTimingBlock( String::createWithFormat("%08X - %s", __id__, __name__)->getCString() ); } while(0)

static std::function<PerformceAllocScene*()> createFunctions[] =
{
    CL(NodeCreateTest),
    CL(NodeDeallocTest),
    CL(SpriteCreateEmptyTest),
    CL(SpriteCreateTest),
    CL(SpriteDeallocTest),
};

#define MAX_LAYER    (sizeof(createFunctions) / sizeof(createFunctions[0]))

enum {
    kTagInfoLayer = 1,

    kTagBase = 20000,
};

enum {
    kMaxNodes = 15000,
    kNodesIncrease = 500,
};

static int g_curCase = 0;

////////////////////////////////////////////////////////
//
// AllocBasicLayer
//
////////////////////////////////////////////////////////

AllocBasicLayer::AllocBasicLayer(bool bControlMenuVisible, int nMaxCases, int nCurCase)
: PerformBasicLayer(bControlMenuVisible, nMaxCases, nCurCase)
{
}

void AllocBasicLayer::showCurrentTest()
{
    int nodes = ((PerformceAllocScene*)getParent())->getQuantityOfNodes();

    auto scene = createFunctions[_curCase]();

    g_curCase = _curCase;

    if (scene)
    {
        scene->initWithQuantityOfNodes(nodes);

        Director::getInstance()->replaceScene(scene);
        scene->release();
    }
}

////////////////////////////////////////////////////////
//
// PerformceAllocScene
//
////////////////////////////////////////////////////////
void PerformceAllocScene::initWithQuantityOfNodes(unsigned int nNodes)
{
    //srand(time());
    auto s = Director::getInstance()->getWinSize();

    // Title
    auto label = LabelTTF::create(title().c_str(), "Arial", 40);
    addChild(label, 1);
    label->setPosition(Point(s.width/2, s.height-32));
    label->setColor(Color3B(255,255,40));

    // Subtitle
    std::string strSubTitle = subtitle();
    if(strSubTitle.length())
    {
        auto l = LabelTTF::create(strSubTitle.c_str(), "Thonburi", 16);
        addChild(l, 1);
        l->setPosition(Point(s.width/2, s.height-80));
    }

    lastRenderedCount = 0;
    currentQuantityOfNodes = 0;
    quantityOfNodes = nNodes;

    MenuItemFont::setFontSize(65);
    auto decrease = MenuItemFont::create(" - ", [&](Object *sender) {
		quantityOfNodes -= kNodesIncrease;
		if( quantityOfNodes < 0 )
			quantityOfNodes = 0;

		updateQuantityLabel();
		updateQuantityOfNodes();
        updateProfilerName();
        CC_PROFILER_PURGE_ALL();
        srandom(0);
	});
    decrease->setColor(Color3B(0,200,20));
    auto increase = MenuItemFont::create(" + ", [&](Object *sender) {
		quantityOfNodes += kNodesIncrease;
		if( quantityOfNodes > kMaxNodes )
			quantityOfNodes = kMaxNodes;

		updateQuantityLabel();
		updateQuantityOfNodes();
        updateProfilerName();
        CC_PROFILER_PURGE_ALL();
        srandom(0);
	});
    increase->setColor(Color3B(0,200,20));

    auto menu = Menu::create(decrease, increase, NULL);
    menu->alignItemsHorizontally();
    menu->setPosition(Point(s.width/2, s.height/2+15));
    addChild(menu, 1);

    auto infoLabel = LabelTTF::create("0 nodes", "Marker Felt", 30);
    infoLabel->setColor(Color3B(0,200,20));
    infoLabel->setPosition(Point(s.width/2, s.height/2-15));
    addChild(infoLabel, 1, kTagInfoLayer);

    auto menuLayer = new AllocBasicLayer(true, MAX_LAYER, g_curCase);
    addChild(menuLayer);
    menuLayer->release();

    updateQuantityLabel();
    updateQuantityOfNodes();
    updateProfilerName();
    srandom(0);
}

std::string PerformceAllocScene::title()
{
    return "No title";
}

std::string PerformceAllocScene::subtitle()
{
    return "";
}

void PerformceAllocScene::updateQuantityLabel()
{
    if( quantityOfNodes != lastRenderedCount )
    {
        auto infoLabel = static_cast<LabelTTF*>( getChildByTag(kTagInfoLayer) );
        char str[20] = {0};
        sprintf(str, "%u nodes", quantityOfNodes);
        infoLabel->setString(str);

        lastRenderedCount = quantityOfNodes;
    }
}

const char * PerformceAllocScene::profilerName()
{
    return _profilerName;
}

void PerformceAllocScene::updateProfilerName()
{
    snprintf(_profilerName, sizeof(_profilerName)-1, "%s(%d)", testName(), quantityOfNodes);
}

void PerformceAllocScene::onExitTransitionDidStart()
{
    Scene::onExitTransitionDidStart();
    
    auto director = Director::getInstance();
    auto sched = director->getScheduler();

    sched->unscheduleSelector(SEL_SCHEDULE(&PerformceAllocScene::dumpProfilerInfo), this);
}

void PerformceAllocScene::onEnterTransitionDidFinish()
{
    Scene::onEnterTransitionDidFinish();

    auto director = Director::getInstance();
    auto sched = director->getScheduler();

    CC_PROFILER_PURGE_ALL();
    sched->scheduleSelector(SEL_SCHEDULE(&PerformceAllocScene::dumpProfilerInfo), this, 2, false);
}

void PerformceAllocScene::dumpProfilerInfo(float dt)
{
	CC_PROFILER_DISPLAY_TIMERS();
}

////////////////////////////////////////////////////////
//
// NodeCreateTest
//
////////////////////////////////////////////////////////
void NodeCreateTest::updateQuantityOfNodes()
{
    currentQuantityOfNodes = quantityOfNodes;
}

void NodeCreateTest::initWithQuantityOfNodes(unsigned int nNodes)
{
    PerformceAllocScene::initWithQuantityOfNodes(nNodes);

    printf("Size of Node: %lu\n", sizeof(Node));

    scheduleUpdate();
}

void NodeCreateTest::update(float dt)
{
    // iterate using fast enumeration protocol

    Node **nodes = new Node*[quantityOfNodes];

    CC_PROFILER_START(this->profilerName());
    for( int i=0; i<quantityOfNodes; ++i)
        nodes[i] = Node::create();
    CC_PROFILER_STOP(this->profilerName());

    delete [] nodes;
}

std::string NodeCreateTest::title()
{
    return "Node Create Perf test.";
}

std::string NodeCreateTest::subtitle()
{
    return "Node Create Perf test. See console";
}

const char*  NodeCreateTest::testName()
{
    return "Node::create()";
}

////////////////////////////////////////////////////////
//
// NodeDeallocTest
//
////////////////////////////////////////////////////////
void NodeDeallocTest::updateQuantityOfNodes()
{
    currentQuantityOfNodes = quantityOfNodes;
}

void NodeDeallocTest::initWithQuantityOfNodes(unsigned int nNodes)
{
    PerformceAllocScene::initWithQuantityOfNodes(nNodes);

    printf("Size of Sprite: %lu\n", sizeof(Node));

    scheduleUpdate();
}

void NodeDeallocTest::update(float dt)
{
    // iterate using fast enumeration protocol

    Node **nodes = new Node*[quantityOfNodes];

    for( int i=0; i<quantityOfNodes; ++i) {
        nodes[i] = new Node;
        nodes[i]->init();
    }

    CC_PROFILER_START(this->profilerName());
    for( int i=0; i<quantityOfNodes; ++i)
        nodes[i]->release();
    CC_PROFILER_STOP(this->profilerName());

    delete [] nodes;
}

std::string NodeDeallocTest::title()
{
    return "Node Dealloc Perf test.";
}

std::string NodeDeallocTest::subtitle()
{
    return "Node Dealloc Perf test. See console";
}

const char*  NodeDeallocTest::testName()
{
    return "Node::~Node()";
}

////////////////////////////////////////////////////////
//
// SpriteCreateEmptyTest
//
////////////////////////////////////////////////////////
void SpriteCreateEmptyTest::updateQuantityOfNodes()
{
    currentQuantityOfNodes = quantityOfNodes;
}

void SpriteCreateEmptyTest::initWithQuantityOfNodes(unsigned int nNodes)
{
    PerformceAllocScene::initWithQuantityOfNodes(nNodes);

    printf("Size of Node: %lu\n", sizeof(Sprite));

    scheduleUpdate();
}

void SpriteCreateEmptyTest::update(float dt)
{
    // iterate using fast enumeration protocol

    Sprite **sprites = new Sprite*[quantityOfNodes];

    Sprite::create("Images/grossini.png");

    CC_PROFILER_START(this->profilerName());
    for( int i=0; i<quantityOfNodes; ++i)
        sprites[i] = Sprite::create();
    CC_PROFILER_STOP(this->profilerName());

    delete [] sprites;
}

std::string SpriteCreateEmptyTest::title()
{
    return "Create Empty Sprite";
}

std::string SpriteCreateEmptyTest::subtitle()
{
    return "Create Empty Sprite Perf test. See console";
}

const char*  SpriteCreateEmptyTest::testName()
{
    return "Sprite::create(void)";
}

////////////////////////////////////////////////////////
//
// SpriteCreateTest
//
////////////////////////////////////////////////////////
void SpriteCreateTest::updateQuantityOfNodes()
{
    currentQuantityOfNodes = quantityOfNodes;
}

void SpriteCreateTest::initWithQuantityOfNodes(unsigned int nNodes)
{
    PerformceAllocScene::initWithQuantityOfNodes(nNodes);

    printf("Size of Node: %lu\n", sizeof(Sprite));

    scheduleUpdate();
}

void SpriteCreateTest::update(float dt)
{
    // iterate using fast enumeration protocol

    Sprite **sprites = new Sprite*[quantityOfNodes];

    Sprite::create("Images/grossini.png");
    
    CC_PROFILER_START(this->profilerName());
    for( int i=0; i<quantityOfNodes; ++i)
        sprites[i] = Sprite::create("Images/grossini.png");
    CC_PROFILER_STOP(this->profilerName());

    delete [] sprites;
}

std::string SpriteCreateTest::title()
{
    return "Create Sprite.";
}

std::string SpriteCreateTest::subtitle()
{
    return "Create Empty Sprite. See console";
}

const char*  SpriteCreateTest::testName()
{
    return "Sprite::create(\"image\")";
}

////////////////////////////////////////////////////////
//
// SpriteDeallocTest
//
////////////////////////////////////////////////////////
void SpriteDeallocTest::updateQuantityOfNodes()
{
    currentQuantityOfNodes = quantityOfNodes;
}

void SpriteDeallocTest::initWithQuantityOfNodes(unsigned int nNodes)
{
    PerformceAllocScene::initWithQuantityOfNodes(nNodes);

    printf("Size of Node: %lu\n", sizeof(Sprite));

    scheduleUpdate();
}

void SpriteDeallocTest::update(float dt)
{
    // iterate using fast enumeration protocol

    Sprite **sprites = new Sprite*[quantityOfNodes];

    for( int i=0; i<quantityOfNodes; ++i) {
        sprites[i] = new Sprite;
        sprites[i]->init();
    }

    CC_PROFILER_START(this->profilerName());
    for( int i=0; i<quantityOfNodes; ++i)
        sprites[i]->release();
    CC_PROFILER_STOP(this->profilerName());

    delete [] sprites;
}

std::string SpriteDeallocTest::title()
{
    return "Sprite Dealloc Perf test.";
}

std::string SpriteDeallocTest::subtitle()
{
    return "Sprite Dealloc Perf test. See console";
}

const char*  SpriteDeallocTest::testName()
{
    return "Sprite::~Sprite()";
}

///----------------------------------------
void runAllocPerformanceTest()
{
    auto scene = createFunctions[g_curCase]();
    scene->initWithQuantityOfNodes(kNodesIncrease);
    
    Director::getInstance()->replaceScene(scene);
    scene->release();
}
