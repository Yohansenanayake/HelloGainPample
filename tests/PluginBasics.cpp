#include "helpers/test_helpers.h"
#include <PluginProcessor.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

TEST_CASE ("Gain parameter scales mono and stereo audio", "[gain]")
{
    const float input[2][4] = {
        { 1.0f, -0.5f, 0.0f, 0.25f },
        { -0.25f, 0.75f, -1.0f, 0.5f }
    };

    for (const int channels : { 1, 2 })
    {
        for (const float gain : { 0.0f, 0.5f, 1.0f, 2.0f })
        {
            PluginProcessor plugin;
            auto* parameter = plugin.parameters.getParameter ("gain");
            REQUIRE (parameter != nullptr);
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (gain));
            plugin.setPlayConfigDetails (channels, channels, 48000.0, 4);
            plugin.prepareToPlay (48000.0, 4);

            juce::AudioBuffer<float> buffer (channels, 4);
            juce::MidiBuffer midi;
            for (int channel = 0; channel < channels; ++channel)
                buffer.copyFrom (channel, 0, input[channel], 4);

            plugin.processBlock (buffer, midi);

            for (int channel = 0; channel < channels; ++channel)
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                    CHECK (buffer.getSample (channel, sample) == Catch::Approx (input[channel][sample] * gain));

            plugin.releaseResources();
        }
    }
}

TEST_CASE ("Gain is registered with the host", "[gain]")
{
    PluginProcessor plugin;
    REQUIRE (plugin.getParameters().size() == 1);
    auto* parameter = plugin.parameters.getParameter ("gain");
    REQUIRE (parameter != nullptr);
    CHECK (plugin.getParameters()[0] == parameter);
    CHECK (parameter->getName (32) == "Gain");
    CHECK (parameter->isAutomatable());
    CHECK (parameter->convertFrom0to1 (parameter->getDefaultValue()) == Catch::Approx (0.5f));
}

TEST_CASE ("Gain changes ramp equally on both channels across blocks", "[gain]")
{
    PluginProcessor plugin;
    plugin.prepareToPlay (48000.0, 512);
    auto* parameter = plugin.parameters.getParameter ("gain");
    REQUIRE (parameter != nullptr);
    parameter->setValueNotifyingHost (parameter->convertTo0to1 (1.0f));

    juce::AudioBuffer<float> buffer (2, 512);
    juce::MidiBuffer midi;
    float previous = 0.5f;
    for (int block = 0; block < 2; ++block)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            juce::FloatVectorOperations::fill (buffer.getWritePointer (channel), 1.0f, buffer.getNumSamples());

        plugin.processBlock (buffer, midi);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const float current = buffer.getSample (0, sample);
            CHECK (current >= previous);
            CHECK (current <= 1.0f);
            CHECK (current == Catch::Approx (buffer.getSample (1, sample)));
            previous = current;
        }
        if (block == 0)
        {
            CHECK (buffer.getSample (0, 0) > 0.5f);
            CHECK (buffer.getSample (0, 511) < 1.0f);
        }
    }
    CHECK (buffer.getSample (0, 447) == Catch::Approx (1.0f));
    CHECK (previous == Catch::Approx (1.0f));

    juce::AudioBuffer<float> empty (2, 0);
    plugin.processBlock (empty, midi);
    plugin.releaseResources();
}

TEST_CASE ("Gain state survives a host session round trip", "[state]")
{
    PluginProcessor original;
    auto* parameter = original.parameters.getParameter ("gain");
    REQUIRE (parameter != nullptr);
    parameter->setValueNotifyingHost (parameter->convertTo0to1 (1.5f));
    juce::MemoryBlock state;
    original.getStateInformation (state);

    PluginProcessor restored;
    restored.setStateInformation (state.getData(), static_cast<int> (state.getSize()));
    CHECK (restored.parameters.getRawParameterValue ("gain")->load() == Catch::Approx (1.5f));
    restored.prepareToPlay (48000.0, 1);
    juce::AudioBuffer<float> buffer (2, 1);
    buffer.setSample (0, 0, 0.5f);
    buffer.setSample (1, 0, -0.5f);
    juce::MidiBuffer midi;
    restored.processBlock (buffer, midi);
    CHECK (buffer.getSample (0, 0) == Catch::Approx (0.75f));
    CHECK (buffer.getSample (1, 0) == Catch::Approx (-0.75f));

    const char invalid[] = "not a plugin state";
    restored.setStateInformation (invalid, sizeof (invalid));
    CHECK (restored.parameters.getRawParameterValue ("gain")->load() == Catch::Approx (1.5f));
    juce::MemoryBlock unrelated;
    juce::AudioProcessor::copyXmlToBinary (juce::XmlElement ("OtherPlugin"), unrelated);
    restored.setStateInformation (unrelated.getData(), static_cast<int> (unrelated.getSize()));
    CHECK (restored.parameters.getRawParameterValue ("gain")->load() == Catch::Approx (1.5f));
    restored.releaseResources();
}

TEST_CASE ("Plugin instance", "[instance]")
{
    PluginProcessor testPlugin;

    SECTION ("name")
    {
        CHECK_THAT (testPlugin.getName().toStdString(),
            Catch::Matchers::Equals ("Simple Gain"));
    }

    SECTION ("program name")
    {
        // Steinberg's VST3 validator fails plugins whose programs have no name
        CHECK (testPlugin.getProgramName (0).isNotEmpty());
    }
}

#ifdef PAMPLEJUCE_IPP
    #include <ipp.h>

TEST_CASE ("IPP version", "[ipp]")
{
    #if defined(__APPLE__)
    // macOS uses 2021.9.1 from pip wheel (only x86_64 version available)
    CHECK_THAT (ippsGetLibVersion()->Version, Catch::Matchers::Equals ("2021.9.1 (r0x7e208212)"));
    #else
    CHECK_THAT (ippsGetLibVersion()->Version, Catch::Matchers::Equals ("2026.0.0 (r0xa7ad6ebc)"));
    #endif
}
#endif
