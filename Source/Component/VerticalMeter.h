#pragma once

#include <JuceHeader.h>

namespace Gui
{
    using namespace juce;

    class VerticalMeter : public Component
    {
    public:
        void paint(Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();

            //g.setColour(Colours::darkgrey);
            //g.fillRoundedRectangle(bounds, 1.f);

            g.setColour(Colours::white);
            //map level from -60.f - +6.f to 1 - height
            const auto height = jmap(level, -60.f, +6.f, 0.f, static_cast<float>(getHeight()));
            g.fillRoundedRectangle(bounds.removeFromBottom(height), 1.f);
        }
        void setLevel(const float value) { level = value; }
    private:
        float level = -60.f;

    };
};