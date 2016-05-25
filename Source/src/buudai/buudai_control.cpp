////////////////////////////////////////////////////////////////////////////////
//
//  OpenBuudai
//  buudai/control.cpp
//
//  Copyright (C) 2008, 2009  Oleg Khudyakov
//  prcoder@potrebitel.ru
//  Copyright (C) 2010  Oliver Haag
//  oliver.haag@gmail.com
//
//  This program is free software: you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by the Free
//  Software Foundation, either version 3 of the License, or (at your option)
//  any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//  more details.
//
//  You should have received a copy of the GNU General Public License along with
//  this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////


#include <QList>
#include <QMutex>
#include <QDebug>


#include "buudai/buudai_control.h"

#include "helper.h"
#include "buudai/buudai_device.h"
#include "buudai/buudai_types.h"
#define SKIP 8 // Skip ahead even number of bytes to bypass glitch at front of captured data to facilitate triggering


namespace Buudai {
	/// \brief Initializes the command buffers and lists.
	/// \param parent The parent widget.
	Control::Control(QObject *parent) : DsoControl(parent) {
		// Values for the Gain and Timebase enums
		gainSteps << 0.500 << 1.000 << 2.000 << 5.000 << 10.00; // in fullrange Volts
		samplerateChannelMax = 48e6;
		samplerateFastMax = 48e6;
		samplerateMax = samplerateChannelMax;
		samplerateDivider = 1;
        bufferMulti = 1;
		triggerPosition = 0;
		triggerSlope = Dso::SLOPE_POSITIVE;
		triggerSpecial = false;
		triggerSource = 0;
		triggerPoint = 0;

		// Channel level data
		for (unsigned int channel = 0; channel < BUUDAI_CHANNELS; channel++) {
			for (unsigned int gainId = 0; gainId < GAIN_COUNT; gainId++) {
				channelLevels[channel][gainId]= 0x0000;
			}
			sampleRange[channel] = 255;
			gain[channel] = GAIN_500MV;
		}

		offsetReal[0] = double(136)/sampleRange[0];
		offsetReal[1] = double(132)/sampleRange[1];
		cal[0] = 1.18;
		cal[1] = 1.21;

		// USB device
		device = new Device(this);
		
		// Sample buffers
		for (unsigned int channel = 0; channel < BUUDAI_CHANNELS; channel++) {
            samples.append(0);
            samplesSize.append(0);
		}
		
		connect(device, SIGNAL(disconnected()), this, SLOT(disconnectDevice()));
	}
	
	/// \brief Disconnects the device.
	Control::~Control() {
		device->disconnect();
	}
	

	unsigned char Control::sendRequest(unsigned char request, unsigned char value)
	{
	    unsigned char data = 0;
		usbMutex.lock();
		device->controlTransfer(0, request, &data, 1, value, 0, 1);
		usbMutex.unlock();
	    return data;

	}

	/// \brief Gets the physical channel count for this oscilloscope.
	/// \returns The number of physical channels.
	unsigned int Control::getChannelCount() {
		return BUUDAI_CHANNELS;
	}
	
	/// \brief Handles all USB things until the device gets disconnected.
	void Control::run() {
		int errorCode, cycleCounter = 0;

		while (!terminate) {
			if (sampling) {
				errorCode = getSamples(true);
				if (errorCode < 0)
					qDebug("Getting sample data failed: %s", Helper::libUsbErrorString(errorCode).toLocal8Bit().data());
            } else {
             usleep(1000);
            }
		}
		
		device->disconnect();
		emit statusMessage(tr("The device has been disconnected"), 0);
	}
	
	/// \brief Calculates the trigger point from the CommandGetCaptureState data.
	/// \param value The data value that contains the trigger point.
	/// \return The calculated trigger point for the given data.
	unsigned short int Control::calculateTriggerPoint(unsigned short int value) {
		unsigned short int result = value;
    
		// Each set bit inverts all bits with a lower value
		for (unsigned short int bitValue = 1; bitValue; bitValue <<= 1)
			if (result & bitValue)
				result ^= bitValue - 1;
		
		return result;
	}
	
