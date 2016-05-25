////////////////////////////////////////////////////////////////////////////////
//
//  OpenBuudai
/// \file buudai/control.h
/// \brief Declares the Buudai::Control class.
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


#ifndef BUUDAI_CONTROL_H
#define BUUDAI_CONTROL_H


#include <QMutex>


#include "dsocontrol.h"
#include "helper.h"
#include "buudai/buudai_types.h"


namespace Buudai {
	class Device;
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum ControlIndex                                        buudai/control.h
	/// \brief The array indices for the waiting control commands.
	enum ControlIndex {
		//CONTROLINDEX_VALUE,
		//CONTROLINDEX_GETSPEED,
		//CONTROLINDEX_BEGINCOMMAND,
		CONTROLINDEX_SETOFFSET,
		CONTROLINDEX_SETRELAYS,
		CONTROLINDEX_COUNT
	};
	
	//////////////////////////////////////////////////////////////////////////////
	/// \class Control                                            buudai/control.h
	/// \brief The DsoControl abstraction layer for %Buudai USB DSOs.
	class Control : public DsoControl {
		Q_OBJECT
		
		public:
			Control(QObject *parent = 0);
			~Control();
			
			unsigned int getChannelCount();
		
		protected:
			void run();
			
			unsigned short int calculateTriggerPoint(unsigned short int value);
//			int getCaptureState();
			int getSamples(bool process);
			unsigned long int updateBufferSize(unsigned long int size);
			
			Device *device; ///< The USB device for the oscilloscope

			/// Calibration data for the channel offsets
			unsigned short int channelLevels[BUUDAI_CHANNELS][GAIN_COUNT];
			
			// Various cached settings
            unsigned long int bufferMulti; /// buffer-length multiplicator FIX
            unsigned long int bufferPositionOffset; /// buffer-offset FIX
            double triggerPositionOffset; /// Some trigger approach FIX
            unsigned long int samplerateDivider; ///< The samplerate divider
			unsigned long int samplerateMax; ///< The maximum sample rate for the current setup
			unsigned long int samplerateChannelMax; ///< The maximum sample rate for a single channel
			unsigned long int samplerateFastMax; ///< The maximum sample rate for fast rate mode
			Gain gain[BUUDAI_CHANNELS]; ///< The gain id
			unsigned short int sampleRange[BUUDAI_CHANNELS]; ///< The sample values at the top of the screen
			double offset[BUUDAI_CHANNELS]; ///< The current screen offset for each channel
			double offsetReal[BUUDAI_CHANNELS]; ///< The real offset for each channel (Due to quantization)
			double cal[BUUDAI_CHANNELS]; ///< The real offset for each channel (Due to quantization)
			double triggerLevel[BUUDAI_CHANNELS]; ///< The trigger level for each channel in V
			double triggerPosition; ///< The current pretrigger position
            unsigned long int bufferSize; ///< The buffer size in samples
			unsigned int triggerPoint; ///< The trigger point value
			Dso::TriggerMode triggerMode; ///< The trigger mode
			Dso::Slope triggerSlope; ///< The trigger slope
			bool triggerSpecial; ///< true, if the trigger source is special
			unsigned int triggerSource; ///< The trigger source
			
			QList<double *> samples; ///< Sample data arrays
			QList<unsigned int> samplesSize; ///< Number of samples data array
			QMutex samplesMutex; ///< Mutex for the sample data
			QMutex usbMutex; ///< Mutex for the USB connection
			
			// Lists for enums
			QList<double> gainSteps; ///< Voltage steps in V/screenheight

			unsigned char sendRequest(unsigned char request, unsigned char value);

		public slots:
			virtual void connectDevice();
			virtual bool deviceFound();

			unsigned long int setSampleRate(unsigned long int samplerate);
			unsigned long int setBufferSize(unsigned long int size);
			
			int setChannelUsed(unsigned int channel, bool used);
			int setCoupling(unsigned int channel, Dso::Coupling coupling);
			double setGain(unsigned int channel, double gain);
			double setOffset(unsigned int channel, double offset);
			
			int setTriggerMode(Dso::TriggerMode mode);
			int setTriggerSource(bool special, unsigned int id);
			double setTriggerLevel(unsigned int channel, double level);
			int setTriggerSlope(Dso::Slope slope);
			double setTriggerPosition(double position);
			
#ifdef DEBUG
			int stringCommand(QString command);
#endif
	};
}


#endif
