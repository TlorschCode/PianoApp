<h1 align="center">I suck at sightreading music</h1>
<h2 align="center">And so, I present to you, the Flashcard App</h2>
<h6 align="center">(For Lack of a Better Name)</h6>

</br>
<h3 align="center"><b>DISCLAIMER</b></h3>
<p align="center">This project is still very much in its Beta stages.</p></br>

<h2 align="center">How to Use</h2>
Download and run the installer from the lastest release.</br>
Make sure your mic isn't muted, full volume is best, actually. If you own a piano, play any key between G2 (second G on the piano) and G5 (fifth G on the piano). If you don't own a piano, you could try singing one of those notes, or play something from your phone.</br>
<b>NOTE</b></br>
The app isn't very performant or accurate (for very long at least), but that is because it is intended to listen for a target frequency that it chooses, not display whatever it hears. For example, it could display B#3 (third B#), and the user would have to play the key on the piano; the app is meant to be a digital flashcard program.</br>
I'll change it to have the flashcard functionality soon, but as of right now it displays whatever it hears for debugging purposes.</br>

<h2 align="center">Audio Detection</h2>
I'm using FFTW 3 as my FFT library, and am applying a number of algorithms to the data I get from the FFT function; namely a Hanning Window, a Harmonic Product Spectrum, and a Parabolic Product Spectrum.</br>
No, I'm not entirely certain as to what those last two do <i>exactly</i>, but I don't intend on spending my summer learning their intricacies.

<h2 align="center">Graphics</h2>
Since I have relatively little experience making graphical C++ applications, I chose Raylib 5.5 to be the graphics library for this project; it is lightweight and has a relatively simple API.</br>
I'm still trying to figure out how to improve the resolution of default shapes drawn by this library, because as of now, they are all displayed as bitmap objects and look quite ugly.
