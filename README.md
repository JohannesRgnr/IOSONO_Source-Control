## IOSONO Source Control

Simple JUCE project to experiment with a couple things
- Send OSC data to IOSONO Core/IPC or to MAX. 
- Implement a few relevant distance cues: level attenuation, air absorption, doppler effect

  Air absorption is very approximative, but at least efficient: a simple 1-pole lowpass. Cutoff is calculated following the method described here:
  https://computingandrecording.wordpress.com/2017/07/05/approximating-atmospheric-absorption-with-a-simple-filter/

  Doppler shift is done with a simple delay line, with smoothing. TODO: implement a shift limiting, to avoid high-freq doppler shift, using for instance a saturation function.

  