    unsigned long int oldTriggerOffset = SKIP;
	
	/// \brief Gets sample data from the oscilloscope and converts it.
	/// \return 0 on success, libusb error code on error.
	int Control::getSamples(bool process) {
        long int errorCode;


		// Save raw data to temporary buffer
        unsigned long int dataCount = bufferSize * BUUDAI_CHANNELS * bufferMulti; // Adequate sizes for all timebases
        // Oversizing buffer unlikely to overcome an observed capture discountinuity at 2KB boundaries (HW/FW limitation?)
        //unsigned long int dataCount = bufferSize * BUUDAI_CHANNELS * bufferMulti + 4096;
        unsigned long int dataLength = dataCount;
		
		unsigned char data[dataLength];
		usbMutex.lock();
		unsigned char res = 0;
        device->controlTransfer(0, (unsigned char)FIFO_CONTROL, &res, 1, (unsigned char)FIFO_CONTROL_CLEAR, 0, 1);
        errorCode = device->bulkReadMulti(data, dataLength, 3);
        //errorCode = device->bulkRead(data, dataLength, 1);
        // device->controlTransfer(0, (unsigned char)FIFO_CONTROL, &res, 1, (unsigned char)FIFO_CONTROL_CLEAR, 0, 1);

		usbMutex.unlock();

        //usleep(1000000);

		if (errorCode < 0)
			return errorCode;

		// Process the data only if we want it
		if (process) {
			// How much data did we really receive?
			dataLength = errorCode;
            dataCount = dataLength;
			
			samplesMutex.lock();
			
            unsigned long int channelDataCount = (dataCount / BUUDAI_CHANNELS) - SKIP;

                        int chLoop = 0; // allow this for-loop to start with trigger source channel
			for (int channel = triggerSource; chLoop < BUUDAI_CHANNELS; chLoop++, channel++) {
                             if (channel >= BUUDAI_CHANNELS)
                                 channel = 0; // wrap-around to next channel
                             channelDataCount = (dataCount / BUUDAI_CHANNELS) - SKIP; // re-initialize for each channel
				// Reallocate memory for samples if the sample count has changed
				if (!samples[channel] || samplesSize[channel] != channelDataCount) {
					if (samples[channel])
                        delete samples[channel];
					samples[channel] = new double[channelDataCount];
					samplesSize[channel] = channelDataCount;
				}

				// Convert data from the oscilloscope and write it into the sample buffer
                unsigned long int bufferPosition = SKIP ;//+ triggerPoint * 2;
                unsigned int bufferFraction = (dataCount / BUUDAI_CHANNELS) / 2; // only use a fraction of samplebuffer, scales with timebase via bufferMulti to catch slow signals on longer timebase.

                // For buffersizes < 2048 no initial shift is needed, as data fits in fifo without glitch

                //if (bufferMulti >= 2) { // Comment out as usefulness cannot be determined
                //    bufferPosition = 2048;
                //}

                //if (oldTriggerOffset >= 256 && oldTriggerOffset <= channelDataCount) { // Comment out as usefulness
                //bufferPosition = oldTriggerOffset - 256; // cannot be determined and conflict with trigger position sharing
//}
                if (oldTriggerOffset > SKIP && oldTriggerOffset <= dataCount && channel != triggerSource)
                    bufferPosition = oldTriggerOffset; // Use same trigger position offset found by trigger source

                // Trigger Hack for rising edge or falling edge trigger


                double dataValueOld = 0;
                unsigned long int firstTriggerPos = 0; // first trigger position that will be confirmed by another 3 hits
                int hitCounter = 0;
                int bad_Case = 1; // Assume bad case where start dataValue above TriggerPositionOffset for rising edge trigger
                // and where start dataValue below TriggerPositionOffset for falling edge trigger to avoid false positive

                for (unsigned int triggerPositionIndex = 0; triggerPositionIndex < bufferFraction && channel == triggerSource; triggerPositionIndex++){ // Only search for trigger position offset if channel is trigger source
                 double dataValue = ((double) data[bufferPosition + channel]/sampleRange[channel] - offsetReal[channel])*gainSteps[gain[channel]]*cal[channel];

                  // Search only when dataValue is NOT above TriggerPositionOffset for rising-edge trigger and
                  // when dataValue is NOT below TriggerPositionOffset for falling edge trigger to prevent false positive

                  if (bad_Case == 1 && (bufferPosition + (dataCount >> 7) + channel) < dataCount) {
                      // Look ahead comparison provide more stable rising edge detection
                      if (triggerSlope == Dso::SLOPE_POSITIVE) { // Look ahead step size based on time base (aka buffer size)
                          if (dataValue < (triggerPositionOffset-0.02)) { // so slow signal edge detect is more reliable
                              if (data[bufferPosition + channel] < data[bufferPosition + (dataCount >> 11) + channel])
                                  hitCounter++;
                              if (data[bufferPosition + channel] < data[bufferPosition + (dataCount >> 10) + channel])
                                  hitCounter++;
                              if (data[bufferPosition + channel] < data[bufferPosition + (dataCount >> 9) + channel])
                                  hitCounter++;
                              if (data[bufferPosition + channel] < data[bufferPosition + (dataCount >> 8) + channel])
                                  hitCounter++;
                              if (data[bufferPosition + channel] < data[bufferPosition + (dataCount >> 7) + channel])
                                  hitCounter++;
                          }
                      }
                      else if (triggerSlope == Dso::SLOPE_NEGATIVE) { // Look ahead comparison for falling edge detect
                          if (dataValue > (triggerPositionOffset+0.02)) { // +0.02 (arbitary) forced a better comparison
                              if (data[bufferPosition + channel] > data[bufferPosition + (dataCount >> 11) + channel])
                                  hitCounter++;
                              if (data[bufferPosition + channel] > data[bufferPosition + (dataCount >> 10) + channel])
                                  hitCounter++;
                              if (data[bufferPosition + channel] > data[bufferPosition + (dataCount >> 9) + channel])
                                  hitCounter++;
                              if (data[bufferPosition + channel] > data[bufferPosition + (dataCount >> 8) + channel])
                                  hitCounter++;
                              if (data[bufferPosition + channel] > data[bufferPosition + (dataCount >> 7) + channel])
                                  hitCounter++;
                          }
                     }
                     if (hitCounter >= 3) // Enable trigger position search by 3 hits out of 5 in sliding window
                         bad_Case = 0;

                     hitCounter = 0; // Reset for next step to confirm trigger position or next attempt to enable search
                  }

                 if (bad_Case == 0 &&
                 ((dataValue > dataValueOld && dataValue > triggerPositionOffset && triggerSlope == Dso::SLOPE_POSITIVE) ||
                  (dataValue < dataValueOld && dataValue < triggerPositionOffset && triggerSlope == Dso::SLOPE_NEGATIVE)))
                 { // Normal case search

                     // Tried sliding window search same as the above edge detection but not as stable for square waves

                     hitCounter++;
                     if (hitCounter == 1)
                         firstTriggerPos = bufferPosition; // First trigger position found
                     if (hitCounter >= 4) { // Confirm first trigger position by 3 more hits
                     bufferPositionOffset = firstTriggerPos;
                     oldTriggerOffset = firstTriggerPos; // Set confirmed trigger position to first hit position to keep
                     bufferPosition = firstTriggerPos; // the start of displayed waveform close to trigger position cursor
                     firstTriggerPos = 0; // Reset for next data transfer cycle
                     hitCounter = 0;
                     bad_Case = 1; // Reset to bad case assumption for next data transfer and search cycle
                     break;
                     }

                 }

                 dataValueOld = dataValue;
                 bufferPosition += 2; // Next position to check
                }

                if (oldTriggerOffset == SKIP && channel == triggerSource)
                    bufferPosition = SKIP; // No trigger position found reset buffer position for free running waveforms

                // triggerPosition



                //bufferPosition = int(bufferPositionOffset + (triggerPosition * 2));

                // shorten data due to trigger offset

                if (oldTriggerOffset > SKIP) // Check if trigger position is found. Otherwise, allow free running waveform
                    channelDataCount = channelDataCount - (bufferPosition / 2);

                // put data on screen

                for (unsigned long int realPosition = 0; realPosition < channelDataCount ; realPosition++) {
					if (bufferPosition >= dataCount)
                        bufferPosition %= dataCount;
								
					samples[channel][realPosition] = ((double) data[bufferPosition + channel]/sampleRange[channel] - offsetReal[channel])*gainSteps[gain[channel]]*cal[channel];
                     bufferPosition += 2; // Next position to check
				}
			}
oldTriggerOffset = SKIP; // Clear trigger position for next cycle free running waveforms when no trigger found
			samplesMutex.unlock();

            // limit framerate and load but be somewhat in sync with samplerate to avoid glitches

            if (bufferMulti < 2) {
            usleep(32768);
            } else {
                // usleep(bufferSize);
            }
            emit samplesAvailable(&(samples), &(samplesSize), (double) samplerateMax / samplerateDivider, &(samplesMutex));

		} // if (process)

		return 0;
	}
	
