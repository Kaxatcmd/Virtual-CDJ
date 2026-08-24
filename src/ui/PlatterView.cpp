#include "PlatterView.h"

namespace vcdj::ui
{
    PlatterView::PlatterView()
    {
        setInterceptsMouseClicks (true, false);
    }

    void PlatterView::setRotation (double positionSeconds, double effectiveRatio)
    {
        juce::ignoreUnused (effectiveRatio);
        const double revolutions = positionSeconds / secondsPerRevolution;
        angleRadians = revolutions * juce::MathConstants<double>::twoPi;
        repaint();
    }

    void PlatterView::setPlaying (bool isPlaying)
    {
        playing = isPlaying;
    }

    void PlatterView::setScratchMode (bool isActive)
    {
        scratchMode = isActive;
        repaint();
    }

    double PlatterView::angleAtPoint (juce::Point<float> p) const
    {
        const auto centre = getLocalBounds().toFloat().getCentre();
        return std::atan2 ((double) (p.y - centre.y), (double) (p.x - centre.x));
    }

    void PlatterView::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat().reduced (4.0f);
        const float diameter = juce::jmin (bounds.getWidth(), bounds.getHeight());
        auto disc = juce::Rectangle<float> (diameter, diameter).withCentre (bounds.getCentre());

        g.setColour (juce::Colour (0xff1c1c1c));
        g.fillEllipse (disc);

        g.setColour (scratchMode ? juce::Colours::orangered
                                  : (playing ? juce::Colours::limegreen : juce::Colours::grey));
        g.drawEllipse (disc, 2.5f);

        // Rotation indicator stripe, like a label mark on a vinyl record.
        const auto centre = disc.getCentre();
        const float radius = diameter * 0.5f - 6.0f;
        const juce::Point<float> tip (centre.x + radius * (float) std::cos (angleRadians),
                                       centre.y + radius * (float) std::sin (angleRadians));

        g.setColour (juce::Colours::white);
        g.drawLine (juce::Line<float> (centre, tip), 3.0f);
        g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre (centre));
    }

    void PlatterView::mouseDown (const juce::MouseEvent& event)
    {
        dragging = true;
        dragStartAngle = angleAtPoint (event.position);

        if (onScratchActiveChanged != nullptr)
            onScratchActiveChanged (true);
    }

    void PlatterView::mouseDrag (const juce::MouseEvent& event)
    {
        if (! dragging)
            return;

        const double newAngle = angleAtPoint (event.position);
        double delta = newAngle - dragStartAngle;

        // Wrap to [-pi, pi] so a crossing of the +/-180 degree seam doesn't
        // produce a huge jump.
        while (delta > juce::MathConstants<double>::pi)  delta -= juce::MathConstants<double>::twoPi;
        while (delta < -juce::MathConstants<double>::pi) delta += juce::MathConstants<double>::twoPi;

        dragStartAngle = newAngle;

        const double deltaSeconds = (delta / juce::MathConstants<double>::twoPi) * secondsPerRevolution;

        if (onJog != nullptr)
            onJog (deltaSeconds);
    }

    void PlatterView::mouseUp (const juce::MouseEvent&)
    {
        dragging = false;

        if (onScratchActiveChanged != nullptr)
            onScratchActiveChanged (false);
    }
}
