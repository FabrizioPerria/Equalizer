#pragma once

#include <JuceHeader.h>

template <typename FloatType>
class Decibel
{
public:
    Decibel() : linearValue (FloatType (1))
    {
    }

    Decibel (FloatType db) : linearValue (juce::Decibels::decibelsToGain (db, MINUS_INF))
    {
    }

    Decibel<FloatType> operator+= (Decibel<FloatType> rhs)
    {
        this->linearValue += rhs.linearValue;
        return *this;
    }

    Decibel operator-= (Decibel<FloatType> rhs)
    {
        this->linearValue -= rhs.linearValue;
        return *this;
    }

    Decibel<FloatType> operator*= (Decibel<FloatType> rhs)
    {
        this->linearValue *= rhs.linearValue;
        return *this;
    }

    Decibel<FloatType> operator/= (Decibel<FloatType> rhs)
    {
        auto rhsGain = rhs.linearValue;
        if (juce::approximatelyEqual (rhsGain, 0.0f))
        {
            this->linearValue = std::numeric_limits<FloatType>::max();
        }
        else
        {
            this->linearValue /= rhsGain;
        }
        return *this;
    }

    FloatType getGain() const
    {
        return linearValue;
    }

    FloatType getDb() const
    {
        return juce::Decibels::gainToDecibels (linearValue, MINUS_INF);
    }

    void setGain (FloatType g)
    {
        linearValue = g;
    }

    void setDb (FloatType db)
    {
        linearValue = juce::Decibels::decibelsToGain (db);
    }

    friend Decibel<FloatType> operator+ (Decibel<FloatType> lhs, Decibel<FloatType> rhs)
    {
        Decibel<FloatType> result;
        result.setGain (lhs.linearValue + rhs.linearValue);
        return result;
    }

    friend Decibel<FloatType> operator- (Decibel<FloatType> lhs, Decibel<FloatType> rhs)
    {
        Decibel<FloatType> result;
        result.setGain (lhs.linearValue - rhs.linearValue);
        return result;
    }

    friend Decibel<FloatType> operator* (Decibel<FloatType> lhs, Decibel<FloatType> rhs)
    {
        Decibel<FloatType> result;
        result.setGain (lhs.linearValue * rhs.linearValue);
        return result;
    }

    friend Decibel<FloatType> operator/ (Decibel<FloatType> lhs, Decibel<FloatType> rhs)
    {
        Decibel<FloatType> result;
        if (juce::approximatelyEqual (rhs.linearValue, 0.0f))
        {
            result.setGain (std::numeric_limits<FloatType>::max());
        }
        else
        {
            result.setGain (lhs.linearValue / rhs.linearValue);
        }
        return result;
    }

    friend bool operator== (Decibel<FloatType> lhs, Decibel<FloatType> rhs)
    {
        return juce::approximatelyEqual (lhs.linearValue, rhs.linearValue);
    }

    friend bool operator!= (Decibel<FloatType> lhs, Decibel<FloatType> rhs)
    {
        return ! juce::approximatelyEqual (lhs.linearValue, rhs.linearValue);
    }

    friend bool operator< (Decibel<FloatType> lhs, Decibel<FloatType> rhs)
    {
        return lhs.linearValue < rhs.linearValue;
    }

    friend bool operator> (Decibel<FloatType> lhs, Decibel<FloatType> rhs)
    {
        return lhs.linearValue > rhs.linearValue;
    }

    friend bool operator<= (Decibel<FloatType> lhs, Decibel<FloatType> rhs)
    {
        return lhs.linearValue <= rhs.linearValue;
    }

    friend bool operator>= (Decibel<FloatType> lhs, Decibel<FloatType> rhs)
    {
        return lhs.linearValue >= rhs.linearValue;
    }

private:
    FloatType linearValue;

    static constexpr FloatType MINUS_INF = -48.0f;
};