	/// \brief Sets the size of the sample buffer without updating dependencies.
	/// \param size The buffer size that should be met (S).
	/// \return The buffer size that has been set.
	unsigned long int Control::updateBufferSize(unsigned long int size) {
		BufferSizeId sizeId = (size <= BUFFER_SMALL) ? BUFFERID_SMALL : BUFFERID_LARGE;
		bufferSize = (sizeId == BUFFERID_SMALL) ? BUFFER_SMALL : BUFFER_LARGE;
		return bufferSize;
	}
	
	/// \brief Try to connect to the oscilloscope.
	void Control::connectDevice() {
		int errorCode;
		
		emit statusMessage(device->search(), 0);
		if (!device->isConnected())
			return;
		
		bool unsupported = true;
		switch(device->getModel()) {
			case MODEL_DDS120:
				unsupported = false;
                sendRequest(ISOCHRONOS_TRANSPORT, ISOCHRONOS_TRANSPORT_ON);
                sendRequest(FIFO_CONTROL, FIFO_CONTROL_CLEAR);
                sendRequest(CH1_GAIN, CH1_GAIN_1V);
                sendRequest(CH2_GAIN, CH1_GAIN_1V);
				sendRequest(SAMPLERATE, SET_SAMPLERATE_2_4MS);
				break;
			case MODEL_DDS140:
				break;
			
			default:
				device->disconnect();
				emit statusMessage(tr("Unknown model"), 0);
				return;
		}
		
		if (unsupported)
			qDebug("Warning: This Buudai DSO model isn't supported officially yet, so it may not be working as expected. Reports about your experiences are very welcome though (Please open a feature request in the tracker at https://sf.net/projects/openbuudai/ or email me directly to oliver.haag@gmail.com). If it's working perfectly I can remove this warning, if not it should be possible to get it working with your help soon.");

		// Maximum possible sample rate for a single channel
		switch(device->getModel()) {
			case MODEL_DDS120:
				samplerateChannelMax = 48e6;
				samplerateFastMax = 48e6;
				break;
			
			case MODEL_DDS140:
				samplerateChannelMax = 48e6;
				samplerateFastMax = 100e6;
				break;
			default: // just to quite the compiler
				return;
		}
		samplerateMax = samplerateChannelMax;
		samplerateDivider = 1;

		DsoControl::connectDevice();
	}

