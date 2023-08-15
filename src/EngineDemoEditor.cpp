#include "EngineDemoEditor.h"

EngineDemoEditor::EngineDemoEditor(juce::AudioProcessor& processor)
	: AudioProcessorEditor(processor) {
	/** Info Editor */
	this->infoEditor = std::make_unique<juce::TextEditor>();
	this->infoEditor->setMultiLine(true);
	this->infoEditor->setReadOnly(true);
	this->infoEditor->setWantsKeyboardFocus(false);
	this->infoEditor->setTabKeyUsedAsCharacter(true);
	this->infoEditor->setTextToShowWhenEmpty("No Info",
		juce::LookAndFeel::getDefaultLookAndFeel().findColour(
			juce::TextEditor::ColourIds::textColourId));
	this->addAndMakeVisible(this->infoEditor.get());

	/** Min Size */
	this->setResizeLimits(480, 270, INT_MAX, INT_MIN);
	this->setSize(480, 270);
}

void EngineDemoEditor::resized() {
	/** Content Area */
	auto area = this->getContentArea();
	bool contentVertical = area.getHeight() >= area.getWidth();

	/** Info Size */
	juce::Rectangle<int> infoArea = contentVertical
		? area.withTrimmedTop(area.getHeight() / 2)
		: area.withTrimmedLeft(area.getWidth() / 2);
	if (this->infoEditor) {
		this->infoEditor->setBounds(infoArea);
	}
}

void EngineDemoEditor::paint(juce::Graphics& g) {
	/** LAF */
	auto& laf = juce::LookAndFeel::getDefaultLookAndFeel();

	/** Content Area */
	auto area = this->getContentArea();
	bool contentVertical = area.getHeight() >= area.getWidth();

	/** Fill A EngineDemoEditor::rea */
	g.setColour(laf.findColour(juce::ResizableWindow::ColourIds::backgroundColourId));
	g.fillRect(area);

	/** Text Area */
	juce::Rectangle<int> textArea = contentVertical
		? area.withTrimmedBottom(area.getHeight() / 2)
		: area.withTrimmedRight(area.getWidth() / 2);

	/** DMDA Status Area */
	juce::Rectangle<int> DMDAStatusArea = textArea.withHeight(area.getHeight() / 5);
	juce::String DMDAStatusStr = "DMDA Disconnected";
	juce::Colour DMDAStatusColour = juce::Colours::red;
	if (this->handShaked) {
		DMDAStatusStr = "DMDA Connected";
		DMDAStatusColour = juce::Colours::green;
	}

	g.setColour(DMDAStatusColour);
	g.drawFittedText(
		DMDAStatusStr, DMDAStatusArea, juce::Justification::centred, 1, 0.5);

	/** Render Status Area */
	juce::Rectangle<int> renderStatusArea
		= textArea.withTrimmedTop(DMDAStatusArea.getHeight()).withTrimmedBottom(
			area.getHeight() / 5 * 3);
	juce::String renderStatusStr = "Unrendered";
	if (this->rendered) {
		renderStatusStr = "Rendered";
	}

	g.setColour(laf.findColour(juce::Label::ColourIds::textColourId));
	g.drawFittedText(
		renderStatusStr, renderStatusArea, juce::Justification::centred, 1, 0.5);
}

void EngineDemoEditor::setHandShaked(bool handShaked) {
	this->handShaked = handShaked;
	this->repaint();
}

void EngineDemoEditor::setRendered(bool rendered) {
	this->rendered = rendered;
	this->repaint();
}

void EngineDemoEditor::setMidiInfo(const EngineDemoEditor::MidiInfo& info) {
	juce::String infoStr;

	/** Generate Text */
	auto& [trackNum, totalLength, totalEvents] = info;
	infoStr += "Track Num: " + juce::String(trackNum) + "\n";
	infoStr += "Total Length: " + juce::String(totalLength, 3) + "s\n";
	infoStr += "Total Events: " + juce::String(totalEvents) + "\n";

	/** Set Text */
	this->infoEditor->setReadOnly(false);
	this->infoEditor->setText(infoStr);
	this->infoEditor->setReadOnly(true);
}

void EngineDemoEditor::clearMidiInfo() {
	this->infoEditor->setReadOnly(false);
	this->infoEditor->clear();
	this->infoEditor->setReadOnly(true);
}

const juce::Rectangle<int> EngineDemoEditor::getContentArea() const {
	int width = this->getWidth(), height = this->getHeight();
	if (width > 2 * height) {
		width = 2 * height;
	}
	else if (height > 2 * width) {
		height = 2 * width;
	}
	return this->getLocalBounds().withSizeKeepingCentre(width, height);
}
