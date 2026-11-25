#include "CardInfo.h"

CardInfo::CardInfo(int stage, bool shapeSpecific, const wchar_t* name, const wchar_t* line1, const wchar_t* line2)
    : stage(stage), shapeSpecific(shapeSpecific), name(name), line1(line1), line2(line2)
{
}

int CardInfo::getStage() const
{
    return stage;
}

bool CardInfo::getShapeSpecific() const
{
    return shapeSpecific;
}

const wchar_t* CardInfo::getName() const
{
    return name;
}

const wchar_t* CardInfo::getLine1() const
{
    return line1;
}

const wchar_t* CardInfo::getLine2() const
{
    return line2;
}