	bool Control::deviceFound() {
		QString searchResult = device->search();
		return searchResult.startsWith(tr("Device found:")); // not very robust...
	}

	/// \brief Sets the size of the oscilloscopes sample buffer.
	/// \param size The buffer size that should be met (S).
	/// \return The buffer size that has been set.
	unsigned long int Control::setBufferSize(unsigned long int size) {
		if (!device->isConnected())
			return 0;
		
		updateBufferSize(size);
		
		setTriggerPosition(triggerPosition);
		setSampleRate(samplerateMax / samplerateDivider);
		setTriggerSlope(triggerSlope);
		
		return bufferSize;
	}
	
	/// \brief Sets the sample rate of the oscilloscope.
	/// \param sampleRate The sample rate that should be met (S/s).
	/// \return The sample rate that has been set.
	unsigned long int Control::setSampleRate(unsigned long int sampleRate) {
		if (!device->isConnected() || sampleRate == 0)
			return 0;
		unsigned char res;
		if (sampleRate > 2.4e8) {

            // Multiplier to stretch buffersize

            bufferMulti = 1;
            sampleRate = 48e6;

			res = sendRequest(SAMPLERATE, SET_SAMPLERATE_48MS);
		} else if (sampleRate > 240e5) {

            // Multiplier to stretch buffersize

            bufferMulti = 1;
            sampleRate = 2.4e6;

			res = sendRequest(SAMPLERATE, SET_SAMPLERATE_2_4MS);
		} else { // sampleRate <= 240e5

            // Multiplier to stretch buffersize - Calculation

            bufferMulti = int(1179648 / sampleRate) + 1;
            sampleRate = 240e3;

			res = sendRequest(SAMPLERATE, SET_SAMPLERATE_240KS);
		}
		samplerateDivider = samplerateMax / sampleRate;
		return sampleRate;
	}	
	
