/*
 * (C) 2001-2012 Marmalade. All Rights Reserved.
 *
 * This document is protected by copyright, and contains information
 * proprietary to Marmalade.
 *
 * This file consists of source code released by Marmalade under
 * the terms of the accompanying End User License Agreement (EULA).
 * Please do not use this program/source code before you have read the
 * EULA and have agreed to be bound by its terms.
 */
#include "GameLayer.h"
#include "input.h"
#include <iostream>
#include <algorithm>

static CCLayerColor* boxDrawable1;
static CCLayerColor* boxDrawable2;
static b2Body* body1;
static b2Body* body2;
static float screenWidth = (float)s3eSurfaceGetInt(S3E_SURFACE_WIDTH);
static float screenHeight = (float)s3eSurfaceGetInt(S3E_SURFACE_HEIGHT);
static float screenWidthInv = 1/screenWidth;
static float screenHeightInv = 1/screenHeight;
static float coordWidth = 40;
static float coordHeight = coordWidth * screenHeight / screenWidth;
static float coordWidthInv = 1/coordWidth;
static float coordHeightInv = 1/coordHeight;
static float boxR = 2.0f;
static b2DistanceJoint* j;

GameLayer::~GameLayer()
{
    delete g_pInput;
    delete world;
}

CCScene* GameLayer::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    GameLayer *layer = GameLayer::create();

    // Add layer as a child to scene
    scene->addChild(layer);

    // Return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool GameLayer::init()
{
    if (!CCLayer::init())
        return false;
    
    g_pInput = new Input();
    
    // Create main loop
    this->schedule(schedule_selector(GameLayer::update));

	// Create Box2D world
    b2Vec2 gravity(0.0f, -coordHeight);
	world = new b2World(gravity);

    // Create Box2D objects
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, -10.0f);
    b2Body* groundBody = world->CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(50.0f, 10.0f);
    groundBody->CreateFixture(&groundBox, 0.0f);
    
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(2.0f, 40.0f);
    body1 = world->CreateBody(&bodyDef);
    bodyDef.type = b2_staticBody;
    bodyDef.position.Set(12.0f, 40.0f);
    body2 = world->CreateBody(&bodyDef);

    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(2.0f, 2.0f);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.filter.groupIndex = -1;
    body1->CreateFixture(&fixtureDef);
    body2->CreateFixture(&fixtureDef);
    
    b2DistanceJointDef jDef;
    jDef.Initialize(body1, body2, body1->GetPosition(), body2->GetPosition());
    j = (b2DistanceJoint*)world->CreateJoint(&jDef);
    j->SetFrequency(1);
    
    // Create Cocos2D objects
    boxDrawable1 = CCLayerColor::create(ccc4(255, 0, 0, 255));
    float posY = body1->GetPosition().x;
    float posy = body1->GetPosition().y;
    boxDrawable1->setPosition(CCPoint((posY-boxR)*screenWidth*coordWidthInv, (posy-boxR)*coordHeightInv*screenHeight));
    boxDrawable1->setContentSize(CCSize(2*boxR*coordWidthInv*screenWidth, 2*boxR*coordHeightInv*screenHeight));
    boxDrawable1->setColor(ccc3(0, 255, 0));
    addChild(boxDrawable1);
    
    boxDrawable2 = CCLayerColor::create(ccc4(255, 0, 0, 255));
    float posx2 = body2->GetPosition().x;
    float posy2 = body2->GetPosition().y;
    boxDrawable2->setPosition(CCPoint((posx2-boxR)*screenWidth*coordWidthInv, (posy2-boxR)*coordHeightInv*screenHeight));
    boxDrawable2->setContentSize(CCSize(2*boxR*coordWidthInv*screenWidth, 2*boxR*coordHeightInv*screenHeight));
    boxDrawable2->setColor(ccc3(0, 255, 0));
    addChild(boxDrawable2);
    
    return true;
}

void GameLayer::draw()
{
}

void GameLayer::update(float dt)
{
    if (j)
        std::cout << j->GetReactionForce(1/dt).y << std::endl;
    if (j && j->GetReactionForce(1/dt).y > 0) {
        world->DestroyJoint(j);
        j = NULL;
    }

    g_pInput->Update();
    
	// Update Box2D world
	world->Step(dt, 6, 3);
    
    // BOX2D TIP
    // Update objects from box2d coordinates here
    CCPoint b = boxDrawable1->getPosition();
    CCSize s = boxDrawable1->getContentSize();
    CCPoint e = CCPoint(b.x + s.width, b.y + s.height);
    
    bool grabbed = g_pInput->m_Touched && g_pInput->m_X > b.x && g_pInput->m_X < e.x && (screenHeight - g_pInput->m_Y) > b.y && (screenHeight - g_pInput->m_Y) < e.y;
    
    static float lastX, lastY, posY, posy;
    if (grabbed) {
        boxDrawable1->setColor(ccc3(0, 255, 0));
        
        float pixelx = g_pInput->m_X - .5*s.width;
        float pixely = screenHeight-g_pInput->m_Y - .5*s.height;

        posY = (pixelx * screenWidthInv * coordWidth) + 2;
        posy = (pixely * coordHeight * screenHeightInv) + 2;
        
        boxDrawable1->setPosition(CCPoint(pixelx, pixely));
        body1->SetAwake(true);
        body1->SetTransform(b2Vec2(posY, posy), 0);
        static const int vScale = 10;
        body1->SetLinearVelocity(b2Vec2(vScale * (posY - lastX), vScale * (posy - lastY)));
    }
    else {
        boxDrawable1->setColor(ccc3(255, 0, 0));
        
        posY = body1->GetPosition().x;
        posy = body1->GetPosition().y;
        boxDrawable1->setPosition(CCPoint((posY-2)*screenWidth*coordWidthInv, (posy-2)*coordHeightInv*screenHeight));
    }
    
    lastX = posY;
    lastY = posy;
    
    float posx2 = body2->GetPosition().x;
    float posy2 = body2->GetPosition().y;
    boxDrawable2->setPosition(CCPoint((posx2-2)*screenWidth*coordWidthInv, (posy2-2)*coordHeightInv*screenHeight));
}