# Tanks Game
<h2>Platforms</h2>
The game is available on three platforms. You can just download and play
<ul>
<li><a href=https://github.com/PDansss/MyGame/releases/tag/Linux>Linux</a></li>
<li><a href=https://github.com/PDansss/MyGame/releases/tag/Windows_Build>Windows</a></li>
<li><a href=https://github.com/PDansss/MyGame/releases/tag/Android_Build>Android</a></li>
</ul>
<h2>Build</h2>
If want to build the project, you have to install SDL3
<h3>Linux & Windows</h3>
<pre>
git clone https://github.com/PDansss/MyGame.git
mkdir build
cd build
cmake .. 
cmake --build .
</pre>
<h3>Android</h3>
<ol>
 <li>Open folder android-project in Android Studio</li>
 <li>In local.properties file set path to SDL3 in variable sdl3.dir</li>
</ol>