	/// \brief Enables/disables filtering of the given channel.
	/// \param channel The channel that should be set.
	/// \param used true if the channel should be sampled.
	/// \return 0 on success, -1 on invalid channel.
	int Control::setChannelUsed(unsigned int channel, bool used) {
		if (!device->isConnected())
			return -2;
		
		if (channel >= BUUDAI_CHANNELS)
			return -1;
		
		return 0;
	}
	
	/// \brief Set the coupling for the given channel.
	/// \param channel The channel that should be set.
	/// \param coupling The new coupling for the channel.
	/// \return 0 on success, -1 on invalid channel.
	int Control::setCoupling(unsigned int channel, Dso::Coupling coupling) {
		if (!device->isConnected())
			return -2;
		
		switch (channel) {
		case 0:
			if (coupling == Dso::COUPLING_DC)
				sendRequest(CH1_COUPLING, CH1_COUPLING_DC);
			else if (coupling == Dso::COUPLING_AC)
				sendRequest(CH1_COUPLING, CH1_COUPLING_AC);
			else
				return -1;
			break;

		case 1:
			if (coupling == Dso::COUPLING_DC)
				sendRequest(CH2_COUPLING, CH2_COUPLING_DC);
			else if (coupling == Dso::COUPLING_AC)
				sendRequest(CH2_COUPLING, CH2_COUPLING_AC);
			else
				return -1;
			break;

		default:
			return -1;
		}

		return 0;
	}
	
	/// \brief Sets the gain for the given channel.
	/// \param channel The channel that should be set.
	/// \param gain The gain that should be met (V/div).
	/// \return The gain that has been set, -1 on invalid channel.
	double Control::setGain(unsigned int channel, double newGain) {
		if (!device->isConnected())
			return -2;
		
		if (channel >= BUUDAI_CHANNELS)
			return -1;

		int gainId;

		for (gainId = 0; gainId < GAIN_COUNT - 1; gainId++)
			if (this->gainSteps[gainId] >= newGain)
				break;

		this->gain[channel] = (Gain) gainId;

		switch (gainId) {
		case 0:
			if (channel == 0)
				sendRequest(CH1_GAIN, CH1_GAIN_50MV);
			else
				sendRequest(CH2_GAIN, CH2_GAIN_50MV);
			break;
		case 1:
			if (channel == 0)
				sendRequest(CH1_GAIN, CH1_GAIN_100MV);
			else
				sendRequest(CH2_GAIN, CH2_GAIN_100MV);
			break;
		case 2:
			if (channel == 0)
				sendRequest(CH1_GAIN, CH1_GAIN_200MV);
			else
				sendRequest(CH2_GAIN, CH2_GAIN_200MV);
			break;
		case 3:
			if (channel == 0)
				sendRequest(CH1_GAIN, CH1_GAIN_500MV);
			else
				sendRequest(CH2_GAIN, CH2_GAIN_500MV);
			break;
		default:
			if (channel == 0)
				sendRequest(CH1_GAIN, CH1_GAIN_1V);
			else
				sendRequest(CH2_GAIN, CH2_GAIN_1V);
			break;
		}

		return this->gainSteps[gainId];
	}
	
