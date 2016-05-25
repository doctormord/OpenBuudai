////////////////////////////////////////////////////////////////////////////////
//
//  OpenBuudai
/// \file buudai/types.h
/// \brief Declares types needed for the Buudai::Device class.
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


#ifndef BUUDAI_TYPES_H
#define BUUDAI_TYPES_H

#include "helper.h"

#define BUUDAI_DDS120_VID        0x8102 ///< VID for Buudai/Sainsmart DDS120
#define BUUDAI_DDS140_VID        0x8312 ///< VID for Buudai/Sainsmart DDS140
#define BUUDAI_EP_OUT              0x02 ///< OUT Endpoint for bulk transfers
#define BUUDAI_EP_IN               0x82 ///< IN Endpoint for bulk transfers
#define BUUDAI_TIMEOUT              500 ///< Timeout for USB transfers in ms
#define BUUDAI_ATTEMPTS_DEFAULT       3 ///< The number of transfer attempts

#define BUUDAI_CHANNELS               2 ///< Number of physical channels
#define BUUDAI_SPECIAL_CHANNELS       0 ///< Number of special channels


////////////////////////////////////////////////////////////////////////////////
/// \namespace Buudai                                             buudai/types.h
/// \brief All %Buudai DSO device specific things.
namespace Buudai {

/// Control: Value
/// 0x22: Ch1 sensitivity (/div): 0x08:50mV, 0x04:100mV, 0x00:200mV, 0x06:500mV, 0x02:1-5V
/// 0x23: Ch2 sensitivity (/div): 0x20:50mV, 0x10:100mV, 0x00:200mV, 0x12:500mV, 0x02:1-5V
/// 0x24: Ch1 coupling 0x08:DC, 0x00:AC
/// 0x25: Ch2 coupling 0x01:DC, 0x00:AC
/// 0x33: Reset FIFO buffer 0x00 (?)
/// 0x94: Sampling rate 0x10:48MS, 0x01:2.4MS, 0x11:240kS
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum ControlCode                                           buudai/types.h
	/// \brief All supported control commands.
	enum ControlCode {
		/// <p>
		///   The 0x22 control command sets the Ch1 gain
		///   Ch1 gain (/div): 0x08:50mV, 0x04:100mV, 0x00:200mV, 0x06:500mV, 0x02:1-5V
		/// </p>
		CH1_GAIN = 0x22,
		
		/// <p>
		///   The 0x23 control command sets the Ch2 gain
		///   Ch2 gain (/div): 0x20:50mV, 0x10:100mV, 0x00:200mV, 0x12:500mV, 0x02:1-5V
		/// </p>
		CH2_GAIN = 0x23,

		/// <p>
		///   The 0x24 control command sets the Ch1 coupling
		///   Ch1 coupling 0x08:DC, 0x00:AC
		/// </p>
		CH1_COUPLING = 0x24,

		/// <p>
		///   The 0x25 control command sets the Ch2 coupling
		///   Ch2 coupling 0x01:DC, 0x00:AC
		/// </p>
		CH2_COUPLING = 0x25,

		/// <p>
		///   The 0x31 control command set isochronos USB mode
		///   isochronos transport: 0x00
		/// </p>
		ISOCHRONOS_TRANSPORT = 0x31,

		/// <p>
		///   The 0x33 control command clears the FIFO
		///   clear FIFO: 0x00
		/// </p>
        FIFO_CONTROL = 0x33,

		/// <p>
		///   The 0x94 control command sets the hardware sample rate
		///   Sampling rate 0x10:48MHz, 0x01:2.4MHz, 0x11:240kHz
		/// </p>
		SAMPLERATE = 0x94,
	};
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum ControlValue                                          buudai/types.h
	/// \brief All supported values for control commands.
	enum ControlValue {
		CH1_GAIN_50MV  = 0x08,
		CH1_GAIN_100MV = 0x04,
		CH1_GAIN_200MV = 0x00,
		CH1_GAIN_500MV = 0x06,
		CH1_GAIN_1V    = 0x02,
		
		CH2_GAIN_50MV  = 0x20,
		CH2_GAIN_100MV = 0x10,
		CH2_GAIN_200MV = 0x00,
		CH2_GAIN_500MV = 0x12,
		CH2_GAIN_1V    = 0x02,
		
		CH1_COUPLING_DC = 0x08,
		CH1_COUPLING_AC = 0x00,

		CH2_COUPLING_DC = 0x01,
		CH2_COUPLING_AC = 0x00,

		ISOCHRONOS_TRANSPORT_ON = 0x00,

		FIFO_CONTROL_CLEAR = 0x00,

		SET_SAMPLERATE_48MS  = 0x10,
		SET_SAMPLERATE_2_4MS = 0x01,
		SET_SAMPLERATE_240KS = 0x11,
	};
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum Model                                                 buudai/types.h
	/// \brief All supported Buudai DSO models.
	enum Model {
		MODEL_UNKNOWN = -1,
		MODEL_DDS120,
		MODEL_DDS140,
		MODEL_COUNT
	};
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum ConnectionSpeed                                       buudai/types.h
	/// \brief The speed level of the USB connection.
	enum ConnectionSpeed {
		CONNECTION_FULLSPEED = 0, ///< FullSpeed USB, 64 byte bulk transfers
		CONNECTION_HIGHSPEED = 1  ///< HighSpeed USB, 512 byte bulk transfers
	};
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum Samplerate                                            buudai/types.h
	/// \brief The different sample rates supported by Buudai DSOs.
	enum Samplerate {
		SAMPLERATE_48MS,
		SAMPLERATE_2_4MS,
		SAMPLERATE_240KS,
		SAMPLERATE_COUNT
	};
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum Gain                                                  buudai/types.h
	/// \brief The different gain steps supported by Buudai DSOs.
	enum Gain {
		GAIN_50MV,
		GAIN_100MV,
		GAIN_200MV,
		GAIN_500MV,
		GAIN_1V,
		GAIN_COUNT
	};
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum UsedChannels                                          buudai/types.h
	/// \brief The enabled channels.
	enum UsedChannels {
		USED_CH1, USED_CH2,
		USED_CH1CH2
	};
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum TriggerSource                                         buudai/types.h
	/// \brief The possible trigger sources.
	enum TriggerSource {
		TRIGGER_CH2, TRIGGER_CH1,
		TRIGGER_ALT,
	};
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum BufferSize                                            buudai/types.h
	/// \brief The size of the sample buffer.
	enum BufferSize {
        BUFFER_SMALL = 2048,
        BUFFER_LARGE = 32768 /// 2048, 32768
	};
	
	//////////////////////////////////////////////////////////////////////////////
	/// \enum BufferSizeId                                          buudai/types.h
	/// \brief The size id for CommandSetTriggerAndSamplerate.
	enum BufferSizeId {
        BUFFERID_ROLL = 0, ///< Used for the roll mode
		BUFFERID_SMALL, ///< The standard buffer with 10240 samples
		BUFFERID_LARGE ///< The large buffer, 32768 samples (14336 for DSO-5200)
	};
	
}
#endif
