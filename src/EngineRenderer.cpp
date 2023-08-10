#include "EngineRenderer.h"

#define PITCH_ACCURACY 100
#define MIDI_PITCH_MAX 8191
#define MIDI_PITCH_MIN -8192

EngineRenderer::EngineRenderer(int KPSSeed) {
	/** KPS Device */
	auto device = KarPlusStrong::Device::Normal;
	if (juce::SystemStats::hasSSE3()) {
		device = KarPlusStrong::Device::SSE3;
	}
	if (juce::SystemStats::hasAVX2()) {
		device = KarPlusStrong::Device::AVX2;
	}
	if (juce::SystemStats::hasAVX512F()) {
		device = KarPlusStrong::Device::AVX512;
	}

	/** KPS Renderer */
	this->kps = std::make_unique<KarPlusStrong>(KPSSeed, device);
}

void EngineRenderer::releaseData() {
	juce::ScopedWriteLock locker(this->bufferLock);
	this->buffer.clear();
	this->rendered = false;
}

bool EngineRenderer::isRendered() const {
	juce::ScopedReadLock locker(this->bufferLock);
	return this->rendered;
}

void EngineRenderer::prepare(double sampleRate) {
	juce::ScopedWriteLock locker(this->bufferLock);
	if (this->sampleRate != sampleRate) {
		this->sampleRate = sampleRate;
		this->releaseData();
	}
}

void EngineRenderer::render(const juce::MidiFile& context) {
	/** Locker */
	juce::ScopedWriteLock locker(this->bufferLock);

	/** Clear Data */
	this->releaseData();

	/** Init Buffer */
	double timeInSeconds = context.getLastTimestamp();
	int bufferSize = std::ceil(timeInSeconds * this->sampleRate);
	this->buffer.setSize(1, bufferSize);

	/** Render For Each Track */
	for (int i = 0; i < context.getNumTracks(); i++) {
		/** Get Track */
		auto track = context.getTrack(i);
		int totalEvents = track->getNumEvents();

		/** Param Temp */
		juce::Array<double> pitchTemp;
		pitchTemp.resize(bufferSize / PITCH_ACCURACY);

		/** Get Pitch */
		{
			int lastParamPlace = 0;
			double lastPitchData = 0;
			for (int j = 0; j < totalEvents; j++) {
				/** Get Current Note */
				auto event = track->getEventPointer(j);
				if (!event->message.isPitchWheel()) { continue; }

				/** Get Event Time */
				double time = event->message.getTimeStamp();
				int samplePlace = time * this->sampleRate;
				int paramPlace = samplePlace / PITCH_ACCURACY;

				/** Get Pitch Value */
				int pitchValue = event->message.getPitchWheelValue();
				double pitchData = (pitchValue >= 0)
					? (static_cast<double>(pitchValue) / MIDI_PITCH_MAX)
					: (static_cast<double>(pitchValue) / MIDI_PITCH_MIN);
				pitchData *= 2;

				/** Store Last Value */
				for (int k = lastParamPlace; k < paramPlace; k++) {
					pitchTemp.setUnchecked(k, lastPitchData);
				}

				/** Value Temp */
				lastParamPlace = paramPlace;
				lastPitchData = pitchData;
			}

			/** Store Last Value */
			for (int k = lastParamPlace; k < pitchTemp.size(); k++) {
				pitchTemp.setUnchecked(k, lastPitchData);
			}
		}

		/** Synth */
		for (int i = 0; i < totalEvents; i++) {
			/** Check For Stop */
			if (juce::Thread::currentThreadShouldExit()) {
				this->releaseData();
				return;
			}

			/** Get Current Note */
			auto event = track->getEventPointer(i);
			if (!event->message.isNoteOn()) { continue; }
			auto endEvent = event->noteOffObject;

			/** Get Note Info */
			double startTime = event->message.getTimeStamp();
			double endTime =
				endEvent ? endEvent->message.getTimeStamp() : track->getEndTime();
			int startSample = startTime * sampleRate;
			int endSample = endTime * sampleRate;
			int noteNumber = event->message.getNoteNumber();
			double freq = 440 * std::pow(2, static_cast<double>(noteNumber - 69) / 12);

			/** Pitch Param */
			int paramStartPlace = startSample / PITCH_ACCURACY;
			int paramEndPlace = endSample / PITCH_ACCURACY;
			int paramDeviation = startSample - paramStartPlace * PITCH_ACCURACY;
			juce::Array<double> freqTemp;
			freqTemp.resize(paramEndPlace - paramStartPlace);

			/** TODO Caculate Freq */

			/** Synth */
			if (startSample >= this->buffer.getNumSamples()) { continue; }
			int noteLength = std::min(endSample - startSample,
				this->buffer.getNumSamples() - 1 - startSample);
			this->kps->synth(this->buffer, this->sampleRate, startSample,
				noteLength, freqTemp, PITCH_ACCURACY, paramDeviation);
		}
	}
}

void EngineRenderer::getAudio(
	juce::AudioBuffer<float>& buffer, int64_t timeInSamples) const {
	/** TODO Get Audio Data */
}
