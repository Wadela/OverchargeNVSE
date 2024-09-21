#pragma once
#include "MainHeader.h"

//std::unordered_map<Actor*, HeatChange*> WeaponHeater;

struct HeatRGB
{
    //HeatRGB values will be passed to emmisivergb.heatColor
    float heatRed;
    float heatGreen;
    float heatBlue;

    HeatRGB(float red, float green, float blue) : 
        
        heatRed(red), heatGreen(green), heatBlue(blue) 
    
    {}

    //Transitions between two colors 
    static HeatRGB InterpolateColor(const HeatRGB& start, const HeatRGB& end, float interpolFactor)
    {

        interpolFactor = std::clamp(interpolFactor, 0.000f, 1.000f);

        return HeatRGB(

            start.heatRed + interpolFactor * (end.heatRed - start.heatRed),
            start.heatGreen + interpolFactor * (end.heatGreen - start.heatGreen),
            start.heatBlue + interpolFactor * (end.heatBlue - start.heatBlue)

        );

    }

    void ColorShift(float increment)
    {
        heatRed = std::clamp(heatRed + increment, 0.0f, 1.0f);
        heatGreen = std::clamp(heatGreen + increment, 0.0f, 1.0f);
        heatBlue = std::clamp(heatBlue + increment, 0.0f, 1.0f);
    }

};

struct HeatLevel
{

    float heatVal;

    HeatLevel(float numHeat) :

        heatVal(numHeat)

    {}

};

struct ColorTiers
{
    HeatRGB redTier;       
    HeatRGB orangeTier;    
    HeatRGB yellowTier;     
    HeatRGB greenTier;     
    HeatRGB blueTier;      
    HeatRGB whiteTier;      

    ColorTiers() : 
        
        //RGB Values normalized to 1.000
        redTier(0.000f, 0.000f, 0.000f),        
        orangeTier(0.000f, 0.000f, 0.000f),
        yellowTier(0.000f, 0.000f, 0.000f),
        greenTier(0.000f, 0.000f, 0.000f),
        blueTier(0.000f, 0.000f, 0.000f),
        whiteTier(0.000f, 0.000f, 0.000f)
    
    {}

};

struct PlasmaColor : ColorTiers 
{
    HeatRGB DefaultPlasma;

    PlasmaColor() :

        ColorTiers(),
        DefaultPlasma(0.000f, 0.000f, 0.000f)

    {
        //RGB Values normalized to 1.000
        redTier = HeatRGB(1.000f, 0.486f, 0.655f);          //Color: #ff7ca7
        orangeTier = HeatRGB(1.000f, 0.698f, 0.486f);       //Color: #ffb27c
        yellowTier = HeatRGB(1.000f, 1.000f, 0.486f);       //Color: #ffff7c
        greenTier = HeatRGB(0.655f, 1.000f, 0.486f);        //Color: #a7ff7c
        blueTier = HeatRGB(0.486f, 0.780f, 1.000f);         //Color: #7cc7ff
        whiteTier = HeatRGB(0.878f, 0.969f, 1.000f);        //Color: #e0f7ff

        DefaultPlasma = greenTier;
    }
    

};

struct LaserColor : ColorTiers
{

    HeatRGB DefaultLaser;
    HeatRGB DefaultLaserUnique;
    HeatRGB DefaultLAER;
    HeatRGB DefaultRecharger;

    LaserColor() :

        ColorTiers(),
        DefaultLaser(0.000f, 0.000f, 0.000f),
        DefaultLaserUnique(0.000f, 0.000f, 0.000f),
        DefaultLAER(0.000f, 0.000f, 0.000f),
        DefaultRecharger(0.000f, 0.000f, 0.000f)

    {
        //RGB Values normalized to 1.000
        redTier = HeatRGB(1.000f, 0.235f, 0.235f);          //Color: #ff3c3c
        orangeTier = HeatRGB(1.000f, 0.620f, 0.235f);       //Color: #ff9e3c
        yellowTier = HeatRGB(1.000f, 1.000f, 0.235f);       //Color: #ffff3c
        greenTier = HeatRGB(0.192f, 0.965f, 0.325f);        //Color: #31f752
        blueTier = HeatRGB(0.227f, 0.667f, 0.965f);         //Color: #3aaaf7 
        whiteTier = HeatRGB(0.878f, 0.969f, 1.000f);        //Color: #e0f7ff 
        
        DefaultLaser = redTier;
        DefaultLaserUnique = greenTier; 
        DefaultLAER = blueTier;
        DefaultRecharger = orangeTier;
    }


};

struct FlameColor : ColorTiers
{

    HeatRGB DefaultFlame;
    HeatRGB DefaultFlameUnique;

    FlameColor() :

        ColorTiers(),
        DefaultFlame(0.000f, 0.000f, 0.000f),
        DefaultFlameUnique(0.000f, 0.000f, 0.000f)

    {
        //RGB Values normalized to 1.000
        redTier = HeatRGB(1.000f, 0.341f, 0.133f);          //Color: #ff5722
        orangeTier = HeatRGB(1.000f, 0.549f, 0.000f);       //Color: #ff8c00 
        yellowTier = HeatRGB(1.000f, 0.925f, 0.235f);       //Color: #ffeb3b
        blueTier = HeatRGB(0.000f, 0.749f, 1.000f);         //Color: #00bfff

        DefaultFlame = redTier;
        DefaultFlameUnique = blueTier;
    }


};

struct ZapColor : ColorTiers
{

    HeatRGB VioletTier; 
    HeatRGB DefaultZap;
    HeatRGB DefaultZapUnique; 
    HeatRGB DefaultAlien; 

    ZapColor() :

        ColorTiers(),
        VioletTier(0.415f, 0.353f, 0.804f),                 //Color: #6A5ACD
        DefaultZap(0.000f, 0.000f, 0.000f),
        DefaultZapUnique(0.000f, 0.000f, 0.000f),
        DefaultAlien(0.000f, 0.000f, 0.000f)

    {
        //RGB Values normalized to 1.000
        redTier = HeatRGB(1.000f, 0.235f, 0.235f);          //Color: #ff3c3c
        orangeTier = HeatRGB(1.000f, 0.620f, 0.235f);       //Color: #ff9e3c
        blueTier = HeatRGB(0.227f, 0.667f, 0.965f);         //Color: #3aaaf7 
        whiteTier = HeatRGB(0.878f, 0.969f, 1.000f);        //Color: #e0f7ff 

        DefaultZap = blueTier;
        DefaultZapUnique = orangeTier;
        DefaultAlien = whiteTier;

    }

};

struct HeatTiers
{
    HeatLevel level1;
    HeatLevel level2;
    HeatLevel level3;
    HeatLevel level4;
    HeatLevel level5;
    HeatLevel level6;
    HeatLevel maxLevel;

    HeatTiers() :

        level1(0.0f),         //heatVal = 0-24
        level2(25.0f),        //heatVal = 25-49
        level3(50.0f),        //heatVal = 50-74
        level4(75.0f),        //heatVal = 75-99
        level5(100.0f),       //heatVal = 100-124
        level6(125.0f),       //heatVal = 125-149
        maxLevel(150.0f)      //Overheat Threshold

    {}
};

//struct HeatChange : HeatTiers
//{
//
//    float timer;                //Time since last shot
//    float timeBuffer;           //Amount of time before heat decays
//
//    HeatLevel currentHeat; 
//    HeatLevel defaultHeat;
//
//    HeatChange() :
//};