	/// \brief Set the offset for the given channel.
	/// \param channel The channel that should be set.
	/// \param offset The new offset value (0.0 - 1.0).
	/// \return The offset that has been set, -1.0 on invalid channel.
	double Control::setOffset(unsigned int channel, double offset) {
		if (!device->isConnected())
			return -2;
		
		if (channel >= BUUDAI_CHANNELS)
			return -1;
		
		return 0.0;
	}
	
	/// \brief Set the trigger mode.
	/// \return 0 on success, -1 on invalid mode.
	int Control::setTriggerMode(Dso::TriggerMode mode) {
		if (!device->isConnected())
			return -2;
		
		if (mode < Dso::TRIGGERMODE_AUTO || mode > Dso::TRIGGERMODE_SINGLE)
			return -1;
		
		triggerMode = mode;
		return 0;
	}
	
	/// \brief Set the trigger source.
	/// \param special true for a special channel (EXT, ...) as trigger source.
	/// \param id The number of the channel, that should be used as trigger.
	/// \return 0 on success, -1 on invalid channel.
	int Control::setTriggerSource(bool special, unsigned int id) {
		if (!device->isConnected())
			return -2;
		
		if ((!special && id >= BUUDAI_CHANNELS) || (special && id >= BUUDAI_SPECIAL_CHANNELS))
			return -1;
		
		// Generate trigger source value that will be transmitted
		int sourceValue;
		sourceValue = TRIGGER_CH1 - id;
		
		triggerSpecial = special;
		triggerSource = id;
		
		setTriggerLevel(id, triggerLevel[id]);
		
		return 0;
	}
	
	/// \brief Set the trigger level.
	/// \param channel The channel that should be set.
	/// \param level The new trigger level (V).
	/// \return The trigger level that has been set, -1.0 on invalid channel.
	double Control::setTriggerLevel(unsigned int channel, double level) {
		if (!device->isConnected())
			return -2;
		
		if (channel >= BUUDAI_CHANNELS)
			return -1.0;
		
                if (channel == triggerSource)
                    triggerPositionOffset = (level); // return the set triggerlevel to other functions - TRIGGER FIX
                triggerLevel[channel] = level; // Hack to initialise triggerLevel[] 

		return (double) level;
	}
	
	/// \brief Set the trigger slope.
	/// \param slope The Slope that should cause a trigger.
	/// \return 0 on success, -1 on invalid slope.
	int Control::setTriggerSlope(Dso::Slope slope) {
		if (!device->isConnected())
			return -2;
		
		if (slope != Dso::SLOPE_NEGATIVE && slope != Dso::SLOPE_POSITIVE)
			return -1;
		
		triggerSlope = slope;
		return 0;
	}
	


	/// \brief Set the trigger position.
	/// \param position The new trigger position (in s).
	/// \return The trigger position that has been set.
	double Control::setTriggerPosition(double position) {
		if (!device->isConnected())
			return -2;



		// All trigger positions are measured in samples
		unsigned long int positionSamples = position * samplerateMax / samplerateDivider;
		
        //triggerPosition = position;
        triggerPosition = (((positionSamples + 1)/2));


		return (double) positionSamples / samplerateMax * samplerateDivider;
	}

#ifdef DEBUG
	/// \brief Sends bulk/control commands directly.
	/// \param command The command as string (Has to be parsed).
	/// \return 0 on success, -1 on unknown command, -2 on syntax error.
	int Control::stringCommand(QString command) {
		if(!this->device->isConnected())
			return -3;

		return -1;
	}
#endif

}
