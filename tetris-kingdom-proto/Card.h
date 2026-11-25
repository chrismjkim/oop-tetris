#pragma once

class Card {
public:
    enum class Kind {
        Filler, Bonus, Coolant, Slam, Insurance, Tag, CCTV, StrongThief, SoftGiant,
        Shrink, Calm, Metronome, Steady, Softener, Repel, Encore, SmallOnly, Finale, Count
    };
    Card(Kind kind = Kind::Filler, int shape = -1);
    Kind getKind() const;
    int getShape() const;
    bool repeatable() const;

private:
    Kind kind;
    int shape;
};
