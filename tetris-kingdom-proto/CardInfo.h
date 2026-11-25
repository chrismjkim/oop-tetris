#pragma once

class CardInfo {
public:
    CardInfo(int stage, bool shapeSpecific, const wchar_t* name, const wchar_t* line1, const wchar_t* line2);
    int getStage() const;
    bool getShapeSpecific() const;
    const wchar_t* getName() const;
    const wchar_t* getLine1() const;
    const wchar_t* getLine2() const;

private:
    int stage;
    bool shapeSpecific;
    const wchar_t* name;
    const wchar_t* line1;
    const wchar_t* line2;
};
