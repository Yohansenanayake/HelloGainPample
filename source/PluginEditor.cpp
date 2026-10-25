#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
      gainAttachment (processorRef.parameters, "gain", gainSlider)
{
    gainSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 80, 24);
    gainSlider.setTextValueSuffix (" x");
    gainSlider.setTitle ("Gain");
    addAndMakeVisible (gainSlider);
    addAndMakeVisible (gainLabel);

    addAndMakeVisible (inspectButton);

    // this chunk of code instantiates and opens the melatonin inspector
    inspectButton.onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }

        inspector->setVisible (true);
    };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds();
    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " + CMAKE_BUILD_TYPE;
    g.drawText (helloWorld, area.removeFromTop (40), juce::Justification::centred, false);
}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (40);
    gainLabel.setBounds (area.removeFromTop (24));
    gainSlider.setBounds (area.removeFromTop (48));
    inspectButton.setBounds (area.removeFromBottom (40).withSizeKeepingCentre (120, 30));
}
