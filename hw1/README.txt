1. I am using a Linux system (Ubuntu 22.04.5 LTS x86_64) and cmake version 3.22.1. I compiled and ran  the program using the commands provided in the assignment. 

Inside a build/assignment1 directory inside of the unzipped folder, I ran

cmake ../..
make
./assignment1

2.I did not collaborate with anybody in the class.

3. The most useful information was looking at the equations in the lecture slides. The rest of my research was spent trying to learn C++ and debug the various segfaults that showed up, but I cannot find all of the stackoverflow/reddit searches I used. The heart artifact was made by using a manual tool to generate points and put them together https://www.desmos.com/calculator/d1ofwre0fr. The helical torus was an accidental creation when I attempted to use a script to make a donut, but I decided to keep it since it looked cool.

4. I do not know of any known problems. The segements for the patches and lines are discontinuous, but I believe that is a medium level extra credit that I did not implement.

5. I did not do any of the extra credit. 

6. Similar to the previous pset, I believe that the majority of the difficulty lies not in the mathematical graphics implementation itself, but instead in understanding the framework with which we are building the graphics within. The matrix math to do basis changes, changing control points between bases, or creating the normals/gradients/points for the patches roughly took only 20% of my pset time. The rest was trying to understand how the different GLOO components work, and learning C++ from scratch. I understand the necessity of these skills, but I wish there was more guidance, especially when C++ is not a prerequisite. Additionally, it feels extremely demotivating when you plan out the operations on paper very quickly, but are staring at a black screen and segfaults for hours until you get any results because you're reverse engineering the "intended" way of utilizing all of the gloo structures/classes. It seems against the "spirit" of the class to spend such a small portion of time actually doing the graphics math we learned in lecture.



