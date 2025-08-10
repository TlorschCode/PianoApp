<h1 align="center">I suck at sightreading music</h1>
<h2 align="center">And so, I present to you, the Flashcard App</h2>
<h6 align="center">(For Lack of a Better Name)</h6>

</br>
<h3 align="center"><b>DISCLAIMER</b></h3>
<p align="center">This project is still very much in its Beta stages.</p></br>

<h2 align="center">How to Use</h2>
Download and run the installer from the <a href="https://github.com/TlorschCode/PianoApp/releases/tag/v1.0.0-beta">lastest release</a>. (Alternatively from <a href="">Itch.io</a>)</br>
Make sure your mic isn't muted, full microphone volume is best, actually.</br>
The app will display a note, if you play that note successfully it will change the target note to a different note. The app only supports notes G2-G5 right now.</br>
If you don't own a piano, you could try singing the note, or playing something from your phone.</br>

</br></br></br></br><hr></br></br></br></br>

<h2 align="center">Audio Detection</h2>
I'm using FFTW 3 as my FFT library, and am applying a number of algorithms to the data I get from the FFT function; namely a Hanning Window, a Harmonic Product Spectrum, and a Parabolic Product Spectrum.</br>
No, I'm not entirely certain as to what those last two do <i>exactly</i>, but I don't intend on spending my summer learning their intricacies.

<h2 align="center">Graphics</h2>
Since I have relatively little experience making graphical C++ applications, I chose Raylib 5.5 to be the graphics library for this project; it is lightweight and has a relatively simple API.</br>
I'm still trying to figure out how to improve the resolution of default shapes drawn by this library, because as of now, they are all displayed as bitmap objects and look quite ugly.
