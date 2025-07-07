<h1 align="center">I suck at sightreading music</h1>
<h2 align="center">And so, I present to you, the Flashcard App</h2>
<h6 align="center">(For Lack of a Better Name)</h6>

</br>
<h3 align="center"><b>DISCLAIMER</b></h3>
<p align="center">This project is still very much in its Alpha stages.</p></br>

<h2 align="center">Audio Detection</h2>
I'm using FFTW 3 as my FFT library, and am applying a number of algorithms to the data I get from the FFT function; namely a Hanning Window, a Harmonic Product Spectrum, and a Parabolic Product Spectrum.</br>
No, I'm not entirely certain as to what those last two do <i>exactly</i>, but I don't intend on spending my summer learning their intricacies.

<h2 align="center">Graphics</h2>
Since I have relatively little experience making graphical C++ applications, I chose Raylib 5.5 to be the graphics library for this project.</br>
I'm still trying to figure out how to improve the resolution of default shapes drawn by this library, because as of now, they are all displayed as bitmap objects and look quite ugly.
