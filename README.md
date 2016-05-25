# OpenBuudai
OpenBuudai - Open Source Oscilloscope Software based on OpenHantek

OpenBuudai is initially based on OpenHantek by Oliver Haag.

https://github.com/OpenHantek/openhantek

Current support:
- SainSmart DDS120
- SainSmart DDS140 (untested)
- Buudai/Rocktech BM102

##Hardware, Teardown & Discussion

http://www.360customs.de/en/2014/10/usb-oszilloskop-sainsmart-dds120-2-kanal-20mhz-50msps-buudairocktech-bm102/
http://www.eevblog.com/forum/testgear/sainsmart-dds120-usb-oscilloscope-%28buudai-bm102%29/

##Build

###Windows

You want this version of Qt and MinGW:
http://sourceforge.net/projects/qtx64/files/qt-x86/5.3.2/mingw-4.9/dwarf/qt-5.3.2-x86-mingw491r1-dw2-opengl.exe/download

Get this version of FFTW:
ftp://ftp.fftw.org/pub/fftw/fftw-3.3.4-dll32.zip

Note that you must create .lib files using MS Visual Studio.
I believe the Express C++ version has the lib.exe program which does this.
Issue 'vcvars32.bat' in C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin to set up the path. Then, lib /def:libfftw3-3.def etc. in the fftw-3.3.4-dll32 folder to create the lib file.)

Get this version of LibUSB:
http://sourceforge.net/projects/libusb/files/libusb-1.0/libusb-1.0.18/

See the #Configuration section of the .pro file for where to put LibUSB and FFTW.

Screenshots of path setup is shown here:

http://www.eevblog.com/forum/testgear/sainsmart-dds120-usb-oscilloscope-%28buudai-bm102%29/msg548786/#msg548786

###Linux

ToDo

###OSX

ToDo
