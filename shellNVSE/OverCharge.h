#pragma once
#include "MainHeader.h"

std::unordered_map<Actor*, WeaponHeat*> WeaponHeater;

struct RGB
{
    float r;
    float g;
    float b;
};

struct RGB
{
    RGB blue = { 0, 0, 255 };
    RGB green = { 0, 255, 0};
    RGB red = { 255, 0, 0};
};

struct WeaponHeat
{

    float timer;            //Time since last shot
    float timeBuffer;

    float startingheat;

    RGB currentColor; 


    float heat;

    void TransitionColor(RGB nextColor)
    {

    }
};