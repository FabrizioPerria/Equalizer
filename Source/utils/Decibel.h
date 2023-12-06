#pragma once

#include <JuceHeader.h>

template <typename FloatType>
class Decibel
{
public:
    Decibel() : dbValue (FloatType (0))
    {
    }

    Decibel (FloatType db) : dbValue (db)
    {
    }

    Decibel& operator+= (Decibel rhs)
    {
        this->dbValue += rhs.dbValue;
        return *this;
    }

    Decibel& operator-= (Decibel rhs)
    {
        this->dbValue -= rhs.dbValue;
        return *this;
    }

    Decibel& operator*= (Decibel rhs)
    {
        this->dbValue *= rhs.dbValue;
        return *this;
    }

    Decibel& operator/= (Decibel rhs)
    {
        if (! juce::approximatelyEqual (rhs.dbValue, 0.0f))
        {
            this->dbValue /= rhs.dbValue;
        }
        return *this;
    }

    FloatType getGain() const
    {
        return juce::Decibels::decibelsToGain (dbValue, MINUS_INF);
    }

    FloatType getDb() const
    {
        return dbValue;
    }

    void setGain (FloatType g)
    {
        dbValue = juce::Decibels::gainToDecibels (g, MINUS_INF);
    }

    void setDb (FloatType db)
    {
        dbValue = db;
    }

    friend Decibel operator+ (Decibel lhs, Decibel rhs)
    {
        return Decibel (lhs.dbValue + rhs.dbValue);
    }

    friend Decibel operator- (Decibel lhs, Decibel rhs)
    {
        return Decibel (lhs.dbValue - rhs.dbValue);
    }

    friend Decibel operator* (Decibel lhs, Decibel rhs)
    {
        return Decibel (lhs.dbValue * rhs.dbValue);
    }

    friend Decibel operator/ (Decibel lhs, Decibel rhs)
    {
        if (! juce::approximatelyEqual (rhs.dbValue, 0.0f))
        {
            return Decibel (lhs.dbValue / rhs.dbValue);
        }
        return lhs;
    }

    friend bool operator== (Decibel lhs, Decibel rhs)
    {
        return juce::approximatelyEqual (lhs.dbValue, rhs.dbValue);
    }

    friend bool operator!= (Decibel lhs, Decibel rhs)
    {
        return ! juce::approximatelyEqual (lhs.dbValue, rhs.dbValue);
    }

    friend bool operator< (Decibel lhs, Decibel rhs)
    {
        return lhs.dbValue < rhs.dbValue;
    }

    friend bool operator> (Decibel lhs, Decibel rhs)
    {
        return lhs.dbValue > rhs.dbValue;
    }

    friend bool operator<= (Decibel lhs, Decibel rhs)
    {
        return lhs.dbValue <= rhs.dbValue;
    }

    friend bool operator>= (Decibel lhs, Decibel rhs)
    {
        return lhs.dbValue >= rhs.dbValue;
    }

private:
    FloatType dbValue;

    static constexpr FloatType MINUS_INF = -48.0f;
};
