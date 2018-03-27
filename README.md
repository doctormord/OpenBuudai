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

To build OpenHantek from source, you need Qt 4 and FFTW 3. Under Debian or Ubuntu you can just install the packages libqt4-dev and libfftw3-dev. I don't know the package names for other distributions but they may be similar.

Edit Source/OpenHantek.pro to enable the following lines while disabling comparable lines above
```
INCLUDEPATH += C:/Qt/lib/libusb/include/ # Find .h files
# LIBS += -LC:/Qt/lib/libusb/MS32/dll/ -llibusb-1.0 # Find .lib files
LIBS +=  -lusb-1.0 # Find .lib files Linux Build

INCLUDEPATH += C:/Qt/lib/fftw-3.3.4-dll32 # Find .h files
# LIBS += -LC:/Qt/lib/fftw-3.3.4-dll32/ -lfftw3-3 # Find .lib files
LIBS += -lfftw3 # Find .lib files Linux Build
```

After you've installed the requirements run the following commands inside the Source directory:

```bash
$ qmake PREFIX=/usr
$ make
$ make install
```


###OSX

ToDo
