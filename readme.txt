
I don't know if anybody else finds this useful? but if you do, please feel free to use this anyway you like.
I have included the schematic pdf file as well as the Kicad.sch file.
I build this on a bread board so I didn't bother with making a PCB for a one off project, but that would be easy to do 
if anyone feels the urge to do so. 


To compile this code you need to install the pico-sdk and the toolchain
there are several good tutorials on YouTube about this. (I like this one) https://www.youtube.com/watch?v=mUF9xjDtFfY

After you installed the toolchain
Create a project source directory and copy the Program.cpp and CMakeLists.txt into it.
Create a subdirectory under the source directory called build 
go into the build directory and run the command: cmake -G "Ninja" ..

After cmake is done creating the build system
Then run the command: ninja
this should build the code and generate a westinghouse12k.uf2 file   

these are the commands 
md MySrcDir
cd MySrcDir
copy Program.cpp and CMakeLists.txt files into this directory
md build
cd build
cmake -G "Ninja" ..
ninja



This controller was designed using a Pico RP2040 with a few extra external components to start,stop and detect power
of the Westinghouse WGen9500DF generator via the build-in smart switch port from a transfer-switch relay contact 
that will close when utility power fails and open when power is present.

The generator is normally started manaully using a start pushbutton or a key fob remote starter, but
it could not be hooked up directly to the transfer-switch relay since the generator needs a pulse
of a specific length to start and longer pulse stop the generator. 

The controller was designed with the following features.

1) After powering up it will wait for the generator controller to become active before 
   sending a start pulse, this is necessary because the generator controller does not
   accept commands right after power-up.

2) The Controller will try to start the generator up to 3 times if it fails after that the controller will
   "lockup" and blink all LEDs this is done to prevent draining the battery with infinite restart attempts. 
   if the generator runs out of gas or fails for some other mechanical reason (such as forgetting to open the gas spigot) 
   (Lockup is reset by turning the power off and on again)

3) When power returns the controller will run the generator in "cool down" mode for 40 seconds before stopping it,
   if power fails during cool-down, it will just keep running and exit cool-down mode.

best of luck 

carsten svanholm
csvanholm@comcast.net






