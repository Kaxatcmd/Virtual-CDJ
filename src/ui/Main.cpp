#include "MainComponent.h"

namespace vcdj::ui
{
    class VirtualCdjApplication final : public juce::JUCEApplication
    {
    public:
        VirtualCdjApplication() = default;

        const juce::String getApplicationName() override    { return "Virtual CDJ"; }
        const juce::String getApplicationVersion() override  { return "0.1.0"; }
        bool moreThanOneInstanceAllowed() override           { return true; }

        void initialise (const juce::String& commandLine) override
        {
            mainWindow = std::make_unique<MainWindow> (getApplicationName());

            // Optional convenience: `VirtualCDJ.exe <path-to-audio-file>` loads
            // the track immediately, useful for OS "open with" integration and
            // for quick manual/automated smoke testing.
            const auto trimmed = commandLine.trim().unquoted().trim();
            if (trimmed.isNotEmpty())
            {
                const juce::File file (trimmed);
                if (file.existsAsFile())
                    mainWindow->getMainComponent().loadFile (file);
            }
        }

        void shutdown() override
        {
            mainWindow = nullptr;
        }

        void systemRequestedQuit() override
        {
            quit();
        }

    private:
        class MainWindow final : public juce::DocumentWindow
        {
        public:
            explicit MainWindow (const juce::String& name)
                : DocumentWindow (name,
                                   juce::Desktop::getInstance().getDefaultLookAndFeel()
                                       .findColour (juce::ResizableWindow::backgroundColourId),
                                   DocumentWindow::allButtons)
            {
                setUsingNativeTitleBar (true);
                setContentOwned (new MainComponent(), true);

                centreWithSize (getWidth(), getHeight());
                setResizable (true, false);
                setVisible (true);

                if (auto* content = getContentComponent())
                    content->grabKeyboardFocus();
            }

            MainComponent& getMainComponent()
            {
                return *dynamic_cast<MainComponent*> (getContentComponent());
            }

            void closeButtonPressed() override
            {
                JUCEApplication::getInstance()->systemRequestedQuit();
            }
        };

        std::unique_ptr<MainWindow> mainWindow;
    };
}

START_JUCE_APPLICATION (vcdj::ui::VirtualCdjApplication)
