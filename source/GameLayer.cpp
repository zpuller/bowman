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

static CCLayerColor* lg;
static b2Body* body;
static float w = (float)s3eSurfaceGetInt(S3E_SURFACE_WIDTH);
static float h = (float)s3eSurfaceGetInt(S3E_SURFACE_HEIGHT);
static float winv = 1/w;
static float hinv = 1/h;

GameLayer::~GameLayer()
{
    delete g_pInput;
}

CCScene* GameLayer::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    GameLayer *layer = GameLayer::create();

    // Add layer as a child to scene
    scene->addChild(layer);

    lg = CCLayerColor::create(ccc4(255, 0, 0, 255));
    float posy = body->GetPosition().y;
    lg->setPosition(CCPoint(0, (posy-1)*.333*.5*h));
    lg->setContentSize(CCSize(.1*w, .1*h));
    lg->setColor(ccc3(0, 255, 0));
    scene->addChild(lg);

    // Return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool GameLayer::init()
{
    if (!CCLayer::init())
        return false;

    // Create main loop
    this->schedule(schedule_selector(GameLayer::update));

    // COCOS2D TIP
    // Create Cocos2D objects here

	// Create Box2D world
	world = new b2World(b2Vec2(0, 100));

    // BOX2D TIP
    // Create Box2D objects here
    b2Vec2 gravity(0.0f, -10.0f);
    world = new b2World(gravity);
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, -10.0f);
    b2Body* groundBody = world->CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(50.0f, 10.0f);
    groundBody->CreateFixture(&groundBox, 0.0f);
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(0.0f, 4.0f);
    body = world->CreateBody(&bodyDef);
    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(1.0f, 1.0f);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    body->CreateFixture(&fixtureDef);
    
    g_pInput = new Input();

    return true;
}

void GameLayer::draw()
{
}

void GameLayer::update(float dt)
{
    g_pInput->Update();
    
	// Update Box2D world
	world->Step(dt, 6, 3);
    
    // BOX2D TIP
    // Update objects from box2d coordinates here
    CCPoint b = lg->getPosition();
    CCSize s = lg->getContentSize();
    CCPoint e = CCPoint(b.x + s.width, b.y * s.height);
    
    bool grabbed = g_pInput->m_Touched && g_pInput->m_X > b.x && g_pInput->m_X < e.x && (h - g_pInput->m_Y) > b.y && (h - g_pInput->m_Y) < e.y;
    
    static float lastx, lasty, posx, posy;
    if (grabbed) {
        lg->setColor(ccc3(0, 255, 0));
        
        float pixelx = g_pInput->m_X - .5*s.width;
        float pixely = h-g_pInput->m_Y - .5*s.height;
        pixely = std::max(1.0f, pixely);

        posx = pixelx * winv * 40;
        posy = (pixely * 3 * 2 * hinv) + 1;
        
        lg->setPosition(CCPoint(pixelx, pixely));
        body->SetAwake(true);
        body->SetTransform(b2Vec2(posx, posy), 0);
        static const int scale = 10;
        body->SetLinearVelocity(b2Vec2(scale * (posx - lastx), scale * (posy - lasty)));
    }
    else {
        lg->setColor(ccc3(255, 0, 0));
        
        posx = body->GetPosition().x;
        posy = body->GetPosition().y;
        lg->setPosition(CCPoint(posx*w*.025, (posy-1)*.333*.5*h));
    }
    
    lastx = posx;
    lasty = posy;
}

